#!/usr/bin/env sh
set -eu

here="$(dirname "$(readlink -f "$0")")"

### Main ###

main() {
  echo hello, "$here"
}

###################
### Entry Point ###
###################

if [ -n "${DEBUG:-}" ] && [ "$DEBUG" != 0 ]; then
  sh --version >&2 || echo >&2 "POSIX sh (probably)"
  set -x
fi

main
