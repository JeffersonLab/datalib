from pathlib import Path
import json,shutil,hashlib,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';a=json.loads((e/'independent-audit.json').read_text());assert a['status']=='passed_with_existing_qdp_test_limitation'
maxerr=a['max_paired_absolute_error'];rss=a['max_rank_rss_kib']/1024
report=f'''# Fresh MPI parscalar validation — 2026-11-10

The dbtools extraction and its original regression documentation were committed
and pushed before this MPI work:

| Repository | Pushed reorg commit |
|---|---|
| datalib | `96d47257d7100a58047831c9f0a22612a028e937` |
| ADAT | `a934f3307bf68ba670a1043298e158697cecc845` |
| Redstar | `bca8a45c584c7aae588348579ca9af6bc512957e` |

Only `rge/devel-datalib-reorg` was pushed. Devel, Kanoni, preexisting submodule
work and configure backups were preserved. This MPI validation introduces no
production C++ changes or merges.

## Fresh build configuration

New empty build/install directories and clean source clones were used under
`evidence/mpi-parscalar-20261110-001`. They are not overlays on prior installs.
QMP and SuperbBLAS also have local checkouts on `rge/devel-datalib-reorg` in the
reorg workspace; their matching upstream source commits were reused without
pulling or merging new code.

- QMP: `--with-qmp-comms-type=MPI`, built with the MPI C wrapper.
- QDP++: `--enable-parallel-arch=parscalar --enable-Nd=4
  --enable-precision=double --enable-openmp --with-qmp=FRESH_QMP_PREFIX`.
  The installed configuration reports `parscalar`; QMP reports `MPI`.
- Serial and MPI SuperbBLAS: fresh CPU builds from the same pinned source,
  OpenMP enabled, with copied installed headers rather than source symlinks.
- Datalib: fresh bundled-FILEDB build using serial SuperbBLAS for its optional
  standalone tensor tools. **11/11 tests passed** with ADAT, QDP/QMP and
  external xpath_reader access denied.
- Chroma: fresh build against that datalib, fresh parscalar QDP++ and fresh
  MPI SuperbBLAS, with ADAT and external xpath_reader access denied during
  compilation and on every executing MPI rank.

Apple Clang/C++20, libomp, Accelerate and Open MPI 5.0.7 were used. Both the
Chroma executable and the combined boundary probe link libmpi and libomp,
without libgomp or libstdc++. The existing matching PRIMME source/install was
reused; no new PRIMME, MPI implementation, gauge data or solver qualification
is implied.

## Communication and XML boundary

Fresh one- and two-rank programs passed these independent checks:

- MPI world size agrees with QMP's node count and node IDs.
- A distributed 4x4x4x16 QDP lattice has the correct global sum, 1024, and
  consistent local volume (512 sites per rank with two ranks).
- Only the primary rank receives valid QDP XML; a nested QDP reader broadcasts
  the scalar value 72 to every rank. Nonprimary input is deliberately invalid.
- Fully qualified datalib XML calls retain distinct local scalars (37 and 38)
  and per-rank arrays. Linking QDP/MPI does not make datalib serial reads
  collective or add a hidden broadcast.
- Every rank forms an actual two-worker OpenMP team. MPI reports thread
  support level 3 for this Open MPI implementation.

These probes confirm that QDP's collective I/O and datalib's independent
libxml2-backed I/O coexist in the same executable with explicit namespaces.

## Real producer and full numerical regressions

Fresh real meson and baryon producer runs passed at one and two MPI ranks,
with independent datalib record readback. Each has 32 keys; mesons have 128
complex values and baryons have 256. Old/new record payloads match byte-for-byte
at each fixed rank count. New one-versus-two-rank maxima are 7.77156e-16 for
mesons and 3.06904e-17 for baryons. Metadata matches except explicitly checked
run timestamps, rank geometry and the automatic timeslice-batch execution setting.
Physics fields are unchanged. These are targeted single-configuration software
checks using the real native color-vector input.

The full native-double matrix then passed with two Chroma MPI ranks:

| Lane | Passed cases | Keys | Finite complex values |
|---|---:|---:|---:|
| Paired NPT/CORR_SUPERB | 30 | 925 | 4197 |
| NPT-only, separately qualified | 3 | 6 | 96 |

The strict per-value gate, exact key/time/shape checks, immutable dataset hashes,
explicit direct-DISCO supplement and all original user deferrals remain in
place. Maximum paired absolute difference: **{maxerr:.16g}**. The independent
audit verifies all 30 output XML reports describe a genuine two-rank geometry,
full finite readback, graph format 4, XML schema version 14, matched physics
flags, zero CG solves, unchanged unevaluatable-graph accounting and the exactly
zero eta control. NPT-only passes do not establish deferred CORR support.

Chroma used **two MPI ranks and two OpenMP workers per rank**. The MPI wrapper
sets the worker count explicitly because the unchanged Kanoni suite runner
otherwise forces `OMP_NUM_THREADS=1`. The NPT reference remains the previously
validated single-process implementation with one OpenMP worker; NPT and the
standalone database tools are not launched as duplicated MPI writers.

Every actual Chroma rank is separately monitored, rather than monitoring only
the MPI launcher: 60 rank records have positive RSS samples, successful exits,
an 8192 MiB per-rank cap and a timeout shorter than the outer case timeout.
The maximum observed rank RSS was **{rss:.2f} MiB**. `SB_CACHEGB_CPU=1` and the
2 GiB contraction planner remain unchanged. No memory cap or numerical gate
was weakened to obtain these results.

This establishes single-host, two-rank CPU functionality for the cached-tensor
path. It does not establish MPI scaling, multihost execution, GPU operation,
new CG solves or full on-demand GenProp. Historical latest-YLM qualifications
remain separate.

## Retained QDP test failure and build-driver corrections

QMP's build/check/install commands passed, and QDP's library/examples build and
install passed. **QDP's stock `make check` did not pass**: its legacy SSE
unit/benchmark programs reference unavailable `vaxpy4`, `vaxpyz4` and `vaxmy4`
helpers in this generic Apple-arm64 configuration. The same failure is present
in the earlier reorg QDP check log. It was not hidden, patched out or labelled
as a successful test. No QDP test/source implementation was changed. Functional
MPI probes, producer checks and the full Chroma matrix passed independently.

An initial serial-SuperbBLAS build-driver invocation omitted the separate
OpenMP flags. That attempt was preserved, and an empty build/install profile
was rebuilt with the flags before datalib configuration; no installed overlay
was used. The first full-suite driver attempt referenced the old datalib prefix
name and stopped before any numerical case started. The corrected attempt is
retained with the original diagnostic.

The previous dbtools report inferred four OpenMP workers from the outer
build/run environment. The suite runner actually forces one. Its local current
qualification has been corrected; the independent one-/four-worker team probe
remains valid. Frozen historical evidence is unchanged. This qualification
correction and this new MPI validation have not been committed or pushed.

## Evidence and reproduction

Complete evidence is retained under `evidence/mpi-parscalar-20261110-001`:
`build.json`, `continued-build.json`, `sources.json`, `environments.json`,
`serial-openmp-build.json`, `qdpxx-check-limitation.json`, `boundary-probe.json`,
`guard-probes.json`, `producer-comparison.json`, `paired/suite/results.json`,
`npt-only/suite/results.json` and `independent-audit.json`. The original failed
QDP test log and prior counterpart are retained. Large completed diagnostics
are losslessly compressed with verified decompression hashes. Numerical inputs,
outputs, binaries and all source snapshots remain local and outside Git.

The build, corrected continuation, per-rank guard, full-suite driver and audit
are saved alongside these records. No devel or Kanoni content was changed.
'''
(r/'MPI-PARSCALAR-VALIDATION.md').write_text(report)
for p in Path('/tmp/reorg-mpi').glob('*'):
 if p.suffix in ('.py','.cc'):shutil.copy2(p,e/p.name)
for name in ('build','continue'):
 p=Path('/tmp/reorg-mpi')/(name+'-driver.log')
 if p.exists():shutil.copy2(p,e/p.name)
# Current commit heads/index/protected files, including Kanoni untracked work.
pub=r/'evidence/dbtools-publish-20261110-001';before=json.loads((pub/'before.json').read_text());commits=json.loads((pub/'commits.json').read_text())
def git(p,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(p),*args],text=True)
for name,old in before['reorg'].items():
 p=r/name;assert git(p,'rev-parse','HEAD').strip()==commits.get(name,old['head']);assert git(p,'diff','--cached','--binary')==old['index'];assert git(p,'for-each-ref','--format=%(refname) %(objectname)','refs/heads/devel','refs/remotes/origin/devel')==old['devel']
for name,old in before['protected'].items():
 p=r.parent/name;assert git(p,'rev-parse','HEAD')==old['head'];assert git(p,'status','--short')==old['status'];assert git(p,'rev-parse','refs/heads/devel')==old['devel'];assert hashlib.sha256(git(p,'diff','--binary','HEAD').encode()).hexdigest()==old['diff_sha256'];assert hashlib.sha256(git(p,'diff','--cached','--binary').encode()).hexdigest()==old['index_sha256']
 for path,digest in old.get('untracked_sha256',{}).items():assert hashlib.sha256((p/path).read_bytes()).hexdigest()==digest
(e/'protected-state.json').write_text(json.dumps({'status':'passed','published_heads_unchanged':commits,'indexes_unchanged':True,'local_and_primary_devel_unchanged':True,'kanoni_tracked_and_untracked_work_unchanged':True,'new_mpi_report_uncommitted':True,'local_documentation_correction':'datalib/docs/reorg/DBTOOLS-EXTRACTION-REGRESSIONS.md'},indent=2)+'\n')
print('MPI report saved; all protected state preserved.')
