from pathlib import Path
import subprocess,json,gzip,hashlib,re,shutil,sys
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';d=r/'datalib'
assert (e/'finish.exit').read_text().strip()=='0'
subprocess.run([sys.executable,str(Path(__file__).parent/'report.py')],check=True)
# Keep the scripts and selected source/command records with the run, then publish bounded evidence.
for p in Path(__file__).parent.iterdir():
 if p.suffix in ('.py','.cc'):shutil.copy2(p,e/p.name)
folder=d/'docs/reorg/evidence/tools-filedb-20261109-001';folder.mkdir(parents=True,exist_ok=True)
selected=[]
for p in e.iterdir():
 if p.is_file() and p.suffix in ('.json','.py','.cc','.sb'):
  if p.name in ('chroma-final-link-raw.json',):continue
  selected.append(p)
for lane in ('paired','npt-only'):
 for name in ('command.json','suite/results.json'):selected.append(e/lane/name)
# Logs stay compressed. Numerical databases, binaries, graph XML and build/source trees stay outside Git.
lognames=[p for p in e.iterdir() if p.is_file() and p.suffix=='.log']
for p in lognames:
 target=folder/(p.name+'.gz');payload=p.read_bytes()
 with target.open('wb') as out:
  with gzip.GzipFile(filename='',mode='wb',fileobj=out,mtime=0) as stream:stream.write(payload)
 assert gzip.decompress(target.read_bytes())==payload
for p in selected:
 rel=p.relative_to(e);rel=Path('initial-source-commits.json') if str(rel)=='source-commits.json' else (Path('initial-build.json') if str(rel)=='build.json' else rel);target=folder/rel;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(p.read_bytes())
# Preserve final numerical logs already compressed by the independent audit.
for lane in ('paired','npt-only'):
 for p in (e/lane).glob('*.log*'):
  if p.suffix=='.gz':target=folder/p.relative_to(e);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(p.read_bytes())
report=d/'docs/reorg/FILEDB-TOOLS-REGRESSIONS.md';p=d/'docs/reorg/README.md';s=p.read_text().replace('- [Bundled array headers', '- [FILEDB ownership, standalone tools and CORR_SUPERB regression](FILEDB-TOOLS-REGRESSIONS.md)\n- [Bundled array headers');s+='\nThe 9 November checkpoint moves FILEDB ownership and diagnostic tools to\ndatalib, retains old Chroma measurement calls, verifies the CORR_SUPERB build\ngates, and records the full numerical matrix plus existing platform limits.\n';p.write_text(s)
p=d/'README.md';s=p.read_text()+'''\n## CORR_SUPERB in Chroma\n\nConfigure Chroma with `--with-datalib=PREFIX` and\n`--with-superbblas=PREFIX` to enable the CORR_SUPERB measurement. The normal\nFILEDB-enabled datalib installation already provides its shared graph, key,\ncodec, storage and evaluator headers. Chroma uses its own matching QDP/QMP and\nSuperbBLAS communication configuration; ADAT is not a dependency. Datalib's\n`--with-superbblas` option enables the separate standalone dbutil tensor backend.\n\n[FILEDB ownership and full CORR_SUPERB regression](docs/reorg/FILEDB-TOOLS-REGRESSIONS.md)\nrecords the tested stack, unchanged old Chroma measurement sources and retained\nQDP++ platform-test limitations.\n''';p.write_text(s)
# Update current ADAT direct-API guidance.
p=r/'adat/docs/datalib-api.md';s=p.read_text().replace('datalib/graph/process_corrs.h','datalib/graph/corr_graph.h');s+='''\nFILEDB is supplied by datalib's pinned `other_libs/filedb` submodule or its\nexplicit external provider. ADAT no longer builds FILEDB or installs dbutil.\nUse datalib's `bin/dbutil`; metadata and standard correlator access are in\n`datalib/storage/dbtype.h` and `datalib/storage/corr_reader.h`. ENSEM conversions\nand the remaining ENSEM/physics database utilities stay in ADAT.\n''';p.write_text(s)
# Redstar keeps workflow guides while the standalone executables move.
for name in ('view_corr_graph','dump_corr_graph'):
 p=r/f'redstar/docs/{name}.md';s=p.read_text();first=s.index('\n');s=s[:first]+'''\n\nOn `rge/devel-datalib-reorg`, this executable is built and installed by\nstandalone datalib. Put that installation's `bin/` on PATH before running\nthe examples below. The CLI and format-4 output are unchanged.\n'''+s[first:]
 s=s.replace('"$repo/build/src/'+name+'"',name).replace('`../src/'+name+'.cc`','`datalib/main/'+name+'.cc`').replace('`../lib/process_corrs.cc`','`datalib/src/corr_graph.cc`');p.write_text(s)
p=r/'redstar/docs/README.md';s=p.read_text().replace('"$repo/build/src/view_corr_graph"','view_corr_graph').replace('"$repo/build/src/dump_corr_graph"','dump_corr_graph');s+='\nOn the reorg branch, obtain `view_corr_graph`, `dump_corr_graph` and `dbutil`\nfrom the matching standalone datalib installation and put its `bin/` on PATH.\n';p.write_text(s)
p=r/'redstar/docs/build.md';s=p.read_text();i=s.index('\n');s=s[:i]+'''\n\nReorg branch note: `rge/devel-datalib-reorg` uses datalib as FILEDB provider\nand installs graph viewers/dbutil from datalib. The source-pinning helper below\nretains its historical devel-stack recipe; the exact tested reorg build commands\nare recorded in datalib's `docs/reorg/FILEDB-TOOLS-REGRESSIONS.md` and evidence.\n'''+s[i:];p.write_text(s)
# Chroma and QDP build-boundary notes are local to each repository.
p=r/'chroma/docs/datalib-reorg.md';p.write_text('''# Standalone datalib and CORR_SUPERB\n\nThis reorg branch requires `--with-datalib=PREFIX` for shared data and FILEDB\nsupport. Add `--with-superbblas=PREFIX` to enable the SUPERB measurements,\nincluding CORR_SUPERB. Both BUILD_SB and BUILD_REDSTAR_DATALIB must be defined.\nUse the same compiler/OpenMP runtime and matching QDP/QMP/SuperbBLAS builds.\nNo ADAT installation is required.\n\nDatalib owns FILEDB and the typed meson/baryon/propagator/GenProp/DISCO storage\ninterfaces. The older measurement code is unchanged. Its former QDP FILEDB\nadapter is retained as `util/ferm/filedb_qdp.h` until that code is modernized.\nThe data-parallel XML boundary and SuperbBLAS communicator selection are unchanged.\n\nDatalib's `docs/reorg/FILEDB-TOOLS-REGRESSIONS.md` records the full native-double\npaired suite, retained deferrals, clean source provenance and platform-test limits.\n''')
p=r/'chroma/README';p.write_text(p.read_text()+'\nReorg datalib/FILEDB and CORR_SUPERB build notes: docs/datalib-reorg.md\n')
p=r/'chroma/Makefile.am';p.write_text(p.read_text()+'\nEXTRA_DIST += docs/datalib-reorg.md\n')
p=r/'qdpxx/docs/filedb-reorg.md';p.parent.mkdir(exist_ok=True);p.write_text('''# FILEDB ownership on the reorg branch\n\nQDP++ no longer includes the FILEDB submodule, database wrapper headers,\n--enable-filedb option, libraries or exported FILEDB link flags. It does not\ngain a datalib dependency. Chroma obtains FILEDB from installed datalib and\nretains its old database adapter locally while legacy writers are modernized.\nQDP's XML and lattice I/O/communication behavior is unchanged.\n\nThe macOS ARM library/examples build succeeded. Full make check still fails\nin unchanged upstream x86 SIMD tests, reproduced on the previous installation.\nSee datalib's docs/reorg/FILEDB-TOOLS-REGRESSIONS.md and recorded evidence.\n''')
p=r/'qdpxx/README';p.write_text(p.read_text()+'\nReorg FILEDB dependency change and platform-check scope: docs/filedb-reorg.md\n')
p=r/'qdpxx/Makefile.am';p.write_text(p.read_text()+'\nEXTRA_DIST += docs/filedb-reorg.md\n')
# Shared workspace index, outside any source repository.
p=r/'README.md';s=p.read_text()+'\n[FILEDB ownership, standalone tools and full CORR_SUPERB regression](FILEDB-TOOLS-REGRESSIONS.md)\nrecords the 9 November checkpoint. FILEDB is now owned by datalib; old Chroma\nmeasurement modernization remains deferred.\n';p.write_text(s)
# Record published payload hashes, then include all new evidence in the distribution manifest.
files=sorted(p for p in folder.rglob('*') if p.is_file());manifest={'report':str(report.relative_to(d)),'report_sha256':hashlib.sha256(report.read_bytes()).hexdigest(),'files':{str(p.relative_to(folder)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files}}
(folder/'publication.json').write_text(json.dumps(manifest,indent=2)+'\n');files.append(folder/'publication.json')
p=d/'Makefile.am';s=p.read_text()+'\n# FILEDB/tool extraction and full native-double regression evidence.\nEXTRA_DIST += \\\n    docs/reorg/FILEDB-TOOLS-REGRESSIONS.md \\\n'+ ' \\\n'.join('    '+str(p.relative_to(d)) for p in files)+'\n';p.write_text(s)
# Check local report links and every new source-distribution entry.
for m in re.finditer(r'\]\(([^)]+)\)',report.read_text()):
 ref=m.group(1)
 if '://' not in ref:assert (report.parent/ref).exists(),ref
for p in files:assert p.stat().st_size<5*1024*1024,p
print('Published report and',len(files),'bounded evidence files; local links and hashes checked.')
