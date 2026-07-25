// Copyright 2026 - 2026, Marseille Bouchard
// SPDX-Liense-Identifier: LGPL-3.0-or-later

#define A_IMPLEMENTATION
#include "a.h"

#include <stdint.h>
#include <assert.h>

/// # Temporary Allocator
///
/// First up, we have a statically allocated byte array, `tHeap`,
///   and `tNext` points to the next place we will allocate.
/// A cheeky lil' `tEnd` definition points to just past the `tHeap`, so we can check for out-of-bounds.
  static
  alignas(max_align_t)
char tHeap[A_TALLOC_SIZE];
  static
#define tEnd (&tHeap[A_TALLOC_SIZE])
char* tNext = &tHeap[0];
///
/// Note that I've gone for static storage instead of thread-local.
/// This means the temp allocator is not thread safe!
/// However, I have yet to find a way to initialize `tNext` per-thread without requiring the user to call some sort of \*`a_initThread` function.
///

/// `talloc` just bumps `tNext` and re-aligns.
/// The `tHeap` address is already aligned to the fundamental alignment, so we can do the alignment at the end.
void* a_talloc(size_t nBytes) {
  if (nBytes == 0) {
    nBytes = 1;
  }
  nBytes = a_alup(nBytes, A_FUND_ALIGN);
  if ((ptrdiff_t)nBytes > tEnd - tNext) {
    return NULL;
  }
  void* out = tNext;
  tNext += nBytes;
  return out;
}

/// The save function just returns the current `tNext`,
/// and the reset function re-installs it.
A_Savepoint a_tallocSave() {
  return tNext;
}
void a_tallocReset(A_Savepoint base) {
/// There's an assertion for it to be in-range, but that's all the checking we do here.
/// I don't expect anyone to screw it up, as long as they stick to keeping the savepoints inside a stack frame.
  assert((void*)tHeap <= base && base < (void*)tEnd);
  tNext = base;
}

///////////////////////////////
////// Garbage Collector //////
///////////////////////////////

////// Engine //////

typedef struct gcEngine {
  char* active;
  usz cap;
  usz used;
  char* stale;
  // TODO a rootlist
  a_gcObj* finiList; // nullable
} gcEngine;

gcEngine gc;

static
void* gcAlloc(usz objSize) {
  if (gc.used + objSize > gc.cap) {
    // out of memory, needs collection
    return NULL;
  }
  assert((gc.used & 3) == 0);
  void* out = gc.active + gc.used;
  gc.used += objSize;
  assert((gc.used & 3) == 0);
  return out;
}

////// Formation Rules //////

typedef uptr a_gcHdr;
struct a_gcObj {
  union {
    a_gcHdr hdr;
    a_gcObj* fwd;
  } as;
};
typedef struct gcFini {
  void (*fini)(a_gcObj*); // the finializer, called on the object itself
  a_gcObj* next; // the next (ir previously-allocated) object in the heap that has a finalizer.
                 // the next object may or may not be forwarded; forwarded objects need their next pointer updated, and unforwarded ones need their finalizer called
} gcFini;
/// A `gcObj*` is the user interface to gc-managed objects.
/// The `gcObj` is the actual memory holding that object, which can vary from object to object.
/// Every `gcObj` starts with a `gcHdr` describing the layout of the specific object.
/// I've decided not to hide the pointer on the user side, since as far as the user is concerned, ocObjs _are_ just o[aque poitners.
///
/// A gcObj is laid out like this in memory:
/// ```
/// 1x header: pointer-sized <--- 0 offset
/// Nx children: each a pointer to a gcobj
/// Mx bytes: bytes that do not contain any gcobj pointers; M must be a multiple of pointer-size
/// 0-1x finalizer info:
///        pointer to a function to be called when the object is about to be collected,
///        and a pointer to the next/previous object with a finalizer
/// ```
/// The header is a bitfield that describes the layout of the specific object.
/// Instead of using notorious C bitstructs, I'll just do the bit arithmetic myself.
/// Here's its layout:
/// ```
/// Regular object:
///   bit:   | 62--31   | 31--16       | 15--2         | 1 | 0
///   usage: | reserved | num children | num bytes / 4 | 1 | has finalizer?
/// Forwarded object, used only during collection.
///   bit:   | 63--2          | 1--0 |
///   usage: | forwarding ptr | 0    |
/// ```
///
/// I'm only using the lowest two bits for tags because the gc should also be able to work on 32-bit systems, where the alignment of a pointer is only 4 bytes.
/// Even if it never compiles to a 32-bit system, it may be advantageous to restrain the gc heap to 4GiB to make more efficient use of the bit in representing pointers
///   (such an optimization would require a global that tracks the base of the gcheap).
/// I'm also only using bits 31--2 for lengths so that the header functions easily work with a 32-bit size.
///
/// The number of bytes or children held by an object both max out at 2^16.
/// If you need larger contiguous byte arrays, I suggest a malloc/mmap'd pointer stored in the bytes, equipped with the relevant finalizer.
/// If you need larger arrays of children, I suggest a more sophisticated data structure, such as a finger tree, block list, or the like.
/// If your data has alignment restrictions, it's best to store a pointer to a separate `aligned_malloc`.
///
/// The number of butes should reflect allocated bytes, not necesarily used bytes.
/// The number of allocated bytes should be a multiple of the size of a `gcObj*`.

////// Elimination Rules //////

// check for forwarding
// outside of collection, objects will not be forwarded
static inline
bool gc_isFwded(a_gcObj* obj) {
  return (obj->as.hdr & 3) == 0;
}
static inline
a_gcObj* gc_fwdPtr(const a_gcObj* obj) {
  assert(gc_isFwded(obj));
  return obj->as.fwd;
}

// accessing children
static inline
usz gc_nChildren(const a_gcObj* obj) {
  assert(!gc_isFwded(obj));
  return (obj->as.hdr >> 16) & 0xFFFF;
}
static inline
a_gcObj** gc_child(const a_gcObj* obj, usz n) {
  // returns a pointer to an objects child, so that child can be accessed or mutated
  assert(!gc_isFwded(obj));
  assert(n < gc_nChildren(obj));
  void* addr = (void*)obj  // base of the object
    + sizeof(a_gcHdr)      // skip the header
    + n*sizeof(a_gcObj*);  // skip `n` children
  return (a_gcObj**)addr;  // double pointer b/c the address (pointer) we calculated holds a `gcObj*`
}

// accessing bytes
static inline
usz gc_nBytes(const a_gcObj* obj) {
  assert(!gc_isFwded(obj));
  return obj->as.hdr & 0xFFFD;
}
static inline
byte* gc_byte(const a_gcObj* obj, usz off) {
  // returns a pointer to an object's raw byte at the offset, so that bytes starting there can be accessed or mutated
  assert(!gc_isFwded(obj));
  assert(off < gc_nBytes(obj));
  void* addr = (void*)obj                 // base of the object
    + sizeof(a_gcHdr)                     // skip the header
    + gc_nChildren(obj)*sizeof(a_gcObj*)  // skip all the children
    + off;                                // skip `off` bytes
  return (byte*)addr; // single pointer b/c the address (pointer) we calculated holds a `byte`
}

// accessing the finalizer
static inline
bool gc_hasFini(const a_gcObj* obj) {
  assert(!gc_isFwded(obj));
  return obj->as.hdr & 1;
}
static inline
gcFini* gc_getFini(const a_gcObj* obj) {
  assert(!gc_isFwded(obj));
  assert(gc_hasFini(obj));
  void* addr = (void*)obj                 // base of the object
    + sizeof(a_gcHdr)                     // skip the header
    + gc_nChildren(obj)*sizeof(a_gcObj*)  // skip all the children
    + gc_nBytes(obj);                     // skip `off` bytes
  return (gcFini*)addr; // single pointer b/c the address (pointer) we calculated holds a `gcFini`
}

// internals inspection
static inline
usz gc_objSize(const a_gcObj* obj) {
  assert(!gc_isFwded(obj));
  return sizeof(a_gcHdr)
       + gc_nChildren(obj)*sizeof(a_gcObj*)
       + gc_nBytes(obj)
       + gc_hasFini(obj)*sizeof(gcFini);
}

////// Introduction Rules //////

static inline
a_gcObj gcNewHdr(struct a__gcNewParams kwargs) {
  assert(kwargs.nChildren < 0x10000);
  kwargs.nBytes = a_alup(kwargs.nBytes, 4);
  assert(kwargs.nBytes < 0x10000);
  a_gcObj out;
  out.as.hdr
    = ( kwargs.nChildren    << 16 ) // 16-bit number of children
    | ( (kwargs.nBytes / 4) << 2  ) // 16-bit number of bytes, aligned to 4 bytes
    | ( 1                   << 1  ) // constant for regular object
    | ( kwargs.fini != NULL       ) // set to one if there's a finalzer
    ;
  return out;
}

a_gcObj* a__gcNew(struct a__gcNewParams kwargs) {
  a_gcObj tmpObj = gcNewHdr(kwargs);
  usz objSz = gc_objSize(&tmpObj);
  // TODO probably enforce some kind of maximum object size
  a_gcObj* obj = gcAlloc(objSz);
  if (obj == NULL) {
    // TODO collect
    fprintf(stderr, "unimplemented: gc collection\n");
    exit(1);
    obj = gcAlloc(objSz);
    if (obj == NULL) {
      // the most general would be to parameterize the handler,
      // but panic is good enoguh for now.
      // I suppose there's only a few useful options:
      //   panic,
      //   return NULL (which then requires all callers to check or risk UB),
      //   or make a longjump (yikes), or
      //   call a noreturn function.
      fprintf(stderr, "Out of GC Memory\n");
      exit(1);
    }
  }
  // initialize header
  obj->as.hdr = tmpObj.as.hdr;
  // TODO maybe I should zero out the children and bytes, at least offer it as an option in the kwargs
  // initialize the object's finalizer
  if (kwargs.fini != NULL) {
    gcFini* objFini = gc_getFini(obj);
    objFini->fini = kwargs.fini;
    // add register this object as finalizable
    objFini->next = gc.finiList;
    gc.finiList = obj;
  }
}

////// Collection //////

/// For simplicity, it's a non-generational compacting garbage collector.
/// This strategy means that allocation and deallocation are fast, and related data tends to end up in the same place, but collection can cause pauses.
/// If you don't like the pauses, use this gc sparingly or notat all.
/// After all, this is a gc embedded in C for use from C.
/// If this were for a language runtime, more complex systems could yield useful performance improvements.
///
/// Under the hood, the gc engine consists primarily of two bump allocators (one active, one that we are collecting from), a root list, and ofc a handful of global state.
/// Collection proceeds as:
/// - swap active and stale buffers
/// - move rootlist-mentioned objects to the active buffer:
///   - memcopy the object from stale to active
///   - set the stale obj to a forwarding pointer
/// - use the active buffer as the list of objects to trace:
///   - ie the active buffer before a certain point contains traced&updated objects, and after that contains objects that have been moved but not traced/updated
///   - iterate over the children in the active buffer (which may be extended during iteration):
///     - if the child is in the active buffer:
///       - skip
///     - if the child is a forwarded object in the stale buffer:
///       - replace the pointer in the active buffer with the forwarding pointer
///     - if the child is an unforwarded object in the stale buffer:
///       - memcopy the child into the active buffer (which extends what we have to iterate over)
///       - set the stale child to a forwarding pointer
/// - run any finalizers:
///   - the engine data will hold a (nullable) finalizable object which is the root of a linked list of such objects
///   - iterate over the finilizable list, keeping the previous node:
///     - each object in the node will always be stale, because we don't update the gcFini during tracing
///     - if the object if forwarded:
///       - update the previous node to point to the active copy
///     - if the object is not forwarded:
///       - run the finalizer
///       - delete the node (do not "increment" the previous node variable)

///////////////////
////// Bytes //////
///////////////////

int a_cmp_Bs(A_Bytes a, A_Bytes b) {
  size_t minlen = min(a.len, b.len);
  int ord = strncmp((char*)a.str, (char*)b.str, minlen);
  if (ord < 0) { return -1; }
  else if (ord  > 0) { return 1; }
  else {
    if (a.len < b.len) { return -1; }
    else if (a.len > b.len) { return 1; }
    else { return 0; }
  }
}

bool a_eq_Bs(A_Bytes a, A_Bytes b) {
  if (a.len != b.len) { return false; }
  return 0 == strncmp((char*)a.str, (char*)b.str, a.len);
}

A_Asciiz a_AsciifromBytes(A_Bytes bytes) {
  for (iptr i = 0; i < bytes.len; i++) {
    byte b = bytes.str[i];
    if ((b == 0) | (0x7F < b )) {
      return a_mk_bs(i, bytes.str);
    }
  }
  return bytes;
}

void a_CStrFromAsciiz(CStr restrict dst, A_Asciiz src) {
  memcpy(dst, src.str, src.len);
  dst[src.len] = '\0';
}
