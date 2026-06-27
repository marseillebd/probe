#!/usr/bin/env bash
# Copyright 2026 - 2026, Marseille Bouchard
# SPDX-Liense-Identifier: GPL-3.0-or-later
set -euox pipefail

(grep -E -h '^/// ?' || true) | sed -E 's|^/// ?||'
