#ifndef DEFER_H
#define DEFER_H

#ifdef __clang__
#include "defer-clang.h"
#elifdef __GNUC__
#include "defer-gcc.h"
#endif


#endif
