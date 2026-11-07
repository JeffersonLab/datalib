import json
from pathlib import Path
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
v=json.loads((e/'verification.json').read_text())
assert v['status']=='passed'
commands=json.loads((e/'build.json').read_text())
seconds=sum(row['seconds'] for row in commands)
rows='\n'.join('| `'+row['case']+'` | '+str(row['keys'])+' | '+str(row['values'])+' | '+str(row['max_abs_difference'])+' | Yes |' for row in v['smoke']['cases'])
source='\n'.join('| '+name+' | `'+v[name+'_source']['head']+'` |' for name in ('datalib','chroma'))
counts='\n'.join('| '+name+' | '+str(row['compiler_dependency_files'])+' | 0 |' for name,row in v['builds'].items())
text=f'''# Clean standalone datalib and ADAT-free Chroma builds

7 November 2026. Both clean datalib variants and the complete Chroma build
passed on `rge/devel-datalib-reorg`. Datalib built with ADAT, QDP and QMP
filesystem access denied; Chroma built and ran CORR_SUPERB with ADAT access
denied. No source changes were required.

## Source and isolation

| Clean checkout | Commit |
| --- | --- |
{source}

The source repositories were cloned from the committed reorg branches with
`git clone --no-local --single-branch --branch rge/devel-datalib-reorg`.
Both checkouts were clean before bootstrapping. Chroma's submodules were
checked out at their recorded gitlinks, using local repositories only as
object stores. The builds used new empty build directories and new install
prefixes; no earlier objects, archives or Chroma executables were overlaid.
The audit verified that the tracked library/application C++ sources were
not modified. Autotools regenerated its build metadata only in the isolated
checkouts.

A macOS `sandbox-exec` profile denied reads of ADAT directories, headers and
libraries in both builds. The datalib profile additionally denied QDP/QMP
paths and libraries. Explicit canaries failed to read the installed ADAT
header in both profiles and the installed QDP header in the datalib profile;
QDP remained accessible to Chroma. The build environments were recorded
explicitly, without inherited include or library search variables.

All evidence is under [the clean-build evidence directory](evidence/standalone-clean-20261107-001/).
The [sandbox profiles](evidence/standalone-clean-20261107-001/datalib.sb),
[Chroma profile](evidence/standalone-clean-20261107-001/chroma.sb) and
[canary results](evidence/standalone-clean-20261107-001/sandbox-canaries.json)
record the enforced restrictions.

## Builds and dependencies

Apple Clang, libc++ and libomp built the libraries and applications through
Autotools. Datalib used the standalone xpath_reader array headers and
Homebrew libxml2/libomp. Its core build used `--without-qdp --without-filedb`;
all three primitive, graph and storage checks passed. The second build used
`--without-qdp --with-filedb=.../install/filedb-standalone-001`; all four
checks, including FILEDB, passed. These checks include the frozen format
fixtures. No ADAT, QDP, QMP or MPI dependency was exported by either datalib
configuration. The standalone xpath_reader headers remain a dependency;
the library uses its own isolated XML parser/writer and does not link
`libXPathReader` or `libxmlWriter`.

Chroma used `--with-datalib` pointing to the newly built FILEDB-enabled
installation. It reused the matching, previously built QDP++/QMP and
SuperbBLAS MPI installations and PRIMME support from the recorded no_1pt
stack, plus the standalone FILEDB installation. Those support dependencies
were not rebuilt in this check. They were read in place; Kanoni was not
modified. The complete Chroma application and auxiliary programs were built
and installed. `BUILD_REDSTAR_DATALIB` was enabled, and the final application
link line names the fresh `libdatalib_serial` and no ADAT library.

New install prefixes, relative to this evidence directory, are
`install/datalib-core`, `install/datalib-filedb` and `install/chroma`.
[All build commands and return codes](evidence/standalone-clean-20261107-001/build.json)
and [the explicit environments](evidence/standalone-clean-20261107-001/environments.json)
are preserved. The combined recorded bootstrap/configure/build/check/install
command time was {seconds:.1f} seconds.

The audit checked compiler-generated dependency records:

| Build | Dependency records | Forbidden dependency paths |
| --- | ---: | ---: |
{counts}

It also checked exported compiler/link flags and demangled symbols in both
datalib archives and the Chroma executable. No ADAT/ADATIO/ADATXML/ADATUtil/ENSEM
symbols were present. Chroma's dynamic dependencies include libomp and no
libgomp or libstdc++; datalib's exported flags contain no QDP/QMP/MPI links.
The exact [Chroma link line](evidence/standalone-clean-20261107-001/chroma-link.log)
and [verification record](evidence/standalone-clean-20261107-001/verification.json)
are saved.

## ADAT-free runtime smoke checks

Three cached native-double CORR_SUPERB cases ran with ADAT filesystem access
denied. Each used the saved format-4 graph and unchanged numerical input from
the prior full reorg suite. A new readback program compiled and ran solely
against the fresh datalib core install, with both ADAT and QDP/QMP denied.
It checked exact XML keys, metadata, one-configuration/time shapes, checksums,
finite complex values and the unchanged per-value gate. Every output database
was byte-identical to the saved reorg output; zero CG solves were performed.

| Case | Keys | Finite complex values | Maximum difference from saved CORR | Byte-identical |
| --- | ---: | ---: | ---: | --- |
{rows}

The [smoke results](evidence/standalone-clean-20261107-001/smoke.json) and
[standalone readback source](evidence/standalone-clean-20261107-001/readback.cc)
are saved with the inputs, outputs and logs. The smoke run used one MPI rank
and one OpenMP thread per case. OpenMP was enabled in both libraries and
Chroma. A separate probe built with the fresh installed datalib flags
observed actual teams of one and four threads. This was a build/dependency check with targeted runtime coverage;
it did not repeat the full matrix, on-demand solver, multi-rank MPI, GPU
or CMake validation. Existing numerical deferrals were unchanged.

The final protected-state audit matched all original reorg checkouts and
local `devel` refs, all original `devel` checkout refs, and Kanoni's existing
working state against their starting snapshots. No original repository,
branch or Kanoni file was changed.
'''
(root/'STANDALONE-CLEAN-BUILD.md').write_text(text)
p=root/'README.md'
s=p.read_text()
needle='ColorVec and Redstar builds and full rerun after removing compatibility headers.'
assert needle in s
if '[Clean standalone build verification](STANDALONE-CLEAN-BUILD.md)' not in s:
    s=s.replace(needle,needle+'\n[Clean standalone build verification](STANDALONE-CLEAN-BUILD.md) records fresh\ndatalib builds without ADAT/QDP and a fresh ADAT-free Chroma build and runtime checks.')
p.write_text(s)
print(root/'STANDALONE-CLEAN-BUILD.md')
