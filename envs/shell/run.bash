#!/usr/bin/env bash
set -euo pipefail
shopt -s globstar

### Main ###

main() {
  local input="input.txt"

  echo >&2 "=== regenerating input file ==="
  echo | tee "$input" <<EOF
Hello, world!
Hello, user!"
42 137 42
EOF

  echo >&2 "=== initial indexing ==="
  reindex "$input" | tee "$input.index-words"

  echo >&2 "=== setup inotify ==="
  ( inotifywait -e modify "$input" >&2
    echo >&2 "=== reindexing ==="
    cp "$input.index-words" "$input.index-words.bak"
    reindex "$input" | tee "$input.index-words"
    echo >&2 "=== the resulting diff ==="
    icdiff "$input.index-words"{.bak,}
  ) &

  echo >&2 "=== editing input file ==="
  sleep 1
  echo "foo world" >> "$input"

  wait
}

reindex() {
  if [[ $# != 1 ]]; then
    echo >&2 "$0 - internal error: usage: reindex <filepath>"
    return 1
  fi
  local f="$1"
  <"$f" grep -ioP '[a-z]+|[0-9]+' | sort -u
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
