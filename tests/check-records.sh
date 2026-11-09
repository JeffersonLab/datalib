#!/bin/sh
set -eu
tmp=$(mktemp -d "${TMPDIR:-/tmp}/datalib-records.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
./records_probe "$srcdir/tests/fixtures" "$tmp/records.sdb"
