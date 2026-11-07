# Full native-double NPT/CORR_SUPERB regression

6 November 2026. The full cached-tensor matrix for
`rge/devel-datalib-reorg` passed all **30 active paired cases**
with **925 keys and 4,197 finite complex values**.
There were zero strict per-value failures and zero CG solves. The largest
absolute NPT/CORR difference was **1.219576646e-13**
in `3pi`.

## Stack and acceptance

| Component | Reorg commit |
| --- | --- |
| datalib | `71d6eed1def87b2eb00d499c61ef73424dbb1616` |
| adat | `ebef988aec67947b95b8aa46645c29ebb9a22e35` |
| colorvec | `bab4ff2f050f3b91cf10d2fc394c74aa42faa7c0` |
| redstar | `839141e9adbc4b9e2b1f82cff0869a1b2d35b742` |
| chroma | `0f832d738ee1a512f0b6ea6efffda4e77afc5bd0` |

The run used the installed Apple Clang/libomp stack from the
[build report](VALIDATION.md). Both evaluators used the matching
format-4 graph generator and explicit `average_1pt_diagrams=false`.
The runner used one OpenMP thread per job and one MPI rank for Chroma.
It verified the pinned native-double `RsdCG=1e-14` tensor dataset and
explicit one-file DISCO supplement against their manifests. It matched
the graph and evaluator settings, including `pion_3pt`'s two vectors.
Fake data was disabled.

The independent audit rechecked the program and data hashes, exact key
and time coverage, one-configuration shapes, finite complex readback, and
the unchanged per-value gate
`abs(NPT-CORR) <= 1e-9 * max(abs(NPT),abs(CORR)) + 1e-12 * max(reference_global_max,1e-30)`.


## Paired results

| Case | Keys | Finite complex values | Maximum absolute NPT/CORR difference |
| --- | ---: | ---: | ---: |
| `pion` | 3 | 48 | 9.0411331e-15 |
| `1pt` | 3 | 48 | 8.6168924e-18 |
| `33` | 1 | 16 | 7.2364964e-16 |
| `3pi` | 3 | 48 | 1.2195766e-13 |
| `a0.d3` | 324 | 1296 | 4.6479365e-20 |
| `a1_rhopi` | 1 | 16 | 3.4331144e-15 |
| `d` | 1 | 16 | 3.6638411e-16 |
| `delta_J1o2m` | 1 | 16 | 1.1150913e-16 |
| `eta8eta8_F8` | 1 | 16 | 1.4202501e-15 |
| `eta_eta_3pt` | 1 | 16 | 0 |
| `h8` | 4 | 64 | 6.4926177e-15 |
| `hard.1_tslice.ini` | 529 | 2116 | 2.5816236e-19 |
| `nuc` | 1 | 16 | 2.220446e-16 |
| `nuc.p100` | 2 | 14 | 2.120695e-16 |
| `nuc_gA` | 1 | 7 | 1.4267046e-17 |
| `nuc_nuc_3pt` | 1 | 7 | 4.1777674e-17 |
| `nuc_nuc_ax_3pt` | 4 | 28 | 8.725935e-17 |
| `nuc_nuc_proj_3pt` | 1 | 7 | 2.6196745e-19 |
| `nucnuc_small` | 2 | 8 | 7.9839694e-17 |
| `nucpi` | 1 | 16 | 3.5391633e-17 |
| `nucpi_I3o2` | 1 | 16 | 3.332295e-17 |
| `nucpi_J1o2m` | 1 | 16 | 2.5086874e-17 |
| `nucpi_nucpi_J1o2m` | 2 | 32 | 2.7755576e-15 |
| `pi` | 1 | 16 | 1.260013e-15 |
| `pi_pi_3pt` | 1 | 6 | 2.9326577e-16 |
| `pion_3pt` | 1 | 7 | 3.0184188e-16 |
| `pipi_i1` | 1 | 16 | 3.0201331e-15 |
| `psi` | 2 | 32 | 4.0120859e-15 |
| `rho` | 3 | 48 | 1.2440854e-14 |
| `rho_rho_3pt` | 27 | 189 | 6.0735531e-16 |

Only the declared `eta_eta_3pt` control had an exactly zero channel.
It had zero graphs, one key and 16 zero values in each evaluator.
Every positive case had nonempty graph coverage.

## Deferrals and NPT-only coverage

CORR_SUPERB remained deferred for `ff_3pt`, `fsq_3pt`, `glue`, `tetra_F27_S2`. The `ff_3pt`
case was also deferred for NPT. The three permitted NPT-only cases
passed complete finite readback with 6 keys and
96 values.

| NPT-only case | Keys | Finite complex values |
| --- | ---: | ---: |
| `fsq_3pt` | 2 | 32 |
| `glue` | 1 | 16 |
| `tetra_F27_S2` | 3 | 48 |

The NPT-only lane had no exactly zero channels.

NPT logged unevaluable graphs in the cases below. The assembled correlators passed their respective comparisons or readback checks. Other cases logged zero.

| Lane | Case | Reorg graphs | Historical graphs |
| --- | --- | ---: | ---: |
| paired | `nuc_nuc_3pt` | 1 | 1 |
| NPT-only | `fsq_3pt` | 2 | 2 |

## Historical no_1pt comparison and evidence

Against the saved 3 November experimental results, 30/30
paired NPT databases, 30/30 CORR_SUPERB databases, and
3/3 NPT-only databases were byte-identical. Applying the
same value gate to the historical readbacks gave 0 NPT
failures and 0 CORR failures. The largest differences were
0 for NPT and 0 for CORR_SUPERB.
The comparison used the saved databases and exports.

The recorded `devel` refs and Kanoni status match before and after the
run. The runner and policy hashes also match. The evidence directory
holds the exact commands, outputs, and independent audit. It records
4 large diagnostic logs compressed after verification
of their uncompressed SHA-256 checksums.

- [Paired results](evidence/native-double-full-20261106-001/paired/suite/results.json)
- [NPT-only results](evidence/native-double-full-20261106-001/npt-only/suite/results.json)
- [Independent audit and per-case historical comparisons](evidence/native-double-full-20261106-001/independent-audit.json)

Coverage is limited to cached tensors on this one-rank build. On-demand
GenProp, multi-rank MPI, GPU and CMake remain outside this run.
