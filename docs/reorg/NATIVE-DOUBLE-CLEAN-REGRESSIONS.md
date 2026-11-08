# Full native-double regression with the clean datalib stack

7 November 2026. Fresh ADAT, ColorVec and Redstar builds used the same
clean standalone datalib installation as the freshly built ADAT-free Chroma.
Kanoni's full cached native-double matrix for
`rge/devel-datalib-reorg` passed all **30 active paired cases**
with **925 keys and 4,197 finite complex values**.
There were zero strict per-value failures and zero CG solves. The largest
absolute NPT/CORR difference was **1.219576646e-13**
in `3pi`.

## Stack and acceptance

| Component | Reorg commit |
| --- | --- |
| datalib | `b3b1d4206ddbd9371468d38e9d0d72a9787d0dd7` |
| adat | `d3d14bf60a5fc3e5801b219c4e43b5ac4fa23575` |
| colorvec | `5069d970c40583c80947a14a52464737b17f6196` |
| redstar | `ed355701fb651626ca8b241bc2f6f326ed9326e5` |
| chroma | `0f832d738ee1a512f0b6ea6efffda4e77afc5bd0` |

All five application/library components were built from clean source checkouts
on `rge/devel-datalib-reorg`, using Apple Clang, libc++ and libomp. Datalib
and Chroma are the fresh installations verified in the
[clean-build report](STANDALONE-CLEAN-BUILD.md), with three core and four
FILEDB fixture checks passing. ADAT, ColorVec and Redstar were rebuilt from
new clean checkouts into new empty build/install directories against that
same datalib installation. All three passed `make check`.
Their [build commands and return codes](evidence/native-double-clean-full-20261107-001/build.json)
are retained. Matching QDP++/QMP, SuperbBLAS, Hadron, tensor and PRIMME support
installations were reused; those support libraries were not rebuilt again.

The installed-source commits are shown above. Datalib's later documentation
commit `9f086e1b7f285b196d6956660d1d3884a3e98c50` was pushed before this
regression; the audit confirmed that its code matches the clean installed
source commit `b3b1d42`. No library/application source changes were needed.

The fresh ADAT, ColorVec and Redstar prefixes are under
`evidence/native-double-clean-full-20261107-001/install/`. The shared datalib
and Chroma prefixes are under `evidence/standalone-clean-20261107-001/install/`.
ADAT's installed-header probe confirmed that all 37 removed compatibility
paths are unavailable. Representative old container/I/O aliases were rejected
by negative compilation probes. The fresh Redstar flags produced actual
OpenMP teams of one and four; both evaluators link libomp.

Every Chroma job ran through an executor that denies ADAT filesystem access.
The runner hashes that executor; the independent audit separately checks the
real Chroma executable hash, sandbox profile and datalib archive. Their
hashes match the clean-build verification.

Both evaluators used the matching
format-4 graph generator and explicit `average_1pt_diagrams=false`.
The runner used one OpenMP thread per job and one MPI rank for Chroma.
It verified the pinned native-double `RsdCG=1e-14` tensor dataset and
explicit one-file DISCO supplement against their manifests. It matched
the graph and evaluator settings, including `pion_3pt`'s two vectors.
Fake data was disabled. Kanoni's runner and policy were used unchanged.
The readback exporter was copied into the reorg evidence directory and
migrated to the direct APIs before compilation; Kanoni sources were untouched.

The independent audit rechecked the program and data hashes, exact key
and time coverage, one-configuration shapes, finite complex readback, and
the unchanged per-value gate
`abs(NPT-CORR) <= 1e-9 * max(abs(NPT),abs(CORR)) + 1e-12 * max(reference_global_max,1e-30)`.


## Direct API checks

ADAT's ENSEM adapters, CG factory classes and NPoint expansion remain in
ADAT. Shared graph and key definitions retain datalib's public `Hadron`
namespace. Generic serializers use argument-dependent lookup to select
the correct ADAT, ENSEM or Hadron overload.

## Resource configuration and retained first attempt

The first full attempt passed 29 paired cases and all three NPT-only cases,
but `nucnuc_small` was terminated by the unchanged 8,192 MiB RSS guard at
8,403,888 KiB (8,206.9 MiB). Its results and incomplete Chroma output remain
in `evidence/native-double-clean-full-20261107-001/`; that attempt is not
labelled successful. The supported SuperbBLAS reusable CPU allocation cache
normally defaults to 10% of system RAM, separately from the 2 GiB contraction
planner budget.

A fresh targeted retry passed with `SB_CACHEGB_CPU=1`. The entire paired
matrix and NPT-only lane were then rerun in fresh directories with that same
1 GiB reusable-cache cap. The 8 GiB RSS guard, 2 GiB tensor planner, physics,
thread/rank counts, executables, numerical gate and deferrals were unchanged.
No source fix or larger process-memory allowance was used. The successful
full run is under `evidence/native-double-clean-full-20261107-002/`.
See [resource retry provenance](evidence/native-double-clean-full-20261107-002/resource-retry.json).
The committed records include the [original failed sweep](evidence/native-double-clean-full-20261107-001/paired/suite/results.json),
[failure details and uncorrected XML-report classification](evidence/native-double-clean-full-20261107-001/memory-limit-failure.json),
[losslessly compressed failure log](evidence/native-double-clean-full-20261107-001/paired/suite/no_1pt/nucnuc_small/chroma.log.gz),
and [targeted retry](evidence/native-double-clean-full-20261107-001/retry-cache1/suite/results.json).

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

The `fsq_3pt` NPT-only check retains its explicitly authorized legacy
`gpdf_2_0.00.sdb` input; it does not claim newly generated native-double gpdf data.

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

Against the immediately preceding full direct-API reorg run, 30/30 NPT,
30/30 CORR_SUPERB and
3/3 NPT-only databases were byte-identical.

The recorded `devel` refs and Kanoni status match before and after the
run. The runner and policy hashes also match. The evidence directory
holds the exact commands, outputs, and independent audit. It records
4 large diagnostic logs compressed after verification
of their uncompressed SHA-256 checksums.

- [Paired results](evidence/native-double-clean-full-20261107-002/paired/suite/results.json)
- [NPT-only results](evidence/native-double-clean-full-20261107-002/npt-only/suite/results.json)
- [Independent audit and per-case historical comparisons](evidence/native-double-clean-full-20261107-002/independent-audit.json)

All original reorg checkout states also matched their starting snapshots.
The run did not change any original repository, `devel` ref or Kanoni file.

Coverage is limited to cached tensors on this one-rank build. On-demand
GenProp, multi-rank MPI, GPU and CMake remain outside this run.

The [publication manifest](evidence/native-double-clean-full-publication.json)
records the hashes of the exact small evidence copies and losslessly compressed
diagnostics committed on 8 November 2026. Full numerical databases, raw graphs,
build trees, installations and other large diagnostics remain outside Git.
