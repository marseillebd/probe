#!/usr/bin/env sh
set -eu

### Main ###

main() {
  echo hello
}

###################
### Entry Point ###
###################

if [ -n "${DEBUG:-}" ] && [ "$DEBUG" != 0 ]; then
  sh --version >&2 || echo >&2 "POSIX sh (probably)"
  set -x
fi

main
