#!/usr/bin/env bash
# set -euo pipefail
set -x

###### Paths &c ######

name=a

# Assuming I'm executing from `$0/..`

# hdr="src/$name.h"
src="src/$name.c"
obj="build/$name.o"
slib="build/lib$name.a"
dlib="build/lib$name.so"
testsrc="src/debug.c" # FIXME rename this c file
testexe="build/smoke"
app="src/debug.c" # FIXME should be less hardcoded
exe="build/$name"

mkdir -p build

###### Compiler Setup ######

flags_common=(
  -std=gnu23
  -Isrc
  -Wall -Werror
)

# optimization flags
if true; then
  flags_common+=(
    -O2
  )
fi

flags_dynamic=(
  -shared
  -fPIC
)

flags_exe=(
  -Lbuild -la
)

###### Actual Building ######

# static library (and static object file)
gcc "${flags_common[@]}" \
  -c "$src" -o "$obj"
ar rcs "$slib" "$obj"

# dynamic library
gcc "${flags_common[@]}" "${flags_dynamic[@]}" \
  -c "$src" -o "$dlib"

# smoke test executable
gcc "${flags_common[@]}" "${flags_exe[@]}" \
  "$testsrc" -o "$testexe"

# DELETEME debug executable, for messing around during development
gcc "${flags_common[@]}" "${flags_exe[@]}" \
  "$app" -o "$exe"
