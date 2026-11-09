#!/bin/sh
set -eu
PYTHONDONTWRITEBYTECODE=1 python3 "$srcdir/tests/test_mod_reader.py" "$(pwd)/dbutil"
