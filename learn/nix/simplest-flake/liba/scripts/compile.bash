#!/usr/bin/env bash
# Copyright 2026 - 2026, Marseille Bouchard
# SPDX-Liense-Identifier: GPL-3.0-or-later

# set -euo pipefail

###### Paths &c ######

name=a

# Assuming I'm executing from `$0/..`

srcdir="${SRC_DIR:-src}"
builddir="${BUILD_DIR:-build}"
# distdir="${DIST_DIR:-dist}"

# hdr="src/$name.h"
# src="$srcdir/$name.c"
# obj="$builddir/$name.o"
# slib="$builddir/lib$name.a"
# dlib="$builddir/lib$name.so"
# exe="$builddir/$name"

mkdir -p "$builddir"

###### Compiler Setup ######

flags_common=(
  -std=gnu23
  -I"$srcdir"
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

###### Functions ######

# `buildObj <basename>`
# compile a `src/<basename>.c` to `build/<basename>.o` library
buildObj() {
  local name src obj
  test -n "$1" && name="$1"
  echo >&2 "Building object file: $name"
  src="$srcdir/$name.c"
  obj="$builddir/$name.o"
  gcc "${flags_common[@]}" \
    -c "$src" -o "$obj"
}

# `buildStaticLib <basename>`
# package `build/<basename>.o` to `build/lib<basename>.a`
# requires `buildObj <basename>` first
buildStaticLib() {
  local name obj lib
  test -n "$1" && name="$1"
  echo >&2 "Building static library: $name"
  obj="$builddir/$name.o"
  lib="$builddir/lib$name.a"
  ar rcs "$lib" "$obj"
}

# `buildDynamicLib <basename>`
# compile `src/<basename>.c` to `build/lib<basename>.so`
buildDynamicLib() {
  local name src lib
  echo >&2 "Building dynamic library: $name"
  test -n "$1" && name="$1"
  src="$srcdir/$name.c"
  lib="$builddir/lib$name.so"
  gcc "${flags_common[@]}" "${flags_dynamic[@]}" \
    -c "$src" -o "$lib"
}

# `buildExe <basename>`
# compile `src/basename.c` to `build/<basename>` executable
# requires `buildStaticLib`, so link to liba
buildExe() {
  local name src exe
  test -n "$1" && name="$1"
  echo >&2 "Building executable: $name"
  src="$srcdir/$name.c"
  exe="$builddir/$name"
  gcc "${flags_common[@]}" "${flags_exe[@]}" \
    "$src" -o "$exe"
}

buildDocs() {
  local infile outname md html
  test -n "$1" && infile="$1"
  test -n "$2" && outname="$2"
  echo >&2 "Building documentation: $outname"
  src="$srcdir/$infile"
  md="$builddir/$outname.md"
  html="$builddir/$outname.html"
  grep -E -h '^///( |$)' "$src" \
    | sed -E 's|^/// ?||' >"$md"
  pandoc \
    --lua-filter="scripts/meta-from-md.lua" \
    --template="scripts/docs-template.html" \
    --from=gfm+footnotes \
    "$md" -o "$html"
}
