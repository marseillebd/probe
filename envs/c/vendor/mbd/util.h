#ifndef MBD_UTIL
#define MBD_UTIL

// # The Usual Suspects

// C is full of footguns. Better program defensively.
#include <assert.h>
#include <errno.h>
#include <stdckdint.h>

// Purely for `noreturn`
#include <stdnoreturn.h>

// C has a vast array of integral types and related constants.
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

// Of note:
// - NULL
// - size_t, ptrdiff_t, max_align_t
// - offsetof(type, member)
// - align{of,as}
#include <stddef.h>
#include <stdalign.h>
#include <stdlib.h>

// Some Explicit "Left-Blank" Syntax
#define pass do {} while (0)

// Printf Debugging
#ifndef NDEBUG
  #include <stdio.h>
  // esp for the `{PRI,SCN}{d,i,u,o,x}{{,LEAST,FAST}{8,16,32,64},MAX,PTR}` macros
  #include <inttypes.h>
  // A function-like
  #define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#else
  #define DEBUG(fmt, ...) pass
#endif

// # Token Pasting

// Token pasting is somewhat fraught.
// The following macro does not work within macros, because its arguments are not expanded.
// I can't actually cite a source, but it seems to be the case.
#define TOKEN_PASTE_DIRECT(t1, t2) t1##t2
// Since token pasting often involves generating a token fomr buildins such as `__LINE__`,
// we usualy want to use this token paster instead, even though it looks like it shouldn't do anything.
#define TOKEN_PASTE(t1, t2) TOKEN_PASTE_DIRECT(t1, t2)

// Similarly, sytingizing is fraught.
#define STR_DIRECT(x) #x
#define STR(x) STR_DIRECT(x)

// # Defer
//
// The ["defer" technical spec][defer-ts] (TS 25755, aka WG14 Working Draft N3734)
// will be an invaluable addition to the C language, and I want to use it right away.
// Thankfully, GCC can implement it with extensions and clang 22 supports the spec with `-fdefer-ts`.
//
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
  #ifndef NDEBUG
    #warning "using stddefer for defer"
  #endif
  #include <stddefer.h>
  #if defined(__clang__)
    #if __is_identifier(_Defer)
      #error "clang may need the option -fdefer-ts for the _Defer feature"
    #endif
  #endif
// If there's no direct compiler support for `defer`, gcc is able to simulate it accurately
//   using nested functions and the cleanup attribute.
#elif __GNUC__ > 8
  #ifndef NDEBUG
    #warning "using GNU extensions for defer"
  #endif
  #define defer _Defer
  #define _Defer _DeferShim(__COUNTER__)
  // clang-format off: it seems not to understand breaking after GCC attributes
  #define _DeferShim(N)                                                        \
    /* forward-declare a cleanup function */                                   \
    auto void TOKEN_PASTE(_DeferF, N)(int*);                                   \
    /* Now, declare a dummy variable */                                        \
    /* and use the `__cleanup__` attribute to register our cleanup function */ \
    /*   to run when the dummy goes out-of-scope. */                           \
    __attribute__(( __cleanup__(TOKEN_PASTE(_DeferF, N)),                      \
                    __deprecated__, __unused__                                 \
                  ))                                                           \
    int TOKEN_PASTE(_DeferV, N);                                               \
    /* Finally, "define" our cleanup function */                               \
    /* This has to be last so that its definition actually comes */            \
    /*   from the block following the macro call. */                           \
    __attribute__((__always_inline__, __deprecated__, __unused__))             \
    inline auto void                                                           \
    TOKEN_PASTE(_DeferF, N)(int*)
  // clang-format on
// We have no more fallbacks, but I suspect I'll be using defer in my code anyway.
// So, we fail early.
#else
  #error "The _Defer feature seems not available"
// Or, we could fail on use instead. TODO
/*
# define defer _Defer
# define _Defer for(int defer_not_available[-1]; true; )
*/
#endif

#endif
