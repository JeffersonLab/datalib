# Datalib reorganization validation

These reports record the macOS build and regression tests performed on
November 2026 with the matching `rge/devel-datalib-reorg` source branches.

- [Clean standalone datalib and ADAT-free Chroma builds](STANDALONE-CLEAN-BUILD.md)
- [Build and targeted validation](VALIDATION.md)
- [Full native-double NPT/CORR_SUPERB regression](NATIVE-DOUBLE-REGRESSIONS.md)

The full matrix passed 30 paired cases with 925 keys and 4,197 finite
complex values. The three permitted NPT-only cases passed with six keys
and 96 values. The reports retain the existing deferrals, physics inputs
and numerical gate.

The evidence committed here contains commands, source and program hashes,
case results and the independent audit. Absolute paths in those records
identify the tested local installations and datasets. The complete run
directories and compressed diagnostics remain in the original
`macos-datalib-reorg/evidence/` workspace. The records describe the source
commits tested on that date.

The build used Apple Clang, libc++ and libomp. The regression used cached
tensors with one OpenMP thread per job and one MPI rank for Chroma.

The clean-build follow-up used fresh source checkouts and empty build/install
directories. ADAT access was blocked for both builds; QDP/QMP access was also
blocked for datalib. Core/FILEDB checks and three CORR_SUPERB runtime cases passed.
Its full build trees, binaries and unabridged logs remain outside Git.
