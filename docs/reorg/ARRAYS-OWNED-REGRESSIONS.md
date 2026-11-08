# Bundled array headers and full native-double regression

8 November 2026. Datalib now owns its rank-1 through rank-4 array headers
under `<datalib/model/xml_array*.h>` and has no external xpath_reader
configure, compiler, link or installed-header dependency. Fresh ADAT,
ColorVec, Redstar and Chroma builds used that installation.
Kanoni's full cached native-double matrix for
`rge/devel-datalib-reorg` passed all **30 active paired cases**
with **925 keys and 4,197 finite complex values**.
There were zero strict per-value failures and zero CG solves. The largest
absolute NPT/CORR difference was **1.219576646e-13**
in `3pi`.

## Stack and acceptance

| Component | Reorg commit |
| --- | --- |
| datalib | `6f6a08a7a1e490d73ced26a002fd4202175a61e6` |
| chroma | `3e4724af9b664766fa6860f65505b5d4cbb4ae26` |
| adat | `d3d14bf60a5fc3e5801b219c4e43b5ac4fa23575` |
| colorvec | `5069d970c40583c80947a14a52464737b17f6196` |
| redstar | `ed355701fb651626ca8b241bc2f6f326ed9326e5` |

All five components were built from clean source checkouts on
`rge/devel-datalib-reorg` into empty build/install directories with Apple
Clang, libc++ and libomp. Datalib configured without `--with-xpath` and
passed four core and five FILEDB checks, with ADAT, QDP/QMP and external
xpath_reader filesystem access denied. Both builds reproduced the existing
frozen formats and the additional rank-1/2/3/4 fixture captured from the old
implementation before extraction. No existing fixture was regenerated.

The four copied headers retain their `XMLArray` namespace, guards, layout
and algorithms. Only internal include paths and trailing whitespace were
changed. The source notices and COPYING.xpath_reader remain in the package;
[array provenance](evidence/arrays-owned-20261108-001/array-provenance.json)
records the upstream commit and hashes. The dependency removal does not
remove libxml2, OpenMP, or the optional FILEDB dependency.

Chroma's QDP array header has pre-existing rank-1 empty-comparison/operator!=
differences. Its two datalib consumers explicitly select QDP's array headers
first, preserving their previous behavior. Installed-header checks and
both include orders with QDP and standalone xpath_reader passed the same
frozen array wire fixtures. No top-level xml_array compatibility headers
were installed by datalib.

Chroma rebuilt with ADAT and external standalone xpath_reader access denied.
ADAT, ColorVec and Redstar rebuilt against the same new datalib installation
and passed `make check`. QDP++/QMP, SuperbBLAS, Hadron, tensor, FILEDB and
PRIMME support installations were reused; those unchanged support libraries
were not rebuilt. ADAT and QDP retain their own XML support dependencies;
they are not datalib dependencies.

[Build commands and return codes](evidence/arrays-owned-20261108-001/build.json),
[owned-header/dependency verification](evidence/arrays-owned-20261108-001/owned-array-verification.json)
and [complete stack verification](evidence/arrays-owned-20261108-001/stack-verification.json)
record the checks. Actual OpenMP teams of one and four were observed from
the fresh Redstar flags. Both evaluators link libomp, with no libgomp or
libstdc++ in either process.

The fresh install prefixes are under `evidence/arrays-owned-20261108-001/install/`.
The installed ADAT probe also confirmed that the 37 removed compatibility
headers and representative legacy aliases remain absent. Every Chroma job
ran through an executor denying ADAT and external standalone xpath_reader
access. The audit separately rechecked the real executable, wrapper, sandbox
profile and datalib archive hashes.

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

## Resource configuration

The full paired matrix and NPT-only lane used the established
`SB_CACHEGB_CPU=1` reusable-cache cap, unchanged 8 GiB RSS guard and 2 GiB
tensor planner. Physics inputs, thread/rank counts, numerical gate and
user deferrals were unchanged. The preceding checkpoint's retained
memory-limit failure and cache-capped rerun are documented in the
[prior full clean-stack report](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md).

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

Against the immediately preceding cache-capped clean-stack run, 30/30 NPT,
30/30 CORR_SUPERB and
3/3 NPT-only databases were byte-identical.

The recorded `devel` refs and Kanoni status match before and after the
run. The runner and policy hashes also match. The evidence directory
holds the exact commands, outputs, and independent audit. It records
4 large diagnostic logs compressed after verification
of their uncompressed SHA-256 checksums.

- [Paired results](evidence/arrays-owned-20261108-001/paired/suite/results.json)
- [NPT-only results](evidence/arrays-owned-20261108-001/npt-only/suite/results.json)
- [Independent audit and per-case historical comparisons](evidence/arrays-owned-20261108-001/independent-audit.json)

All original reorg checkout states also matched their starting snapshots.
The validation run kept the reorg repositories at their recorded extraction
commits and did not change any `devel` ref or Kanoni file.

Coverage is limited to cached tensors on this one-rank build. On-demand
GenProp, multi-rank MPI, GPU and CMake remain outside this run.
