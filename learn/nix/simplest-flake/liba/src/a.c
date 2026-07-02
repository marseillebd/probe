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
