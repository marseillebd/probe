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
/// - `A_VERSION`: a human version name, following `PROJECT.MAJOR.MINOR.PATCH`, perhaps with an `-alpha` or `-rcXXXX` suffix
///   Semantic and Romantic versions are left for later, if ever.
/// - `A_RELEASE`: the release date, as an integer encoded `YY,YYM,MDD`.
///   If you aren't as worried about guessing compatibility, this is the machine identifier.
///   Since liba isn't as worried about compat as of writing, this is what we probably want for filtering versions programmatically.
///
#define A_VERSION_MAJOR 0
#define A_VERSION_MINOR 0
#define A_VERSION_PATCH 1
#define A_VERSION "1.0.0.1-alpha"
#define A_RELEASE 20260620

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

/// #### `pass`
///
/// Explicit syntax indicating "this statement intentionally left-blank".
#define pass do {} while (0)
///

/// #### `defer`
///
/// The ["defer" technical spec][defer-ts] (TS 25755, aka WG14 Working Draft N3734)
/// will be an invaluable addition to the C language, and I want to use it right away.
/// Thankfully, GCC can implement it with extensions and clang 22 supports the spec with `-fdefer-ts`.
///

// The following code is based off the sample from [Jens Gustedt's blog][jgustet-defer].
//
// [defer-ts]: https://www.open-std.org/JTC1/SC22/WG14/www/docs/n3734.pdf
// [jgustedt-defer]: https://gustedt.wordpress.com/2026/02/15/defer-available-in-gcc-and-clang/

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
                  ))                                                           \
    int TOKEN_PASTE(a__deferV, N);                                             \
    /* Finally, "define" our cleanup function */                               \
    /* This has to be last so that its definition actually comes */            \
    /*   from the block following the macro call. */                           \
    __attribute__((__always_inline__, __deprecated__, __unused__))             \
    inline auto void                                                           \
    TOKEN_PASTE(a__deferF, N)(int*)
// We have no more fallbacks, but I suspect I'll be using defer in my code anyway.
// So, we fail early.
#else
  #error "The defer feature seems not available"
#endif

/// #### `noreturn`
///
/// Which just comes from `stdnoreturn.h`.
#include <stdnoreturn.h>
///

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



/// ### Recommended APIs
///
/// TODO: document this more than just listing the APIs
/// TODO: also, recommend normalized names

/// ## Naming Conventions
///
/// 1. Identifiers use camelCase; UpperCamel for types, lowerCamel for values.
/// 2. Variations on a function (etc) may be indicated with a single-letter suffix after underscore.
///    When multiple letters stack up, order them alphabetically.
/// 3. Avoid abbreviations, unless they are common and mentioned in this section.
///    The single-letter function variants are technically exempt, but if there's a pattern for the function, then document it.
/// 4. Any variable which holds a measurement (say of time, bytes, and so on) should be suffixed with the SI abbreviation for its unit after an underscore.
///    Examples: `n_bytes`, `deltaTime_us`, `speed_kph`, `acceleration_m_per_s2`.

/// ### Known Abbreviations
///
/// - `str`: string
/// - `buf`: buffer
/// - `len`: length
/// - `cap`: capacity

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

/// - `Bit` for a single bit, stored in a byte.
///   Effectively C `bool`, but the name calls out boolean blindness.
typedef bool Bit;
/// - `Octet` is specifically 8 bits, unsigned. `char` is `CHAR_BITS >= 8` bits
typedef uint8_t Octet;
/// - `Byte` is an unsigned `char`, which may or my not be 8-bits, even though we can generally assume it is;
///   I distinguish it from `Octet` mostly because I like being very clear with the reason behind my types.
typedef unsigned char Byte;

/// - `Sz` is the (unsigned) result type of the `sizeof` operator, and often used for the sizes of arrays and other objects
typedef size_t Sz;
// typedef ssize_t ISz; // TODO this is apparently POSIX, but I'm not sure exactly what use it'd be

/// - `UPtr` and `IPtr` are integer types large enough to hold pointers, thus enabling advanced pointer arithmetic
typedef uintptr_t UPtr;
typedef intptr_t IPtr;

/// - `PtrDiff` is the (signed) result of subtracting two pointers
typedef ptrdiff_t PtrDiff;

/// - `{U,I}{8,16,32,64,128}` are all exact-width unsigned/signed integer types, with no defined endianness
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef unsigned __int128 U128; // gcc-specific?

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;
typedef __int128 I128; // gcc-specific?

//////////////////////////////
/// # Strip Library Prefix
//////////////////////////////
///
/// You might notice that all definitions in this library are prefixed with `a_` or `A_`.
/// Define `A_NOPREFIX` to expose these (in the API, not ABI) without this prefix.
/// Eg `a_talloc` can now be written `talloc`, but `a_talloc` still exists and is linked against.
///

#ifdef A_NOPREFIX

  #define STR          A_STR
  #define TOKEN_PASTE  A_TOKEN_PASTE

  #define talloc       a_talloc
  #define tallocSave   a_tallocSave
  #define tallocReset  a_tallocReset

  #define Bytes         A_Bytes
    #define mk_Bs       a_mk_Bs
    #define lit_Bs      a_lit_Bs
    #define spread_Bs   a_spread_Bs
    #define spreadr_Bs  a_spreadr_Bs
    #define BsfromCStr  a_BsfromCStr
    #define cmp_Bs      a_cmp_Bs
    #define eq_Bs       a_eq_Bs
  #define Ascii                A_Ascii
    #define AsciiFromBytes_Bs  a_AsciiFromBytes_Bs
    #define cstrLen_Ascii      a_cstrLen_Ascii
    #define AsciiToCStr        a_AsciiToCStr
    #define AsciiToCStr_t      a_AsciiToCStr_t
    #define AsciiToCStr_m      a_AsciiToCStr_m

#endif

/// Some definitions (ie configuration macros) do not have their prefix stripped.
///

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

/// ## Config Macro: `A_TALLOC_SIZE`
///
/// Define `A_TALLOC_SIZE` to set the maximum number of bytes that can be held in temporary allocator.
/// If not set, the default is 8MiB. I have no reason for this number.
#ifndef A_TALLOC_SIZE
  #define A_TALLOC_SIZE (8 MiB)
#endif
///

/// ## `void* a_talloc(size_t nBytes)`
///
/// Allocate `nBytes` in the temporary allocator.
/// Returns `NULL` if there's an error.
///
/// All allocations are aligned to the fundamental alignment (ie `alignof(max_align_t)`).
/// A zero-byte talloc is allowed, and its address will be distinct from any other talloc object (it will, ofc, use some memory).
void* a_talloc(size_t nBytes);
///

/// ## Save and Restore Temp Allocator
///
/// The functions `a_talloc{Save,Reset}` work in tandem.
/// The save function returns an opaque handle (`A_Savepoint`), and the reset function accepts it.
/// The savepoints are structured as a stack.
///
/// Short story long,
/// a savepoint value may only be used once (by reset), but not be reused.
/// Additional savepoints are also invalidated: when they were created between (runtime-wise) the creaation and usage of some other savepoint.
///
typedef void A_Savepoint;
A_Savepoint* a_tallocSave();
void a_tallocReset(A_Savepoint* base);

//////////////////////
/// # Byte Strings
//////////////////////
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

/// The type `A_Bytes` is a struct with `len` (length) and `str` (pointer to the string data) members.
/// I'm hoping that most ABIs have a way to pass/return two-word data types in registers, since I expect this to be used by-value.
///
/// Ultimately, this type is intended as a mutable (but not growable) view/slice.
/// It _may_ also contain an entire string, but I'm sure it will often contain substrings.
typedef struct A_Bytes {
  IPtr len;
  Byte* str;
} A_Bytes;
///

/// Introduction forms will be used often enough to warrant a little syntactic sugar.
/// `a_mk_Bs` creates the struct from length and pointer, in that order.
/// `a_lit_Bs` creates the struct from a literal string (and will not include the trailing NUL that C inserts and we do not need or want.
#define a_mk_Bs(n, ptr) ((A_Bytes){ .len = (n), .str = (ptr) })
#define a_lit_Bs(str) A_mk_Bs(sizeof(str) - 1, (str))
///

/// `A_spread_Bs` disassembles the Bytes struct to pass to C functions that take the pointer and count separately.
/// It puts the length before the string, which matches the order of the `%.*s` printf format specifier.
/// It also would atch libraries who take strings as VLAs, like `(int n, char[n] data)`.
/// For interacting with libraries that take them in reverse order for whatever reason, use `A_spreadr_Bs`.
#define a_spread_Bs(bytes) (bytes.len), (bytes.str)
#define a_spreadr_Bs(bytes) (bytes.str), (bytes.len)
///

/// Conversions between `Bytes` and C-strings would be nice.
/// Unfortunately, `Bytes` is able to hold NUL, which break c-strings, so we can't convert cleanly to C-strings.
/// `A_fromCStr_Bs` _can_ convert C-strings to `Bytes`, however.
/// If you _do_ have, say, an Ascii `Bytes` to convert to a C-string, that _is_ safe, but it's part of the Ascii module: `A_Ascii_toCStr`. TODO
///
/// Just like `Bs_mk`, `Bs_fromCStr` will not include the trailing Nul in the length.
A__INLINE
A_Bytes a_BsfromCStr(char* cstr) {
  return a_mk_Bs(strlen(cstr), (Byte*)cstr);
}
///

/// `a_cmp_Bs` compares two byte strings lexicographically (not locale-sensitive),
///   returning `-1, 0, 1` for less-than, equal-to, and greater-than, respectively.
/// `a_eq_Bs` tests two bytestrings for equality, and has a nicer interface in `if`-statements that only care about equality.
bool a_cmp_Bs(A_Bytes a, A_Bytes b);
bool a_eq_Bs(A_Bytes a, A_Bytes b);
///

/// TODO isPrefix, isSuffix, findInfix, findInfix_r, findByte, findByte_r.
/// TODO handy syntax for infix/split iterating, trimming.
/// TODO trim chars left/right/both. (zero-init means use the default whitespace one).
/// TODO readline+-chomp.
///
/// TODO wrappers to interface with system calls that take strings
/// TODO a similar module for generic arrays (but I gotta figure out how to API unboxed w/size and boxed)
/// TODO a string builder module, so we can cat these and whatnot
///
/// TODO NOPREFIX
///

/// ## Ascii
///
/// This is just an alternate name for `Bytes`, but indicating the intention to hold Ascii text.
///

/// `A_Ascii` struct is an alias for `Bytes`
typedef A_Bytes A_Ascii;

/// `A_Ascii_fromBytes` will take the longest valid ascii prefix of `Bytes`.
/// Normally, valid ascii codepoints are 0x0--0x7F inclusive, but since we're in C,
///   we disallow NUL (the zero byte) from the string.
///
/// If the length of the returned `Ascii` is the same as the input `Bytes`,
///   that indicates the entire `Bytes` was valid (C) Ascii.
/// If less, then the length indicates how many bytes of the input were valid ascii,
///   and the next byte of the input is non-ascii.
A_Ascii a_AsciiFromBytes(A_Bytes bytes);

/// While `Bytes` values cannot be safely converted to C-strings as duscussed earlier,
///   `Ascii` values are guaranteed (given it was made via `Ascii_fromBytes` to avoid NUL, and can therefore be converted.
/// This is done with `a_AsciiToCStr`.
/// To avoid malloc-ing, it takes an in-parameter which must hold the input ascii's length + 1.
/// To make it ever-so-slightly easier to compute, we also have `A_Ascii_cstrLen` to return the necessary size.
A__INLINE
size_t a_cstrLen_Ascii(A_Ascii str) { return str.len + 1; }
void a_AsciiToCStr(char* restrict dst, A_Ascii src);
///

/// As a convenience, `a_AsciiToCStr_t` will allocate a destination buffer in the [temporary heap](#temporary-allocator)
/// Likewize, `a_AsciiToCStr_m` will use `malloc`.
A__INLINE
char* a_AsciiToCStr_t(A_Ascii src) {
  char* dst = a_talloc(a_cstrLen_Ascii(src));
  assert(dst);
  a_AsciiToCStr(dst, src);
  return dst;
}
A__INLINE
char* a_AsciiToCStr_m(A_Ascii src) {
  char* dst = malloc(a_cstrLen_Ascii(src));
  assert(dst);
  a_AsciiToCStr(dst, src);
  return dst;
}
///

////////////////////
/// # Miscellany
////////////////////
///
/// ## Debugging Helpers
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
/// Classic `max`, `min, `clamp{,Hi,Lo}`
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
/// The unprefixed macro `ver(x)` computes `1 - x`.
/// I'm imagining `ver(sin(theta))` or `sqrt(ver(v*v / c*c))`.
/// Even `ver(sqrt(x))` might come up!
///
#define ver(x) ((typeof x)1 - (x))

#endif
