from pathlib import Path
import subprocess,json,gzip,hashlib,re,sys,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';d=r/'datalib';assert (e/'finish.exit').read_text().strip()=='0'
subprocess.run([sys.executable,str(Path(__file__).parent/'report.py')],check=True)
subprocess.run([sys.executable,str(Path(__file__).parent/'final_docs_updates.py')],check=True)
for p in Path(__file__).parent.iterdir():
 if p.suffix in ('.py','.cc'):(e/p.name).write_bytes(p.read_bytes())
p=d/'docs/reorg/FILEDB-TOOLS-REGRESSIONS.md';text=p.read_text();first=text.index('\n');p.write_text(text[:first]+'\n\nThe later [direct Chroma storage checkpoint](CHROMA-STORAGE-REGRESSIONS.md)\nsupersedes the deferred writer modernization below and records STOCH_HADRON\nremoval. This report is retained as the earlier tested source checkpoint.\n'+text[first:])
p=d/'tests/fixtures/chroma-records.json';a=json.loads(p.read_text());a['provenance']='Frozen before wrapper removal using the previous installed Chroma code; branch tip b067c360 contains documentation-only changes relative to the installed writer code.';a['installed_chroma_code_commit']='af17465139e0d82cca9e4ae20c1ef28f76e3d741';p.write_text(json.dumps(a,indent=2)+'\n')
p=d/'docs/reorg/README.md';s=p.read_text().replace('- [FILEDB ownership','- [Direct Chroma storage, removed STOCH_HADRON and lib layout](CHROMA-STORAGE-REGRESSIONS.md)\n- [FILEDB ownership',1);s+='\nThe later 9 November storage checkpoint supersedes the earlier deferred\nlegacy-writer modernization. Chroma uses datalib storage directly; temporary\nwrappers and STOCH_HADRON are removed. Datalib sources use lib/, and no QDP\nconfigure switch remains. See the latest report above for the full rerun.\n';p.write_text(s)
p=d/'README.md';s=p.read_text().replace('records the tested stack, unchanged old Chroma measurement sources and retained\nQDP++ platform-test limitations.', 'records the earlier ownership checkpoint and its retained platform-test limits.');s+='''
[Direct Chroma storage and full regression](docs/reorg/CHROMA-STORAGE-REGRESSIONS.md)
records the subsequent writer migration, removal of temporary wrappers and
STOCH_HADRON, consistent SuperbBLAS storage profiles, and the lib/ source layout.
Datalib has no QDP or ADAT configure dependency. Chroma retains its own QDP/MPI
coordination and matching SuperbBLAS configuration.
''';p.write_text(s)
p=r/'README.md';s=p.read_text()+'\n[Direct Chroma storage and lib-layout regression](CHROMA-STORAGE-REGRESSIONS.md)\nsupersedes the deferred writer modernization: temporary Chroma wrappers and\nSTOCH_HADRON are removed; datalib sources now live in lib/.\n';p.write_text(s)
folder=d/'docs/reorg/evidence/chroma-storage-20261109-001';folder.mkdir(parents=True)
selected=[p for p in e.iterdir() if p.is_file() and p.suffix in ('.json','.py','.cc','.sb','.cmake')]
for lane in ('paired','npt-only'):
 selected += [e/lane/'command.json',e/lane/'suite/results.json']
for p in (e/'producer-smoke').rglob('*'):
 if p.is_file() and p.name in ('input.xml','command.json'):selected.append(p)
for group in ('storage-abi-before','storage-abi-after'):
 selected += [p for p in (e/group).iterdir() if p.suffix=='.cc']
logs=[p for p in e.iterdir() if p.is_file() and p.suffix=='.log']
logs += [p for p in (e/'producer-smoke').rglob('run.log')]
for group in ('storage-abi-before','storage-abi-after'):logs += list((e/group).glob('*.log'))
for lane in ('paired','npt-only'):logs += list((e/lane).glob('*.log*'))
for p in selected:
 target=folder/p.relative_to(e);target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(p.read_bytes())
for p in logs:
 target=folder/p.relative_to(e);target=target if p.suffix=='.gz' else target.with_suffix(target.suffix+'.gz');target.parent.mkdir(parents=True,exist_ok=True)
 if p.suffix=='.gz':target.write_bytes(p.read_bytes())
 else:
  with p.open('rb') as src,target.open('wb') as out:
   with gzip.GzipFile(filename='',mode='wb',fileobj=out,mtime=0) as dst:shutil.copyfileobj(src,dst)
files=sorted(p for p in folder.rglob('*') if p.is_file());assert all(p.stat().st_size<5*1024*1024 for p in files)
(folder/'publication.json').write_text(json.dumps({'files':{str(p.relative_to(folder)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},'excluded':'Build/source trees, programs, SDB/SBT payloads and generated numerical databases stay outside Git.'},indent=2)+'\n');files.append(folder/'publication.json')
p=d/'Makefile.am';s=p.read_text()+'\n# Direct Chroma storage, source layout and full regression evidence.\nEXTRA_DIST += \\\n    docs/reorg/CHROMA-STORAGE-REGRESSIONS.md \\\n'+' \\\n'.join('    '+str(p.relative_to(d)) for p in files)+'\n';p.write_text(s)
for m in re.finditer(r'\]\(([^)]+)\)',(d/'docs/reorg/CHROMA-STORAGE-REGRESSIONS.md').read_text()):
 ref=m.group(1)
 if '://' not in ref:assert (d/'docs/reorg'/ref).exists(),ref
print('Published bounded report/evidence:',len(files),'files',sum(p.stat().st_size for p in files),'bytes')
