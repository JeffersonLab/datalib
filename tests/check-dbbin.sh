#!/bin/sh
set -eu
command -v python3 >/dev/null 2>&1 || exit 77
# Use a new subdirectory; the Python runner refuses an existing work directory.
python3 "$srcdir/tests/test_dbbin.py" "./dbbin" "./dbbin_fixture" "./dbbin-check-$$"
