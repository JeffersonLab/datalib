from pathlib import Path
import subprocess,json,tarfile,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';source=e/'publication-source/datalib';env=json.loads((e/'environments.json').read_text())['datalib'];b=e/'build/publication-package-storage';b.mkdir();records=[]
for label,cmd in [('configure',[str(source/'configure'),'--prefix='+str(e/'install/publication-package-storage'),'--with-libxml2=/opt/homebrew/opt/libxml2']),('dist',['make','dist'])]:
 with (e/('publication-storage-'+label+'.log')).open('w') as f:p=subprocess.run(cmd,cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':cmd,'returncode':p.returncode});p.check_returncode()
archive=b/'datalib-0.1.tar.gz'
with tarfile.open(archive) as f:
 names=set(f.getnames())
 for rel in ('lib/binary.cc','docs/reorg/CHROMA-STORAGE-REGRESSIONS.md','docs/reorg/evidence/chroma-storage-20261109-001/validation-final.json'):assert 'datalib-0.1/'+rel in names,rel
 assert not any(x.startswith('datalib-0.1/src/') for x in names)
(e/'publication-package.json').write_text(json.dumps({'status':'passed','source_commit':subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip(),'archive':str(archive),'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'files':len(names),'steps':records,'qualification':'Distribution uses the normal bundled-provider configuration so its recursive distribution target has a configured FILEDB subdirectory; the earlier core-only distribution attempt is retained in publication-dist.log. Core builds from the archive passed previously.'},indent=2)+'\n');print('Final committed report/evidence included in the default source distribution')
