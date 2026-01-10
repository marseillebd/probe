#ifndef DEFER_CLANG_H
#define DEFER_CLANG_H

#include <assert.h>

#define defer_unimplemented_on_clang 0
#define defer for(;assert(defer_unimplemented_on_clang), 0;)

/* static inline void defer_cleanup(void (^*b)(void)) { (*b)(); } */
/* #define defer_merge(a,b) a##b */
/* #define defer_varname(a) defer_merge(defer_scopevar_, a) */
/* #define defer __attribute__((cleanup(defer_cleanup))) void (^defer_varname(__COUNTER__))(void) = */

#endif

