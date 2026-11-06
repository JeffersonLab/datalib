#!/bin/sh
set -eu
workdir=$(mktemp -d "${TMPDIR:-/tmp}/datalib-storage.XXXXXX")
trap 'rm -rf "$workdir"' EXIT
./storage_probe "$srcdir/tests/fixtures/correlator-v0-little.bin" \
    "$srcdir/tests/fixtures/correlator-v0-big.bin" "$workdir"
