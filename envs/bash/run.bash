#!/usr/bin/env bash
set -euo pipefail
shopt -s globstar

### Main ###

main() {
  echo hello
}

###################
### Entry Point ###
###################

if [[ -n "${DEBUG:-}" && "$DEBUG" != 0 ]]; then
  echo >&2 "$BASH"
  bash --version 1>&2
  set -x
fi

main

### Reference ###

# "You Suck at Programming"'s coding style:
#   https://style.ysap.sh/
