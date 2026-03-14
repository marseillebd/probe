#!/usr/bin/env bash
set -euo pipefail

(grep -Fw -Ls '@file' {src,app}/*.{h,c} || true) | while read -r undocFile; do
  printf 'warning undocumented file: %s\n' "$undocFile"
done
