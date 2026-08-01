// Copyright 2026 - 2026, Marseille Bouchard
// SPDX-Liense-Identifier: LGPL-3.0-or-later

/// # Lib A
///
/// My (Marseille Bouchard's) utility library for C.
/// It's meant to help prototype C code quickly, and nothing more.
/// If anyone else wants to use it, then ig that's cool.
///
/// Specifically, Lib A should:
///
/// - mitigate the develop-time overhead of ubiquitous manual memory management
/// - fill in some basic data structures and utilities that other languages take for granted
/// - provide little syntactic niceties, normalize naming conventions
/// - make _me_ happy when I play around with C
///
/// It is NOT meant to:
///
/// - over-abstract (ie no generic allocator interface; just use malloc)
/// - be portable
/// - minimize dependencies (if gmp or unistd comes in handy, I'll use it)
/// - achieve maximum performance (speed, efficiency, memory footprint, binary size, build time, or whatever else)
/// - reinvent the wheel
/// - worry about backwards- or forwards-compatibility (just vendor the version you want)
///
/// C is an incredibly popular, well-supported language, and it pains me to say it, but
///   that popularity has significant advantages.
/// Of course, C is scuffed, and needs some real help.
/// There's only so much I can do, but liba will try to do what it can.
///

/////////////////////
/// # Metalibrary
/////////////////////
///
/// Y'know, the boring stuff that lets user of the library identify it.
///
/// The definitions here are _not_ avalable without the `A_` prefix.
///

#ifndef A_H
#define A_H

/// ## Version
///
/// - `A_VERSION`: a human version name, following `PROJECT.MAJOR.MINOR.PATCH`, perhaps with an `-alpha` or `-rcXXXX` suffix, or the like.
///   Semantic and Romantic versions are left for later, if ever.
/// - `A_RELEASE`: the release date, as an integer encoded `YY,YYM,MDD`.
///   If you aren't as worried about guessing compatibility, this is the machine identifier.
///   Since liba isn't as worried about compat as of writing, this is what we probably want for filtering versions programmatically.
///
#define A_VERSION_PROJECT 1
#define A_VERSION_MAJOR 0
#define A_VERSION_MINOR 0
#define A_VERSION_PATCH 2
#define A_VERSION "1.0.0.2-alpha"
#define A_RELEASE 20260627

/// ## Dialect
///
/// We assume the compiler is gcc, using the C23 standard (with at least some gnu extensions).
///

/// TODO: I'd love a decent way to mark pointers as non-null.
/// Gcc's attribute is kinda awful.

/// ### Syntax via Preprocessor
///
/// I generally avoid special-use syntax, but for C, I'll make an exception.
/// These bits of syntax are not prefixed.
/// Some of them are even shims (ok, just `defer` atm).
///

/// #### Inline
///
/// C `inline` can be quite annoying.
/// You'll likely need to emit exactly one linkable definition of an inline function in an executable.
/// You'll define the inline function in a header which will likeyl be included multiple times.
/// Some people use `static inline`, which will bloat the executable without a sufficiently smart toolchain.
/// You could also use `extern inline` in the header, but then we still need a plain `inline` somewhere,
///   but it needs _the exact same_ definition, which very meaningfully violates DRY.
///
/// I've decided to use an STB-/single-header-library-inspired approach.
/// This header defines `external inline` functions, except when `A_IMPLEMENTATION` is defined,
///   in which case the definitions become plain `inline`.
/// This way, ordinary header use need not worry about eitting extra symbols,
///   there will naturally be one file that emits the standalone function for linking.
///   and no extra (unlinked) copies will be emitted.
///
/// Liba devs should use A__INLINE instead of `inline` or its variants.
/// TODO actually, what if I have smth like A_INLINE which can be durned on by define A_EMIT_INLINE_DEFS?
///

#ifdef A_IMPLEMENTATION
  #define A__INLINE inline
#else
  #define A__INLINE extern inline
#endif

/// #### `defer`
///
/// The ["defer" technical spec][defer-ts] (TS 25755, aka WG14 Working Draft N3734)
/// will be an invaluable addition to the C language, and I want to use it right away.
/// Thankfully, GCC can implement it with extensions and clang 22 supports the spec with `-fdefer-ts`.
///
///
/// The implementation is based off the sample from [Jens Gustedt's blog][jgustet-defer].
///
/// [defer-ts]: https://www.open-std.org/JTC1/SC22/WG14/www/docs/n3734.pdf
/// [jgustedt-defer]: https://gustedt.wordpress.com/2026/02/15/defer-available-in-gcc-and-clang/

// Section 6.5 specifies this macro do be defined.
// If so, we simply include the header.
#if defined(__STDC_DEFER_TS25755__)
  #include <stddefer.h>
// Let's imagine that we can import `stddefer.h` despite the lack of feature macro.
// Then let's include it, and hope for the best, with a little error message if it doesn't seem to work.
#elif __has_include(<stddefer.h>)
  #include <stddefer.h>
  #if defined(__clang__)
    #if __is_identifier(_Defer)
      #error "clang may need the option -fdefer-ts for the _Defer feature"
    #endif
  #endif
// If there's no direct compiler support for `defer`, gcc is able to simulate it accurately
//   using nested functions and the cleanup attribute.
#elif __GNUC__ > 8
  #define defer a__defer
  #define a__defer a__deferShim(__COUNTER__)
  #define a__deferShim(N)                                                      \
    /* forward-declare a cleanup function */                                   \
    auto void A_TOKEN_PASTE(a__deferF, N)(int*);                               \
    /* Now, declare a dummy variable */                                        \
    /* and use the `__cleanup__` attribute to register our cleanup function */ \
    /*   to run when the dummy goes out-of-scope. */                           \
    __attribute__(( __cleanup__(A_TOKEN_PASTE(a__deferF, N)),                  \
                    __deprecated__, __unused__                                 \
                 ))                                                            \
    int A_TOKEN_PASTE(a__deferV, N);                                           \
    /* Finally, "define" our cleanup function */                               \
    /* This has to be last so that its definition actually comes */            \
    /*   from the block following the macro call. */                           \
    __attribute__((__always_inline__, __deprecated__, __unused__))             \
    inline auto void                                                           \
    A_TOKEN_PASTE(a__deferF, N)(int*)
// We have no more fallbacks, but I suspect I'll be using defer in my code anyway.
// So, we fail early.
#else
  #error "The defer feature seems not available"
#endif

/// #### Ensure Macros
/// `A_ENSURE(cond, msg)` and `a_ensure(cond, msg)` fail to compile when `cond` is not true at compiletime.
/// The error message will include `msg` as part of an identifier.
///
/// `A_ENSURE` is valid where typedefs are, and `a_ensure` is valid as an expression (which evaluates to zero).
///

# define A_ENSURE(cond, msg)                                     \
  typedef char A_TOKEN_PASTE(a__ensure_,msg)[(cond) ? 1 : -1];

#define a_ensure(cond, msg)                                      \
  (0 * (int)sizeof(struct {                                     \
    char A_TOKEN_PASTE(a__ensure_,msg)[(cond) ? 1 : -1]; \
  }))

/// #### `isArray`
/// `a_isArray(a) fails at compiletime if `a` is not an identifier or expression of array type.
/// It is based on `a_ensure`, so it can be used wherever it would be.
///
/// The implementation is based on [n3325](https://www.open-std.org/JTC1/SC22/WG14/www/docs/n3325.pdf)
///

#define a__isSameTypeof(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define a__decay(a) (&*(a))
#define a_isArray(a) (!a__isSameTypeof((a), a__decay(a)))

/// #### `lengthof`
/// `a_lengthof(some_array)` computes the number of elements in an array.
///
/// There's a chance something like this gets added to C2y, but I'm not sure what name they'll choose.
/// So, this dialect macro takes the `a_` prefix.
#define a_lengthof(x) ((sizeof(x) / sizeof(x[0])) + a_ensure(a_isArray(x), isArray))

/// #### `match`
///
/// `match (condition, declaration) { body... }` will evaluate `body` with `declaration` in-scope only when `contition` is true.
/// `match` statements can have `else` clauses, and thus `else match` clauses.
/// Unfortunately, this will mess with the expected behavior of `break` inside the body;
///   use break-to-label inside the body i fyou must.
/// Likewise for `continue`.
///
/// This is useful for simple pattern-matching, like `match (ptr != NULL, int value = *ptr) { ... }`.
#define match(cond, decl) a__match(cond, decl, __COUNTER__)
#define a__match(cond, decl, N) \
  if (cond) \
    for (bool A_TOKEN_PASTE(stop,N) = true; A_TOKEN_PASTE(stop,N);) \
      for (decl; A_TOKEN_PASTE(stop,N); A_TOKEN_PASTE(stop,N) = false)
///

/// #### `pass`
///
/// Explicit syntax indicating "this statement intentionally left-blank".
#define pass do {} while (0)
///

/// #### `noreturn`
///
/// Which just comes from `stdnoreturn.h`.
#include <stdnoreturn.h>
///

/// #### `impossible()`
/// If control flow reaches here, panic with a diagnostic (function name + line number) message.
///
#define impossible() ({                                                  \
  fprintf(stderr, "[Impossible!] %s: line %d\n", __func__, __LINE__);    \
  exit(1);                                                               \
  NULL;                                                                  \
})

/// ## Default Headers
///
/// These headers come free with your freaking xbox.
///

/// C is full of footguns. Better program defensively.
/// `assert`, `errno`, `stdckdint`.
#include <assert.h>
#include <errno.h>
#include <stdckdint.h>
///

/// A bunch of ordinary C is hidden behind includes.
/// `stddef`, `stdalign`, `stdlib`
#include <stddef.h>
// - NULL
// - size_t, ptrdiff_t, max_align_t
// - ssize_t comes from unitstd
// - offsetof(type, member)
#include <stdalign.h>
// - align{of,as}
///

/// C has a vast array of integral types and related constants.
/// `stdbool`, `limits`, `stdint`
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
///

/// C is nothing without arrays.
/// `string`, `stdlib` for malloc.
#include <string.h>
#include <stdlib.h>
///

/// It's so normal to do I/O everywhere in C, **sigh**.
/// `stdio`, `inttypes`
///   esp for the `{PRI,SCN}{d,i,u,o,x}{{,LEAST,FAST}{8,16,32,64},MAX,PTR}` macros
#include <stdio.h>
#include <inttypes.h>
///

/// I may as well accept POSIX as "portable".
/// At this point the only holdouts are embedded systems (with good reason),
///   and Windows (with no good reason, unless you count billionare greed).
/// Well, liba isn't designed for embedded, and I refuse to bend the knee to corporate interests.
///
/// At the moment, I'm just including `unistd` wholesale, and I'll learn more tidbits about the API later.
#include <unistd.h>
///


/// ### Recommended APIs
///
/// TODO: document this more than just listing the APIs
/// TODO: also, recommend normalized names

/// ## Naming Conventions
///
/// Mostly, there is one "default" convention, but there are some exceptions.
/// Some interfaces depart because they are "comonplace": meant to be used often or as pseudo-builtins.
/// Conversion functions also use their own name template.
/// Beyond that:
/// - Avoid abbreviations that aren't listed [here](#known-abbreviations).
///   Abreviations are never as universal as you expect, and sometimes they even conflict (eg `doc{ument,tor}`).
/// - Use units in identifiers wherever applicable.
///   I'm so tired of not knowing if it's seconds, milliseconds, or nanoseconds without looking at the manual.
///   Prefer placing it after an underscore, and use SI abbreviations.
///   If the base name would just be something like `n`, `count`, `length`, &c, camelCase is fine (eg `nBytes`).
///   Try to use units like `kph` or `ppm` for units that have a denominator, but use `_per_` when a common unit isn't available.
///   Obvs, micro- is `u`.
///

/// The default name template is `<lib prefix><flags><Name><type/module>`.
/// - `Name` is, well, the core name of the interface.
///   They are in `camelCase` (because underscore is less convenient to type).
///   Functions, function-like macros, and macro-based syntax use `lowerCamelCase`.
///   "Primitive" types (ie unboxed, register-size, concrete) use `lowerCamelCase`.
///   "Abstract" types (ie compound, layout hidden or subject to change) use `UpperCamelCase`.
/// - The `lib prefix` is `a_` or `A_`, depending on the case convention of `Name`.
///   Users can make the prefix optional in their sourcecode by defining [`A_NOPREFIX`](#strip-library-prefix).
///   Private interfaces which nonetheless must be in the header use an extra underscore, and never have their prefix stripped.
/// - `flags` is meant to indicate variations on an interface.
///   For example, `fprintf` and `sprintf` as variations on `printf`,
///     or `malloc`, `calloc`, and `talloc` as variations on `alloc` (cf `strcpy` vs `strncpy`).
///   Usually, these are single lowercase letters, but in the inadvisable circumstance that you need multiple flags,
///     each should be a single lowercase letter, they should be ordered allphabetically, and the group should be separated from `Name` by an underscore.
///   Please document what each flag means, probably on the main interface.
/// - `type/module` disambiguates what module the function comes from, or what type it primarily operates on.
///   If it exists, it should be separated from `Name` by an underscore.
///   This is literally because C has a type system and ABI too simple to handle modules or any kind of overloading.
///   Since the reader can usually guess this from context, it's placed at the end where it's less salient.
///

/// The template for conversion functions is `<lib prefix><X>From<Y>`.
/// - `lib prefix` is just as in the default convention.
/// - `X` and `Y` are just the (core) names of types, both in `UpperCamel`.
/// - We use literal `From` always instead of `To` for a) uniformity,
///   and b) the target type is likely more important and so takes the more salient location.
///

/// Commonplace names are just `camelCase`.
/// They don't have a lib prefix or a module.
/// The might use `UpperCamel` for types, just as in the default convention,
///   but they shouldn't be compound, just boxed (eg `CStr`).
///

/// ### Strip Library Prefix
///
/// Define `A_NOPREFIX` to expose most interfaces in this library without this prefix (in the API, not ABI).
/// Eg `a_talloc` can now be written `talloc`, but `a_talloc` still exists and is what gets linked against.
///

#ifdef A_NOPREFIX

  #define STR          A_STR
  #define TOKEN_PASTE  A_TOKEN_PASTE
  #define ENSURE       A_ENSURE
  #define ensure       a_ensure
  #define isArray      a_isArray
  #define lengthof     a_lengthof

  #define FUND_ALIGN   A_FUND_ALIGN
  #define pow2         a_pow2
  #define isPow2       a_isPow2
  #define alup         a_alup
    #define alup_i     a_alup_i
    #define alup_s     a_alup_s
    #define alup_u     a_alup_u

  #define talloc       a_talloc
  #define tallocSave   a_tallocSave
  #define tallocReset  a_tallocReset

  #define Bytes         A_Bytes
    #define mk_bs       a_mk_bs
    #define lit_bs      a_lit_bs
    #define spread_bs   a_spread_bs
    #define spreadr_bs  a_spreadr_bs
    #define BsFromCStr  a_BsFromCStr
    #define cmp_bs      a_cmp_bs
    #define eq_bs       a_eq_bs
  #define Asciiz             A_Asciiz
    #define AsciizFromBytes  a_AsciizFromBytes
    #define cstrLen_Asciiz   a_cstrLen_Asciiz
    #define CStrFromAsciiz   a_CStrFromAsciiz
    #define tCStrFromAsciiz  a_tCStrFromAsciiz
    #define mCStrFromAsciiz  a_mCStrFromAsciiz

  #define vec2f32 a_vec2f32
    #define dot_vec2f32 a_dot_vec2f32
  #define vec2f64 a_vec2f64
    #define dot_vec2f64 a_dot_vec2f64
  #define vec3f32 a_vec3f32
    #define dot_vec3f32 a_dot_vec3f32
  #define vec3f64 a_vec3f64
    #define dot_vec3f64 a_dot_vec3f64
  #define dot a_dot

  #define debug a_debug
#endif

/// Some definitions (ie configuration macros) do not have their prefix stripped.
///

/// ### Known Abbreviations
///
/// - `id`: identifier
/// - `ix`: index, but not for `i`, `j`, `k` (I've seen `idx` and `ndx`, but those are longer for no clear reason)
/// - `ctx`: context (which is kinda meaningless without its own context)
/// - `str`: string
/// - `buf`: buffer
/// - `len`: length
/// - `cap`: capacity
/// - `ptr`: pointer
/// - `alloc`: allocate
/// - `lit`: literal
/// - `mk`: make, cf "new", "init"
/// - `init`: initialize
/// - TODO fini, del, clr

/// ## Token Pasting and Stringizing

/// Token pasting is somewhat fraught.
/// `A_TOKEN_PASTE_DIRECT` does not work within macros, because its arguments are not expanded.
/// Similarly with using the `##` operator directly.
/// I can't actually cite a source, but it seems to be the case.
#define A_TOKEN_PASTE_DIRECT(t1, t2) t1##t2
/// Since token pasting often involves generating a token from builtins such as `__LINE__`,
///   we usualy want to use `A_TOKEN_PASTE` instead, even though it looks like it shouldn't do anything.
#define A_TOKEN_PASTE(t1, t2) A_TOKEN_PASTE_DIRECT(t1, t2)
///

/// Similarly, stringizing is fraught.
/// Use `A_STR` instead of `A_STR_DIRECT` or plain `#`.
#define A_STR_DIRECT(x) #x
#define A_STR(x) A_STR_DIRECT(x)

/// ## Normalize Names
///
/// Probably useless, but it's easy.
/// And after all, there are a lot of annoying things going on with C's basic types.
///

///
/// The definitions here do _not_ use the `A_` prefix.
///

/// ### Numeric Data Types
///
/// Some are built-in, some use the awkward-to-type `_t` suffix, and some are just misnomers via history.
/// Most don't abide by my naming conventions.
/// Worst of all, many of the types that are often preferred are actually more difficult to type.
///

/// I've seen the argument that general use of unsigned types is a mistake.
/// I'm honestly not sold completely; there are fundamental limitations to ergonomics of C (and by inheritance it's competitors).
/// However, this _is_ C, so I'll give it a go.
/// Mostly, signed types are like `int`, where unsignes are like `uint`.
/// Where there's divergence from this trend, I'll call it out.
///

/// ##### `bit`
///
/// For a single bit, stored in a byte.
///  Effectively C `bool`, but the name calls out boolean blindness.
typedef bool bit;
///
/// ##### `bool`
/// `bool` is already an ordinary C type, but don't forget about boolean blindness!
/// Try [`bit`](#bit) or a self-describing enum instead.
///
/// ##### `octet`
///
/// An octet is specifically 8 bits, unsigned.
/// C's `char` is `CHAR_BITS >= 8` bits, and with implementation-defined signedness.
typedef uint8_t octet;
///
/// ##### `byte`
/// A `byte` is an unsigned `char`, which may or my not be 8-bits, even though we can generally assume it is;
///   I distinguish it from `octet` mostly because I like being very clear with the reason behind my types.
/// Also, C's `char` type---normally used to indicate bytes---has implementation-defined signedness, which just complicates everything.
/// That said, you may need to cast `byte` to `char` sometimes to avoid compiler warnings.
typedef unsigned char byte;
///

/// ##### `isz`, `usz`, `ssz`
/// The notion of "size", like the maxium size of a memory object, varies between computers.
/// Thus we use these types for portability.
///
/// The difference between `ssz` (`s` for signed) and `isz` is that
///   `isz` is able to hold the signed difference between two pointers (ie, it's `ptrdiff_t`).
/// Meanwhile, `ssz` comes from Posix, and holds either a positive size, or a `-1` as a sentinel (for eg errors).
/// My guess is that this preserves the usual `u-` vs `i-` semantics, while `ssz` can still do its particular job,
///   but really this depends on `ptrdiff_t` and `size_t` having the same number of bits.
///
/// I'm not entirely sold on signed size types, but I'm willing to give it a shot,
///   and that's why I'm not preferring one to the other.
typedef size_t usz;
typedef ptrdiff_t isz;
typedef ssize_t ssz;
///

/// ##### `iptr`, `uptr`
/// These are integer types large enough to hold pointers, thus enabling advanced pointer arithmetic.
/// I honestly don't know which should be preferred, so they have `u-` vs `i-` prefixes rather than favoring one.
typedef uintptr_t uptr;
typedef intptr_t iptr;
///

/// ##### Fixed-size integer types
/// `{int,uint}{8,16,32,64,128}` are all exact-width signed/unsigned integer types, with no defined endianness.
///
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef unsigned __int128 uint128; // gcc-specific?

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef __int128 int128; // gcc-specific?

/// ##### TODO floating data types
/// `f{32,64}` are IEEE754 binary floading point types.
/// TODO: I'd love to add f16 and f128, maybe f80
///
typedef float f32;
typedef double f64;

/// ### Idiomatic C Types
///
/// At least some commonly-used C types are structural, meaning they don't specify their intent very well.
/// This section provides synonyms that halp clarify intent.
///

/// ##### `CStr`
///
/// This is a synonym for `char*`, but explicitly intended as a nul-terminated string.
typedef char* CStr;
///

/// #### Alignment
///
/// Alignment can be very important in C, but usually uses size types.
/// This set of numeric types indicate that these "sizes" are actually alignments, and should be powers of 2.
///
/// I also include some helper constants and functions.

/// ##### `ial`, `ual`, and `sal`
///
/// We follow the pattern of the size-types, since alignment appears in the same way.
typedef isz ial;
typedef usz ual;
typedef ssz sal;
///

/// ##### `FUND_ALIGN`
/// The fundamental alignment is "the strictest alignment of any type"
///   (though it appears GCC doesn't count SIMD types).
/// It is `A_FUND_ALIGN`, which is easier to read and remember than `alignof(max_align_t)`
///   (I literally already tried `sizeof(...)` just a few days after reading the definition).
///
/// TODO: I beleive max_align_t must has a power-of-two alignment, but I'm not sure
#define A_FUND_ALIGN alignof(max_align_t)
///

/// TODO alignment helpers: ensure one-hot or power-of-2, make power-of-2

/// ##### `pow2`
/// I'm really uncertain about using this name, but,,,
/// `a_pow2(n)` computes 2^n.
/// It's probably UB to pass anything negative or larger than the maximum possible alignment.
#define a_pow2(n) (1 << (n))
///

#include <stdbit.h> // TODO mote this to the imported headers
#define a_isPow2(inp) (1 >= stdc_count_ones(inp))

/// ##### `SZ alup(SZ theSize, SZ alignTo_B)`
/// `a_alup` increases a size type until it is matches the given alignment.
/// It is a generic over `a_alup_u`, `a_alup_i`, and `a_alup_s`.

#define a__mkAlup(ty, name)                           \
  A__INLINE                                           \
ty name(ty theSize, ty alignTo) {                     \
  assert(a_isPow2((ual)alignTo));                     \
  ty loMask = alignTo - 1;                            \
  return theSize + (loMask & -(theSize & loMask));    \
}
  //^ `-(theSize & loMask)` determines how far the lo-bits are from the next requested power of 2.
  //^ The final `loMask &` handles the already-aligned case by trimming off any carry-out beyond the lower bits.

a__mkAlup(ial, a_alup_i)
a__mkAlup(sal, a_alup_s)
a__mkAlup(ual, a_alup_u)
#define a_alup(theSize, alignTo) \
  (_Generic(theSize,             \
    usz     : a_alup_u,   \
    isz     : a_alup_i,   \
  default : _Generic(theSize,    \
    ssz     : a_alup_s,   \
    default : a_alup_i    \
  ))(theSize, alignTo))

/////////////////////
/// # Size Macros
/////////////////////
///
/// It feels silly, but these macros can be used after a number to scale it.
/// Right now, I only need abouts of bytes, so that's what I'm going with.
///
/// - `B`
/// - `KiB`
/// - `MiB`
/// - `GiB`
#define B
#define KiB * 1024 B
#define MiB * 1024 KiB
#define GiB * 1024 MiB
///

/////////////////////////////
/// # Temporary Allocator
/////////////////////////////
///
/// A dynamic allocator off-stack that can be quickly cleared/reset.
/// Meant for temporary data, like buffering a dynamic array or something.
///

/// ##### `A_TALLOC_SIZE`
///
/// Define `A_TALLOC_SIZE` to set the maximum number of bytes that can be held in temporary allocator.
/// If not set, the default is 8MiB. I have no reason for this number.
#ifndef A_TALLOC_SIZE
  #define A_TALLOC_SIZE (8 MiB)
#endif
///

/// ##### `talloc`
///
/// `void* a_talloc(size_t nBytes)`
void* a_talloc(size_t nBytes);
///
/// Allocate `nBytes` in the temporary allocator.
/// Returns `NULL` if there's an error.
///
/// All allocations are aligned to the fundamental alignment (ie `alignof(max_align_t)`).
/// A zero-byte talloc is allowed, and its address will be distinct from any other talloc object (it will, ofc, use some memory).
///

/// #### Save and Restore Temp Allocator
///
/// The functions `a_talloc{Save,Reset}` work in tandem.
/// The save function returns an opaque handle (`A_Savepoint`), and the reset function accepts it.
/// The savepoints are structured as a stack.
///
/// Short story long,
/// a savepoint value may only be used once (by reset), but not be reused.
/// Additional savepoints are also invalidated: when they were created between (runtime-wise) the creaation and usage of some other savepoint.
///

/// ##### `Savepoint`
/// `typedef void* A_Savepoint`
typedef void* A_Savepoint;
///
/// ##### `tallocSave`
/// `A_Savepoint a_tallocSave()`
A_Savepoint a_tallocSave();
///
/// ##### `tallocRestore`
/// `void a_tallocReset(A_Savepoint base)`
void a_tallocReset(A_Savepoint base);
///

//////////////////////////////
/// # Byte Strings (`_bs`)
//////////////////////////////
///
/// Nul-terminated strings have some clear issues.
/// If the string cannot contain NUL or the zero byte.
/// If the NUL is missing, buffer overreads occur.
/// Creating a substring requires an allocation and copy.
/// Operations as common as length are O(n).
///
/// Here, we take the "string view" approach: a length and pointer to string.
/// This also means that these strings will usually be borrows of an underlying memory object,
///   and that object should outlive all `Bytes` pointing into it.
///
/// Of course, the downside to using strings that aren't C strings is interoperation with system libraries.
/// It _could_ be frustrating to constantly convert representations, _if_ you do more system calls than copmutation.
/// Even then, every other language on the planet has alraedy wrapped these syscalls to accept input from pleasant string interfaces,
///   and there's absolutely no reason C cuoldn't do the same.
///
/// Finally, note that this interface is _not_ intended for text, and certainly not unicode.
/// It's a happy accident that you _could_ use this for ascii text directly,
///   if you're willing to accept the type system not stopping codes 0x80 and above,
///   and as long as you don't assume one-column-one-character.
/// As for unicode, while this could be a backing type for utf8 and its variations,
///   traversing and manipulating unicode text is a sophisticated operation that does not lend itself to an array-like interface.
/// Consider something like streaming transformations and grapheme cluster cursors instead.
///
/// Just a note on the arguent order in this API: the count convensionally comes before the data.
/// This mirrors the order in plain old existential types `exists (n :: Integer). Ptr (Array n Byte)`.
/// Even in C, this mirrors the order expected in VLA parameters.
///

/// #### Introduction and Elimination Forms
/// ##### `Bytes`
///
/// ```
/// typedef struct A_Bytes {
///   iptr len;
///   byte* str;
/// } A_Bytes;
/// ```
typedef struct A_Bytes {
  // `iptr` to help manage alignment portably;
  // ie this struct is always two pointer-sized values, which likely each fit in a register
  iptr len;
  byte* str;
} A_Bytes;
///
/// I'm hoping that most ABIs have a way to pass/return two-word data types in registers, since I expect this to be used by-value.
///
/// Ultimately, this type is intended as a mutable (but not growable) view/slice.
/// It _may_ also contain an entire string, but I'm sure it will often contain substrings.
///

/// It's common enough to need to wrap or unwrap an existing `CStr`, so it warrants from syntactic sugar.
///
/// ##### `mk_bs`
/// `a_mk_bs(len, ptr)` creates the struct from length and pointer, in that order.
#define a_mk_bs(n, ptr) ((A_Bytes){ .len = (n), .str = (ptr) })
///
/// ##### `lit_bs`
/// `a_lit_bs(strLiteral)` creates the struct from a literal string (and will not include the trailing NUL that C inserts and we do not need or want.
#define a_lit_bs(str) a_mk_bs(sizeof(str) - 1, (str))
///

/// ##### `spread_bs`
/// `a_spread_bs(Bytes)` disassembles the Bytes struct to pass to C functions that take the pointer and count separately.
///
/// It puts the length before the string, which matches the order of the `%.*s` printf format specifier.
/// It also would match libraries who take strings as VLAs, like `foo(int n, char[n] data)`.
///
/// `a_rspread_bs` is for interacting with libraries that take them in reverse order for whatever reason.
#define a_spread_bs(bytes) (bytes.len), (bytes.str)
#define a_rspread_bs(bytes) (bytes.str), (bytes.len)
///

/// #### Conversions
///
/// Conversions back _and_ forth between `Bytes` and C-strings would be nice.
/// Unfortunately, `Bytes` is able to hold NUL, which break C-strings, so we can't convert cleanly to C-strings.
/// We can at least convert C-strings to `Bytes`, through [`mk_bs`](#mk_bs), but we also provide this operation in function form.
///
/// If you _do_ have, say, a nul-terminated ascii `Bytes` that you need as a `CStr`,
///   see the `Asciiz` module, where you can create and convert an `Asciiz` type (either safely or cheaply).
///
/// ##### `BytesFromCStr`
///
/// `A_Bytes a_BytesFromCStr(CStr cstr)` converts a C-string to `Bytes`.
/// Just like `mk_bs`, `BytesFromCStr` will not include the trailing Nul in the length.
A__INLINE
A_Bytes a_BytesFromCStr(CStr cstr) {
  return a_mk_bs((iptr)strlen(cstr), (byte*)cstr);
}
///

/// #### Comparison

/// ##### `cmp_bs`
/// `int a_cmp_bs(A_Bytes a, A_Bytes b)`
int a_cmp_bs(A_Bytes a, A_Bytes b);
///   compares two byte strings lexicographically (not locale-sensitive),
///   returning `-1, 0, 1` for less-than, equal-to, and greater-than, respectively.
///
/// ##### `eq_bs`
/// `bool a_eq_bs(A_Bytes a, A_Bytes b)`
bool a_eq_bs(A_Bytes a, A_Bytes b);
///   tests two bytestrings for equality, and has a nicer interface in `if`-statements that only care about equality.
///

/// #### Bytes TODO

/// TODO isPrefix, isSuffix, findInfix, findInfix_r, findByte, findByte_r.
/// TODO handy syntax for infix/split iterating, trimming.
/// TODO trim chars left/right/both. (zero-init means use the default whitespace one).
/// TODO readline+-chomp.
///
/// TODO wrappers to interface with system calls that take strings
/// TODO a similar module for generic arrays (but I gotta figure out how to API unboxed w/size and boxed)
/// TODO a string builder module, so we can cat these and whatnot
///

/// ## Ascii (`_az`)
///
/// ##### `Asciiz`
/// `typedef A_Bytes A_Asciiz`
typedef A_Bytes A_Asciiz;
///   is just a synonym for [`Bytes`](#byte-strings-_bs),
///   but indicates the intention to hold nul-terminated Ascii text.
///

/// ##### `AsciizFromBytes`
/// `A_Asciiz a_AsciizFromBytes(A_Bytes bytes)`
A_Asciiz a_AsciizFromBytes(A_Bytes bytes);
///   will take the longest valid ascii prefix of `Bytes`.
///
/// Normally, valid ascii codepoints are 0x0--0x7F inclusive, but since we're in C,
///   we disallow NUL (the zero byte) from the string.
///
/// If the length of the returned `Asciiz` is the same as the input `Bytes`,
///   that indicates the entire `Bytes` was valid nul-terminated Ascii.
/// If less, then the length indicates how many bytes of the input were valid ascii,
///   and the next byte of the input is non-ascii.
///

/// ##### `CStrFromAsciiz`
/// While `Bytes` values cannot be safely converted to C-strings as discussed earlier,
///   `Asciiz` values are guaranteed (given it was made via `AsciizFromBytes` to avoid NUL, and can therefore be converted.
/// `void a_CStrFromAsciiz(CStr restrict dst, A_Asciiz src)`
void a_CStrFromAsciiz(CStr restrict dst, A_Asciiz src);
///   does exactly this.
/// To avoid malloc-ing, it takes an in-parameter which must hold the input ascii's length + 1.
///
/// See also: [`cstrLen_az`](#cstrlen_az), [`tCstrFromAsciiz`](#tSctrFromAsciiz), [`mCstrFromAsciiz`](#mSctrFromAsciiz).
///

/// ##### `cstrLen_az`
/// `size_t a_cstrLen_az(A_Asciiz str)`
A__INLINE
size_t a_cstrLen_az(A_Asciiz str) { return str.len + 1; }
///   makes it easier to avoid the classic off-by-one when allocating space for a C-string.
///

/// ##### `tCStrFromAsciiz`
/// `CStr a_tCStrFromAsciiz(A_Asciiz src)`
A__INLINE
CStr a_tCStrFromAsciiz(A_Asciiz src) {
  CStr dst = a_talloc(a_cstrLen_az(src));
  assert(dst);
  a_CStrFromAsciiz(dst, src);
  return dst;
}
///   converts an `Asciiz` to a `CStr`, allocating in the [temporary heap](#temporary-allocator).
/// ##### `mCStrFromAsciiz`
/// `CStr a_mCStrFromAsciiz(A_Asciiz src)`
A__INLINE
CStr a_mCStrFromAsciiz(A_Asciiz src) {
  CStr dst = malloc(a_cstrLen_az(src));
  assert(dst);
  a_CStrFromAsciiz(dst, src);
  return dst;
}
///   converts an `Asciiz` to a `CStr`, allocating in the main heap with `malloc`.
///

///////////////////////////
/// # Gemoetric Algebra
///////////////////////////
///
/// This module is a real work in progress.
/// Nevertheless, the intention is to have a unified interface for vectors, rotors, and other geometric objects in at least 2- and 3-dimensions.
///

/// ## Types
///
/// ##### `vec<dim><type>`
/// `a_vec<dim><type>` are the `dim`-dimentional vectors with components of `type`.
/// `dim` is in `{2,3}`, and `type` is in `f{32,64}`.
/// Each `vec` type has component names drawn from (in order) `x`, `y`, `z`, `w`.

#define a__mkvec2(ty)                        \
  typedef struct A_TOKEN_PASTE(a_vec2,ty) {  \
    ty x;                                    \
    ty y;                                    \
  } A_TOKEN_PASTE(a_vec2,ty)
#define a__mkvec3(ty)                        \
  typedef struct A_TOKEN_PASTE(a_vec3,ty) {  \
    ty x;                                    \
    ty y;                                    \
    ty z;                                    \
  } A_TOKEN_PASTE(a_vec3,ty)
a__mkvec2(f32);
a__mkvec2(f64);
a__mkvec3(f32);
a__mkvec3(f64);
///

/// ##### `dot_vec<dim><ty>`
/// `<ty> a_dot_vec<dim><ty>(vec<dim><ty> a, vec<dim><ty> avec<dim><ty> b)`
///   computes the dot product between two vectors.
#define a__mkdot_vec2(ty)                                                                      \
  A__INLINE                                                                                    \
  ty A_TOKEN_PASTE(a_dot_vec2,ty)(A_TOKEN_PASTE(a_vec2,ty) a, A_TOKEN_PASTE(a_vec2,ty) b) {    \
    return a.x*b.x + a.y*b.y;                                                                  \
  }
#define a__mkdot_vec3(ty)                                                                      \
  A__INLINE                                                                                    \
  ty A_TOKEN_PASTE(a_dot_vec3,ty)(A_TOKEN_PASTE(a_vec3,ty) a, A_TOKEN_PASTE(a_vec3,ty) b) {    \
    return a.x*b.x + a.y*b.y + a.z*b.z;                                                        \
  }
a__mkdot_vec2(f32)
a__mkdot_vec2(f64)
a__mkdot_vec3(f32)
a__mkdot_vec3(f64)
///

/// ##### `dot`
/// A generic dot product, dispatched based on the type of its inputs.
#define a_dot(a, b) _Generic(typeof(a), \
  a_vec2f32 : a_dot_vec2f32, \
  a_vec2f64 : a_dot_vec2f64, \
  a_vec3f32 : a_dot_vec3f32, \
  a_vec3f64 : a_dot_vec3f64  \
)((a), (b))
///

///////////////////////////
/// # Garbage Collector
///////////////////////////

/// TODO: document and test

// maxHeap is really how many KiB can be allocated until a collection is triggered.
// Pass zero to use the default (for now, I'm just saying 1GB)
bool a_gcInit(usz maxHeap_KiB);

typedef struct a_gcObj a_gcObj;

// The [Vjekoslav Krajacic method of keyword arguments in C](https://x.com/vkrajacic/status/1749816169736073295)
//   stores keyword arguments in a struct (where defaults are indicated by zero-initialization)
//   and uses a macro to arrange the struct building/passing for a pleasant code look.
struct a__gcNewParams {
  usz nChildren;
  usz nBytes;
  void (*fini)(a_gcObj*);
};
a_gcObj* a__gcNew(struct a__gcNewParams kwargs);
#define a_gcNew(...) (a__gcNew((struct a__gcNewParams){__VA_ARGS__}))

// You'll still need to manually manage a rootlist, since I'm not going to try tracing the stack (and wherever else root might be stored).
typedef struct a_gcRoot a_gcRoot;
a_gcRoot* a_gcNewRoot(a_gcObj* theRoot);
void a_gcMarkRoot(a_gcRoot* this, a_gcObj* rootObj); // nullable, which frees the root (ie don't use the root after it has been set to null, and you have to set it null before the root goes out of scope)

void a_gcCollect();

////////////////////
/// # Miscellany
////////////////////
///
/// ## Debugging Helpers
///
/// ##### `debug`
///
/// `a_debug(varname)` takes a variable name and prints the value in that variable to stderr, prefixed by the variable name and a colon.
/// The printf format is determined based on the type of the variable.
/// `char*` is printed as a (NUL-terminated) string, (signed) integers are printed in decimal, unsigned in hexadecimal, and floats in scientific notation.
/// All other variables are assumed to be of pointer type, for lack of a better way to dispatch on all pointers.
///
#define a_debug(x) fprintf(stderr, a__debug(x), (x))
#define a__debug(x) _Generic((x), \
  char*             : A_STR(x) ": %s\n", \
  signed char       : A_STR(x) ": %hhd\n", \
  short             : A_STR(x) ": %hd\n", \
  int               : A_STR(x) ": %d\n", \
  long              : A_STR(x) ": %ld\n", \
  long long         : A_STR(x) ": %lld\n", \
  unsigned char     : A_STR(x) ": %hhX\n", \
  unsigned short    : A_STR(x) ": %hX\n", \
  unsigned int      : A_STR(x) ": %X\n", \
  unsigned long     : A_STR(x) ": %lX\n", \
  unsigned long long: A_STR(x) ": %llX\n", \
  float             : A_STR(x) ": %f\n", \
  double            : A_STR(x) ": %f\n", \
  default           : A_STR(x) ": %p\n" \
)

/// ## Little Math Functions
///
/// ##### `max`, `min`, `clamp`
/// Classic `max`, `min, `clamp{,Hi,Lo}` that do exactly what you'd expect.
// FIXME: shouldn't I gensym the names? in case of nesting?
#define max(a, b) ({ \
  typeof(a) a__tmp_maxa = (a); \
  typeof(a) a__tmp_maxb = (b); \
  a__tmp_maxa >= a__tmp_maxb ? a_tmp_maxa : a__tmp_maxb; \
})
#define min(a, b) ({                                     \
  typeof(a) a__tmp_mina = (a);                           \
  typeof(a) a__tmp_minb = (b);                           \
  a__tmp_mina <= a__tmp_minb ? a__tmp_mina : a__tmp_minb; \
})
#define clamp(lo, x, hi) ({ \
  typeof(x) _lo = (lo);     \
  typeof(x) _x = (x);       \
  typeof(x) _hi = (hi);     \
  _lo > _x ? _lo :          \
  _hi < _x ? _hi :          \
             _x;            \
})
///
/// TODO: `clamp{Hi,Lo}`

/// ##### `ver`
/// The macro `ver(x)` computes `1 - x`.
/// I'm imagining `ver(sin(theta))` or `sqrt(ver(v*v / c*c))`.
/// Even `ver(sqrt(x))` might come up!
///
#define ver(x) ((typeof x)1 - (x))

#endif
