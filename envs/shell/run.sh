#!/usr/bin/env sh
set -eu

### Main ###

main() {
  rm -f input.txt.*
  <input.txt \
    gzip   | tee input.txt.gz | \
    base64 | tee input.txt.gz.base64 | \
    gzip   >     input.txt.gz.base64.gz
  <input.txt \
    base64 | tee input.txt.base64 | \
    gzip   >     input.txt.base64.gz
  <input.txt \
    urlencode | tee input.txt.url | \
    gzip   >        input.txt.url.gz

  # generate report
  ls -l input.txt* | awk '{print($5, "\t", $9);}'
}

###################
### Entry Point ###
###################

if [ -n "${DEBUG:-}" ] && [ "$DEBUG" != 0 ]; then
  sh --version >&2 || echo >&2 "POSIX sh (probably)"
  set -x
fi

main
