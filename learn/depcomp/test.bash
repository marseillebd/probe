#!/usr/bin/env bash
set -euo pipefail


compile() {
  local infile base
  infile="$1"
  base="$(basename "${infile%%.toy}")"

  # ensure the exe is up to date
  cabal build 1>/dev/null
  mkdir -p .build/
  cp "$(find dist-newstyle/ -executable -type f -name toy)" .build/

  # run my compiler
  .build/toy "examples/$base.toy" > ".build/$base.qbe"
  qbe ".build/$base.qbe" > ".build/$base.S"
  gcc ".build/$base.S" -o ".build/$base"
}

compile "$1"
