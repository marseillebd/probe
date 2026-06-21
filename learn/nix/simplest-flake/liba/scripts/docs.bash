#!/usr/bin/env bash
set -euo pipefail

(grep -E -h '^/// ?' || true) | sed -E 's|^/// ?||'
