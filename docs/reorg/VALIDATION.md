# Format-4 datalib reorg validation

6 November 2026. All source checkouts below use
`rge/devel-datalib-reorg`.

| Component | Local commit |
| --- | --- |
| datalib | `71d6eed1de` |
| adat | `ebef988aec` |
| colorvec | `bab4ff2f05` |
| redstar | `839141e9ad` |
| chroma | `0f832d738e` |

Apple Clang, libc++ and libomp built datalib, ADAT, ColorVec, Redstar,
and Chroma via Autotools. These builds reused the pinned support installations
for QDP++, QMP, SuperbBLAS, Hadron, FILEDB and the other unchanged support
dependencies. The final Chroma build recompiled the affected CORR and DISCO
sources against the final datalib prefix and linked the application and auxiliary
programs. Its link line contains datalib and no ADAT library.
Both applications link libomp. The installed Redstar flags produced OpenMP
teams of one and four.

Datalib passed four checks with FILEDB and three without it. Format-4 binary
and XML writers reproduced the frozen bytes. Format-3 binary and XML files
were rejected. All 64 nonzero Dirac–Pauli gamma projections matched the
historical ADAT implementation exactly. ADAT, ColorVec, and Redstar passed
their build checks, and the pion audit captured 419 structured
`MEMAUDIT` records.

## Cached native-double comparisons

All 6 targeted one-rank no_1pt comparisons passed, covering
12 keys and 174 finite complex values. The suite reported
zero strict failures and zero CG solves. The existing Kanoni gate, pinned data,
explicit DISCO supplement and deferrals apply. 6 of 6
Chroma outputs were byte-identical to the historical no_1pt output.

| Case | Keys | Finite complex values | Maximum absolute difference |
| --- | ---: | ---: | ---: |
| `pion` | 3 | 48 | 9.04113e-15 |
| `1pt` | 3 | 48 | 8.61689e-18 |
| `3pi` | 3 | 48 | 1.21958e-13 |
| `nuc` | 1 | 16 | 2.22045e-16 |
| `nuc_nuc_3pt` | 1 | 7 | 4.17777e-17 |
| `pion_3pt` | 1 | 7 | 3.01842e-16 |

Fake data was disabled for all six cases. Independent readback found 0 exactly zero channels across both evaluators. NPT reported one unevaluable graph in `nuc_nuc_3pt`, the same count as the historical no_1pt run. Its assembled correlator passed the seven-value comparison. The other cases reported zero unevaluable graphs.

Large NPT diagnostics were gzip-compressed after verifying their uncompressed SHA-256.

These results describe the targeted run. A separate
[full native-double regression](NATIVE-DOUBLE-REGRESSIONS.md) records
the complete cached-tensor matrix and existing deferrals.

See [build and link evidence](evidence/no1pt-reorg-runtime-001/) and the
[case results](evidence/no1pt-reorg-runtime-001/native-double-smoke/suite/results.json).
