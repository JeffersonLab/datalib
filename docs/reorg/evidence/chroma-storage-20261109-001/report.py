from pathlib import Path
import json,hashlib,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';d=r/'datalib'
a=json.loads((e/'independent-audit.json').read_text());v=json.loads((e/'stack-verification.json').read_text());assert a['status']==v['status']=='passed';p=a['paired'];n=a['npt_only'];sources=v['sources'];rows='\n'.join(f'| {name} | `{row["commit"]}` |' for name,row in sources.items());cases='\n'.join(f'| `{c["case"]}` | {c["keys"]} | {c["values"]} | {c.get("max_abs_error",0):.10g} |' for c in p['cases']);producer=json.loads((e/'producer-comparison.json').read_text());assert all(x['returncode']==0 for x in producer)
report=f'''# Direct Chroma storage and lib layout: full regression

9 November 2026. This checkpoint supersedes the deferred Chroma writer
modernization in the [previous FILEDB/tool checkpoint](FILEDB-TOOLS-REGRESSIONS.md).
All work is on `rge/devel-datalib-reorg` under `~/Downloads/macos-datalib-reorg`.
Devel refs and Kanoni's existing code, policy, data and working tree are preserved.

## Result

The full active native-double suite passed **{p['passed']} paired NPT/CORR_SUPERB
cases**, **{p['keys']} keys** and **{p['values']:,} finite complex values**, with
zero strict numerical failures and zero CG solves. Maximum absolute error:
**{max(c.get('max_abs_error',0) for c in p['cases']):.17g}**. The separate permitted NPT-only lane passed
**{n['passed']} cases, {n['keys']} keys and {n['values']} values**.

All 63 output databases are byte-identical to the preceding reorg checkpoint:
30 NPT, 30 CORR and three NPT-only outputs. See the
[independent audit](evidence/chroma-storage-20261109-001/independent-audit.json)
and complete [paired results](evidence/chroma-storage-20261109-001/paired/suite/results.json).
The matched per-value acceptance gate remains relative tolerance `1e-9` plus
`1e-12 * max(reference_global_max, 1e-30)`.

## Architecture and ownership

Chroma's production C++ and Autotools configuration contain no explicit FILEDB
API, `BinaryStoreDB`, `LocalBinaryStoreDB`, or serializable database wrappers.
The temporary `util/ferm/filedb_qdp.h` and `util/ferm/key_val_db.h` are deleted
from both source and fresh installation. Plain key/value structs retain their
existing binary codec functions.

The migrated legacy producers use Eloy's datalib tensor storage classes:
`Datalib::TensorStorage::StorageMeson`, `StorageBaryon`, `StorageProp4`,
`StorageGenprop4`, `StorageDisco`, `StorageGlue`, and the common
`StorageRecords` serialized-record API for formats without a typed adapter.
The common record layer adds writing to the existing reader classes; the
pinned backend remains inside datalib. No database wrapper class remains in
Chroma. The record interface stores encoded key/value strings verbatim and
preserves metadata, bucket counts, configuration capacity and cache defaults.

Chroma's small `encodeStorageRecord` helper uses its QDP **local** buffer writer.
Collective producers keep their reductions on all ranks, broadcast the primary
rank's file-existence result, and encode/write on the primary rank. Existing
local SUPERB producer ownership is preserved. Open/write/close errors abort
through Chroma. Datalib performs no QDP, QMP or MPI communication; its XML
reader remains serial. QDP's collective XML/lattice I/O and SuperbBLAS
communicator selection remain in their consumers. The tensor backend and
CORR_SUPERB configuration remain independent of this legacy record path.

`STOCH_HADRON` was subsequently **removed at the user's direction**, including
its two source/header files, factory registration and both build/install lists.
It is recorded in Chroma's `docs/removals.md`. There is no new compatibility
implementation or runtime-support work for that obsolete measurement. The final
archive has no matching object, the executable has no `InlineStochHadronEnv`
symbols, and the installed public header is absent. See the
[removal verification](evidence/chroma-storage-20261109-001/stoch-hadron-removal.json).

A pre-suite check caught different class layouts in the writer and CORR reader
translation units when the former omitted `USE_SUPERBBLAS`. Chroma now selects
the matching profile whenever `BUILD_SB` is enabled and exports that profile
and the SuperbBLAS include flags through `chroma-config`. All affected objects
were rebuilt. Separate reader, writer and direct-client translation units now
agree on all seven typed-storage class layouts under the public compiler flags.
This is needed by current producers and CORR independently of the removed
measurement; it does not make standalone datalib depend on SuperbBLAS or MPI.
See the [before probe](evidence/chroma-storage-20261109-001/storage-abi-before.json)
and [passing profile check](evidence/chroma-storage-20261109-001/storage-abi-after.json).

Datalib has no ADAT or QDP dependency. Its obsolete `--without-qdp` and reserved
`--with-qdpxx` options are removed from configure and the current README.
Implementation files moved unchanged from `src/` to `lib/`; Automake,
`AC_CONFIG_SRCDIR`, distribution entries and destination provenance paths
follow that layout. All 43 moved files retained their content hashes.
Public headers remain in `include/datalib/` and tools in `main/`.

The `dbutil` executable already belongs to datalib. Its `file.mod meta`, `keys`
and `keysxml` commands include the standalone MOD inspector; there was no
separate `modmeta` executable left in ADAT. ADAT's remaining ENSEM/physics
utility directory is now `main/dbtools`, with Autotools/CMake paths updated.
These remaining analysis tools still require ADAT. There is no duplicate
dbutil executable or FILEDB submodule in clean ADAT checkouts.

Chroma obtains component flags with `datalib-config --storage-cxxflags`,
`--storage-ldflags` and `--storage-libs`. Older `--filedb-*` queries remain in
datalib for existing consumers; their outputs are verified identical.
`chroma-config --cxxflags` now exports the required datalib include flags.

## Build and targeted checks

Fresh committed source checkouts and empty build/install directories were used
for datalib core, datalib with bundled storage and serial SuperbBLAS dbutil,
ADAT, ColorVec, Redstar and Chroma. The Chroma checkout was fast-forwarded during the full build from
`a74d6d52f` through the committed binding/profile fixes to the final
`STOCH_HADRON` removal. Normal dependency builds recompile affected objects, rebuild the archive and
relink all installed programs; the final installation prefix was empty. The initial and final
source commits and both build/install logs are retained. No source, header or
library snapshots were copied over the checkout or installation. Apple Clang, libc++ and libomp were used throughout. The unchanged
FILEDB-free QDP++ installation and other matching pinned support installations
come from the previous checkpoint. The existing QDP++ ARM/x86 SIMD test
limitation remains documented there; QDP++ itself was not changed or rebuilt.

| Repository | Tested code commit |
| --- | --- |
{rows}

[Build commands, statuses and times](evidence/chroma-storage-20261109-001/build.json)
and [clean-source/dependency checks](evidence/chroma-storage-20261109-001/stack-verification.json)
record the actual artifacts. Datalib built with ADAT, QDP/QMP and external
xpath_reader access denied; Chroma built and ran with ADAT/external
xpath_reader access denied. Datalib core passed **5/5** checks; the storage
build passed **8/8**, including frozen graph/XML/legacy formats, MOD metadata
and key inspection, and the new append/reopen/metadata/raw-record test.
ADAT, ColorVec and Redstar `make check` passed. Full Chroma validation used
the numerical suite below. Actual OpenMP team sizes of one and four were
verified; both evaluators link libomp.

The source distribution contains `lib/` and no `src/` implementation directory.
Its unpacked core independently built and passed all five checks under the
dependency-denying sandbox. See the [package check](evidence/chroma-storage-20261109-001/package-check.json).
All 37 removed ADAT compatibility headers remain absent from source and fresh
installation; the direct qualified API probe passes.

The real native color-vector MOD file also reproduced the old metadata, text
keys and XML keys exactly with ADAT/QDP access denied; see
[MOD inspection](evidence/chroma-storage-20261109-001/mod-inspection.json).

The old Chroma writer's key/value fixture was frozen **before** changing the
source. The new installed public API compiled using only exported Chroma flags,
then matched those exact bytes and metadata at **one and two MPI ranks**, with
ADAT access blocked. It tested primary-only writes, separate per-rank writes,
reopening new stores, and reading stores created by the old writer. See
[clean codec probe](evidence/chroma-storage-20261109-001/codec-clean.json).

The first exploratory whole-database hash check failed because the unchanged
pinned backend persists unspecified unused page space. A repeat with the old
writer reproduces that nondeterminism. Keys, values, metadata, layout and payload
checksums match; the bounded wire comparison excludes only unused metadata
page tails, free hash-page gaps, empty configuration-page bodies and the CRCs
of those same pages, using the backend's documented page structure. It does
not change the numerical acceptance gate or modify the backend. See
[wire-layout evidence](evidence/chroma-storage-20261109-001/codec-layout.json).

Real old/new **meson and baryon producer** smokes additionally ran at one and
two MPI ranks with the archived native-double color vectors, original weak-field
gauge/RNG and smearing conventions. These separate small smokes use two vectors,
zero momentum and the first two original displacement entries, with legacy
record output selected. Their scope is writer validation; the full suite's
physics inputs are unchanged. Independent datalib readback checks exact key
coverage and physics metadata, expected shapes, finite values, nonzero coverage, and
all values under the unchanged numerical gate. It also compares the new
one-rank output to its two-rank output. See
[producer comparisons](evidence/chroma-storage-20261109-001/producer-comparison.json).
At each rank count, all 32 meson and 32 baryon value records are byte-identical
old versus new (128 and 256 finite values). Cross-rank maximum errors are
`7.77156e-16` and `3.06904e-17`, respectively. Run timestamps, MPI decomposition
and the meson reader's documented automatic time-batch default are validated
separately; every other metadata field matches exactly. The initial metadata
comparison stopped on those expected execution-provenance differences; that
probe failure and the corrected independent check are retained.
These real producer checks cover those two writer families, not every legacy
measurement's physics or every writer branch at runtime.

## Full numerical coverage and retained limits

The unchanged Kanoni runner and policy used the same checksum-verified
native-double tensor dataset and explicit gauge-transported direct DISCO
supplement. Format-4 matching graphs and the independent key/time/finite
readback audit passed. Resource controls remain `SB_CACHEGB_CPU=1`, an 8192 MiB
RSS guard and a 2 GiB planner; comparisons use one MPI rank. No CG solves were
performed. The three-hadron `3pi` and disconnected `nuc_nuc_3pt` cases are included.
Only the declared eta control has its expected zero graph/value coverage.

The user-directed CORR deferrals remain `ff_3pt`, `fsq_3pt`, `glue` and
`tetra_F27_S2`; `ff_3pt` remains deferred for NPT too. The separate NPT-only
fsq/glue/tetra passes do not establish CORR support. Existing unevaluable-graph
counts in those scopes are retained in the audit. Full multi-rank CORR, GPU,
on-demand GenProp, CG accuracy, and ensemble physics remain outside this
cached-tensor regression. These results do not turn the historical pre-YLM
dataset qualification into latest-YLM solver or data-generation validation.
The targeted two-rank writer probes do not extend
that claim.

| Paired case | Keys | Values | Maximum absolute error |
| --- | --- | --- | --- |
{cases}

## Evidence

Full local artifacts are in
`~/Downloads/macos-datalib-reorg/evidence/chroma-storage-20261109-001`.
Committed evidence is bounded: commands, hashes, source/check programs,
reports and compressed logs. Build/source trees, executables, actual SDB/SBT
payloads and generated correlator databases remain outside Git. Failed
exploratory attempts are retained and labelled; final build/regression status
comes from the explicit successful phases and independent audit. Protected
branch refs and Kanoni working-tree/diff hashes match the initial snapshots.
'''
(d/'docs/reorg/CHROMA-STORAGE-REGRESSIONS.md').write_text(report);(r/'CHROMA-STORAGE-REGRESSIONS.md').write_text(report)
print('Regression report prepared')
