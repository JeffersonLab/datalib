from pathlib import Path
import subprocess,json,tarfile,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';source=e/'publication-source/datalib';source.parent.mkdir();env=json.loads((e/'environments.json').read_text())['datalib'];records=[]
def run(label,cmd,cwd):
 with (e/('publication-'+label+'.log')).open('w') as f:p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':list(map(str,cmd)),'returncode':p.returncode});p.check_returncode()
run('clone',['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',r/'datalib',source],e)
# Resolve the actual gitmodule section name rather than assuming its label.
for line in subprocess.check_output(['git','-C',str(source),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
 key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(r/'datalib'/path)],check=True)
run('submodules',['git','-c','protocol.file.allow=always','submodule','update','--init','--recursive'],source)
run('bootstrap',['autoreconf','-fi'],source)
b=e/'build/publication-package';b.mkdir();run('configure',[source/'configure','--prefix='+str(e/'install/publication-package'),'--without-filedb','--with-libxml2=/opt/homebrew/opt/libxml2'],b);run('dist',['make','dist'],b)
archive=b/'datalib-0.1.tar.gz'
with tarfile.open(archive) as f:
 names=set(f.getnames());assert 'datalib-0.1/lib/binary.cc' in names;assert 'datalib-0.1/docs/reorg/CHROMA-STORAGE-REGRESSIONS.md' in names;assert 'datalib-0.1/docs/reorg/evidence/chroma-storage-20261109-001/validation-final.json' in names;assert not any(x.startswith('datalib-0.1/src/') for x in names)
(e/'publication-package.json').write_text(json.dumps({'status':'passed','source_commit':subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip(),'archive':str(archive),'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'files':len(names),'steps':records},indent=2)+'\n');print('Final committed documentation and evidence included in source distribution')
