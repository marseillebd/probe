#!/usr/bin/env bash
# Copyright 2026 - 2026, Marseille Bouchard
# SPDX-Liense-Identifier: GPL-3.0-or-later
set -euo pipefail

###### Paths &c ######

name=a

# Assuming I'm executing from `$0/..`

srcdir="${SRC_DIR:-src}"
docsdir="${DOCS_DIR:-docs}"
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

###### Functions ######

buildExe() {
  local name exe
  name="$1"
  cabal build
  exe=find dist-newstyle/ -executable -type f -name toy
  cp "$exe" "$builddir/$name"
}

buildDocs() {
  local name md html
  test -n "$1" && name="$1"
  echo >&2 "Building documentation: $name"
  md="$docsdir/$name.md"
  html="$builddir/$name.html"
  pandoc \
    --lua-filter="scripts/meta-from-md.lua" \
    --template="scripts/docs-template.html" \
    --from=gfm+footnotes \
    "$md" -o "$html"
}

