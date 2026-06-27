#!/usr/bin/env bash
set -euo pipefail
shopt -s globstar

here="$(dirname "$(readlink -f "$0")")"

### Main ###

main() {
  echo hello, "$here"
}

###################
### Entry Point ###
###################

if [[ -n "${DEBUG:-}" && "$DEBUG" != 0 ]]; then
  bash --version 1>&2
  set -x
fi

main

### Reference ###

# "You Suck at Programming"'s coding style:
#   https://style.ysap.sh/
