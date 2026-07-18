#!/usr/bin/env bash
set -euo pipefail


compile() {
  local infile base
  infile="$1"
  base="${infile%%.toy}"
  mkdir -p build
  racket compile.scm > "build/$base.qbe"
  qbe "build/$base.qbe" > "build/$base.S"
  gcc "build/$base.S" -o "$base"
}

compile "$1"
