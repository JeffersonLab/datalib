#!/bin/sh
set -eu
for n in 1 4; do
  OMP_NUM_THREADS=$n OMP_DYNAMIC=FALSE ./codec_probe check "$srcdir/tests/fixtures"
done
