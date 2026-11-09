from pathlib import Path
import json,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';d=r/'datalib'
a=json.loads((e/'independent-audit.json').read_text());assert a['status']=='passed';v=json.loads((e/'stack-verification.json').read_text());assert v['status']=='passed'
sources=v['sources'];qdp=json.loads((e/'qdpxx-base.json').read_text());qdp_commit=subprocess.check_output(['git','-C',str(r/'qdpxx'),'rev-parse','HEAD'],text=True).strip();p=a['paired'];n=a['npt_only'];delta=a['versus_previous_reorg']
assert json.loads((e/'package-check-final.json').read_text())['status']=='passed'
assert json.loads((e/'eloy-filedb-final-check.json').read_text())['status']=='passed'
rows='\n'.join(f'| {name} | `{row["commit"]}` |' for name,row in sources.items())+f'\n| qdpxx | `{qdp_commit}` |'
cases='\n'.join(f'| `{c["case"]}` | {c["keys"]} | {c["values"]} | {c.get("max_abs_error",0):.10g} |' for c in p['cases'])
ntable='\n'.join(f'| `{c["case"]}` | {c["keys"]} | {c["values"]} | {c["npt_unevaluable_graphs"]} |' for c in n['cases'])
report=f'''# FILEDB ownership, standalone tools and full regression

9 November 2026. Work is isolated to `rge/devel-datalib-reorg` in
`~/Downloads/macos-datalib-reorg`. Devel refs, Kanoni's scripts/policy/data,
and its pre-existing working-tree changes are preserved.

Datalib owns the FILEDB submodule at the unchanged commit
`db2a795aaf6cc57b6eefe2ef634fdc91ee7b3693`. ADAT and QDP++ no longer build or
install their own copies. Eloy's typed storage classes continue to use
`FILEDB::ConfDataStoreDBSimple` explicitly for their FILEDB backend and
SuperbBLAS for their tensor backend. The core datalib has no ADAT, QDP,
QMP, MPI or external xpath_reader build dependency.

## Ownership and APIs

- Datalib installs `dbutil`, `view_corr_graph` and `dump_corr_graph`.
  ADAT no longer installs `dbutil`/`libdbutil`; Redstar no longer installs
  the two graph viewers. Existing CLI arguments and exported filenames
  are preserved.
- The graph model/codec header is `<datalib/graph/corr_graph.h>` alongside
  `src/corr_graph.cc`. No old `process_corrs.h` compatibility header is
  installed. Redstar's construction/assembly implementation remains in
  `corr_graph_build.{{h,cc}}`; this code still needs Redstar/ADAT physics.
- Shared correlator storage and metadata use
  `<datalib/storage/corr_reader.h>` and `<datalib/storage/dbtype.h>`.
  Legacy FILEDB values use the frozen signed-length/complex-double codec,
  with standard C++ values. ENSEM value conversions remain in ADAT.
- Neutral historical correlator keys/codecs live in `<datalib/legacy/>`.
  ADAT's formfac headers contain only their actual ENSEM database/map
  adapters. No second copy of their key or codec definitions remains.
- Operator filename formatting, pure naming tables and momentum-selection
  helpers moved with the viewers. The three-hadron joint-momentum naming
  implementation is unchanged. ADAT retains its FSq-specific filename
  formatter.
- The `dbutil` executable and its MOD inspector moved from `main/dbutil`.
  The other **20 installed ENSEM/physics database programs remain in ADAT**;
  this checkpoint does not claim that the entire directory is standalone.

FILEDB is built from `datalib/other_libs/filedb` by default. An external
installation remains selectable with `--with-filedb=PREFIX`; use
`--without-filedb` for the independent core and graph tools. Component flags
come from `datalib-config --filedb-cxxflags`, `--filedb-ldflags` and
`--filedb-libs`. `--with-superbblas=PREFIX` enables the standalone dbutil's
serial tensor backend. ADAT and Chroma obtain FILEDB through these exports.

The inactive original dirty ADAT FILEDB worktree is retained locally at
`adat/other_libs/filedb` and excluded locally. Its gitlink, recursive build
and include paths are removed. Clean ADAT checkouts contain no FILEDB
submodule. The standalone source distribution includes the pinned FILEDB
source.

## CORR_SUPERB configuration

Chroma's supported switches are `--with-datalib=PREFIX` and
`--with-superbblas=PREFIX`. The final executable has both `BUILD_SB` and
`BUILD_REDSTAR_DATALIB` enabled and the CORR_SUPERB registration linked.
The full paired sweep below exercises that measurement, not only the linker.
See [feature build proof](evidence/tools-filedb-20261109-001/corr-superb-build.json).
Datalib's ordinary FILEDB-enabled installation already supplies the shared
models, codecs, keys, storage and evaluator headers. Datalib's own optional
`--with-superbblas` switch enables its standalone dbutil tensor backend;
Chroma selects its own matching SuperbBLAS build.

## Chroma boundary and deferred modernization

The older Chroma measurement implementation is unchanged. Its legacy
`BinaryStoreDB`/`LocalBinaryStoreDB` API is retained in Chroma's
`util/ferm/filedb_qdp.h`, relocated from QDP++ with its algorithms, defaults
and existing communication behavior. `key_val_db.h` includes that adapter.
FILEDB headers and libraries are supplied by datalib. There are no new
QDP-aware database wrappers in datalib.

The user explicitly deferred rewriting old Chroma measurements to direct
FILEDB calls or Eloy's storage interfaces. Eloy's `StorageMeson`,
`StorageBaryon`, `StorageProp4`, `StorageGenprop4` and `StorageDisco` remain
shared typed storage interfaces; they are not removed in this reorganization.
Chroma's `SB::StorageTensor` continues to select its SuperbBLAS communication
mode independently. Datalib's XML readers remain serial.

## Tested source stack

| Repository | Code commit |
| --- | --- |
{rows}

The final core, FILEDB, ADAT, ColorVec and Redstar builds used fresh committed
source checkouts and separate empty build directories. ADAT/ColorVec/Redstar
were installed into empty prefixes after retaining earlier attempts.
Datalib's final component was rebuilt completely and installed at the same
prefix. Chroma used a clean source checkout, resumed after the requested
pause, then rebuilt affected dependencies and relinked its installed programs
against the final datalib archive. No source or binary snapshots were overlaid.
The final archive and executable hashes are recorded in
[evidence/chroma-final-link.json](evidence/tools-filedb-20261109-001/chroma-final-link.json).
The metadata-only inline correction is absent from every Chroma compile
dependency record; the Chroma measurement sources are unchanged.

Apple Clang, libc++ and libomp were used consistently, with OpenMP enabled.
Observed Redstar teams were one and four. QDP++ was freshly built for the
same double-precision parscalar/QMP-MPI configuration as the previous stack.
QMP, Hadron, tensor, SuperbBLAS and PRIMME support installations were reused.

## Build and format checks

- Final core: **5/5** checks; bundled FILEDB/tensor build: **7/7** checks.
  Both builds denied ADAT, QDP/QMP and external xpath_reader filesystem access.
- The external FILEDB mode passed **7/7** checks. `make dist` passed and the
  archive contains FILEDB, all moved tools and canonical headers.
- New legacy 1/2/3-point keys, SU(2) N-point keys, vector values and exported
  filenames reproduce fixtures frozen with the previous ADAT installation
  before extraction. Existing format fixtures were not regenerated.
- The moved viewers produced byte-identical summaries (excluding read timing)
  and XML for real `pion`, `3pi`, `nuc_nuc_3pt` and the declared eta control
  graphs. Format-4 graph checks retain rejection of format 3.
- ADAT, ColorVec and Redstar `make check` completed. Chroma built and installed
  with ADAT and external xpath_reader access denied, has no ADAT/ENSEM symbols,
  and uses datalib's FILEDB installation. No FILEDB libraries, headers or
  database wrappers are installed by the new QDP++.
- Eloy's StorageMeson FILEDB backend read a real read-only public meson database
  at one and two MPI ranks: **63,792 keys**, with **100 finite values** in the
  selected elemental per rank. A separate legacy correlator write/read tested
  two configurations and two times in independent files per rank. These probes
  use no ADAT or QDP APIs; they do not establish multi-rank CORR_SUPERB support.

## Full native-double numerical regression

All **{p['passed']} active paired NPT/CORR_SUPERB cases** passed with
**{p['keys']} keys** and **{p['values']:,} finite complex values**. The separate
NPT-only lane passed **{n['passed']} cases**, **{n['keys']} keys** and
**{n['values']} values**. There were zero strict per-value failures and zero CG
solves. Maximum paired absolute difference:
**{max(c.get('max_abs_error',0) for c in p['cases']):.17g}**.

Compared with the 8 November owned-array checkpoint:
**{delta['paired_npt_identical']}/30 NPT**, **{delta['paired_corr_identical']}/30 CORR**
and **{delta['npt_only_identical']}/3 NPT-only** output databases are byte-identical.
Independent readback rechecked exact keys/times, one-configuration shapes,
finite values, data/tool hashes, fake-data settings and all active case coverage.

The matched gate remains
`abs(NPT-CORR) <= 1e-9 * max(abs(NPT),abs(CORR)) + 1e-12 * max(reference_global_max,1e-30)`.
The relative-only dbutil diagnostic is retained separately.

The suite used the unchanged native-double `RsdCG=1e-14` dataset, explicit
validated one-file DISCO supplement and matching format-4 graph generator.
`average_1pt_diagrams=false`, vector counts, phases, masses, momenta and time
origins are unchanged. `pion_3pt` still uses two vectors. Fake data is disabled.
`SB_CACHEGB_CPU=1`, the 8 GiB process RSS guard and 2 GiB tensor planner are
unchanged. Chroma runs through an ADAT-denying executor at one MPI rank, with
one OpenMP thread per numerical job. Kanoni's runner and policy were used
unchanged; small reorg-owned executable dispatchers resolve the relocated graph
tool paths and record both their destinations and actual executable hashes.

| Paired case | Keys | Finite values | Maximum absolute difference |
| --- | ---: | ---: | ---: |
{cases}

| NPT-only case | Keys | Finite values | Unevaluable graphs |
| --- | ---: | ---: | ---: |
{ntable}

## Preserved deferrals and limits

CORR deferrals remain `ff_3pt`, `fsq_3pt`, `glue` and `tetra_F27_S2`.
`ff_3pt` is also deferred for NPT. The NPT-only fsq lane retains its previously
authorized legacy `gpdf_2_0.00.sdb`; its pass does not establish native-double
Chroma fsq support. The eta control retains zero graphs, one key and 16 zeros.
The existing unevaluable NPT graphs remain: one in `nuc_nuc_3pt`, two in the
NPT-only `fsq_3pt`; no failing channels were discarded.

This cached-tensor sweep performs zero CG solves and does not validate
on-demand GenProp, gauge/tensor regeneration, latest-YLM generation, GPU,
full multi-rank contractions, CMake or ensemble statistics. The corrected
pre-YLM dataset-generation qualification remains in force.

## Retained failures and superseded attempts

QDP++'s library and examples built, but full `make check` failed in unchanged
x86 SIMD tests on this ARM/parscalar configuration. The same SIMD failure was
reproduced against the preceding QDP++ installation. No tests or numerical
gates were disabled. See
[QDP limitation](evidence/tools-filedb-20261109-001/qdp-check-limitation.json).

An exploratory two-rank legacy Chroma adapter probe failed; its one-rank probe
passed. That probe uses the old QDP buffer serializer path, not Eloy's typed
storage backend. The legacy implementation is retained without a general
multi-rank read claim; modernization remains deferred. Its original logs are
preserved alongside the passing direct storage probes.

Inactive ADAT sources outside the installed program set still fail their
optional syntax checks against both the preceding and current installations
(missing historical conversion headers, an old key comparison, and an absent
filename overload). The active retained programs pass. These old failures are
recorded in
[inactive utility inventory](evidence/tools-filedb-20261109-001/adat-inactive-utility-limitations.json).

Initial migration attempts exposed build-list continuations, stale include
paths, an unused private FILEDB include, missing explicit ENSEM qualification,
and the header-only metadata ODR issue. Those attempts are superseded by the
successful final builds and preserved as diagnostics. The requested review
pause interrupted the first Chroma build; resuming used the same source commit.
No failed numerical channels were suppressed to obtain these results.

## Evidence

- [Final build commands/results](evidence/tools-filedb-20261109-001/final-build.json)
- [Dependency, ownership and tool-format checks](evidence/tools-filedb-20261109-001/stack-verification.json)
- [Eloy FILEDB one/two-rank checks](evidence/tools-filedb-20261109-001/eloy-filedb-final-check.json)
- [External mode and distribution](evidence/tools-filedb-20261109-001/package-check-final.json)
- [Full independent regression audit](evidence/tools-filedb-20261109-001/independent-audit.json)

The complete builds, databases, source checkouts and large run artifacts remain
in `{e}`. Versioned evidence contains reports, commands, small probe sources,
provenance and compressed diagnostics, not binaries or database payloads.
'''
# Root and versioned guides intentionally have identical report text.
(r/'FILEDB-TOOLS-REGRESSIONS.md').write_text(report)
(d/'docs/reorg/FILEDB-TOOLS-REGRESSIONS.md').write_text(report)
print('FILEDB ownership and full regression report generated.')
