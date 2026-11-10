#!/bin/sh
set -eu
command -v python3 >/dev/null 2>&1 || exit 77
python3 "$srcdir/tests/test_legacy_tools.py" . ./legacy_tools_fixture \
    "$srcdir/tests/fixtures/dbtools" "./legacy-tools-check-$$"
