#!/bin/sh
set -eu
workdir=$(mktemp -d "${TMPDIR:-/tmp}/datalib-filedb.XXXXXX")
trap 'rm -rf "$workdir"' EXIT
./filedb_probe check "$srcdir/tests/fixtures" "$workdir/roundtrip.sdb"
if ./filedb_probe bad-checksum "$srcdir/tests/fixtures" >"$workdir/checksum.log" 2>&1; then
    echo "FILEDB accepted a corrupted value checksum" >&2
    exit 1
else
    test "$?" = 1
    grep 'data checksum mismatch' "$workdir/checksum.log"
fi
