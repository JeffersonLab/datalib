# Datalib reorganization validation

These reports record the macOS build and regression tests performed in
November 2026 with the matching `rge/devel-datalib-reorg` source branches.

- [Direct Chroma storage, removed STOCH_HADRON and lib layout](CHROMA-STORAGE-REGRESSIONS.md)
- [FILEDB ownership, standalone tools and CORR_SUPERB regression](FILEDB-TOOLS-REGRESSIONS.md)
- [Bundled array headers and full regression](ARRAYS-OWNED-REGRESSIONS.md)
- [Full clean-stack regression and memory-limit recovery](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md)
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

The full clean-stack follow-up retains the failed 8 GiB RSS-limit attempt and
its fresh targeted retry. A complete rerun passed all 30 paired and three
NPT-only cases with `SB_CACHEGB_CPU=1`, the same RSS guard and tensor planner
budget, and the unchanged numerical gate and deferrals.

The 8 November array-header extraction removes datalib's external
xpath_reader dependency. Four core/five FILEDB checks, installed-header
coexistence checks, and the full 30-pair/three-NPT-only matrix passed;
see the bundled-array report for exact results and scope.

The 9 November checkpoint moves FILEDB ownership and diagnostic tools to
datalib, retains old Chroma measurement calls, verifies the CORR_SUPERB build
gates, and records the full numerical matrix plus existing platform limits.

The later 9 November storage checkpoint supersedes the earlier deferred
legacy-writer modernization. Chroma uses datalib storage directly; temporary
wrappers and STOCH_HADRON are removed. Datalib sources use lib/, and no QDP
configure switch remains. See the latest report above for the full rerun.

[Standalone dbtools extraction and full regression](DBTOOLS-EXTRACTION-REGRESSIONS.md)
records single ownership, frozen legacy formats, standalone builds, utility
comparisons and the complete native-double matrix.

[Fresh MPI parscalar validation](MPI-PARSCALAR-VALIDATION.md) records MPI QMP,
parscalar QDP++, independent XML semantics and the full two-rank matrix. It
retains the stock QDP SIMD-test limitation and clarifies actual OpenMP counts.
