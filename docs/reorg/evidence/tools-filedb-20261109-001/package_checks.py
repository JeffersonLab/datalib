from pathlib import Path
import subprocess,json,tarfile,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';src=e/'sources/datalib';env=json.loads((e/'environments.json').read_text())['datalib'];b=e/'build/datalib-external';b.mkdir()
rows=[]
for step,cmd in [('configure',[str(src/'configure'),'--prefix='+str(e/'install/datalib-external'),'--without-qdp','--with-libxml2=/opt/homebrew/opt/libxml2','--with-filedb='+str(e/'install/datalib-filedb')]),('build',['make','-j3']),('check',['make','check','-j3'])]:
 command=['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*cmd]
 with (e/('external-'+step+'.log')).open('w') as f:p=subprocess.run(command,cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT)
 rows.append({'step':step,'command':command,'returncode':p.returncode});p.check_returncode()
# Historical documentation includes filenames with 'adat'; permit only this package's docs for distribution.
profile=(e/'datalib.sb').read_text()+f'\n(allow file-read* (subpath "{src}/docs") (subpath "{e}/build/datalib-filedb/datalib-0.1/docs"))\n';(e/'datalib-dist.sb').write_text(profile)
cmd=['/usr/bin/sandbox-exec','-f',str(e/'datalib-dist.sb'),'make','dist']
with (e/'datalib-dist.log').open('w') as f:p=subprocess.run(cmd,cwd=e/'build/datalib-filedb',env=env,stdout=f,stderr=subprocess.STDOUT)
p.check_returncode();archive=e/'build/datalib-filedb/datalib-0.1.tar.gz'
with tarfile.open(archive) as f:
 names=set(f.getnames())
 for path in ('main/dbutil.cc','main/view_corr_graph.cc','main/dump_corr_graph.cc','include/datalib/graph/corr_graph.h','include/datalib/storage/corr_reader.h','other_libs/filedb/src/FileDB.cpp'):
  assert 'datalib-0.1/'+path in names,path
 assert 'datalib-0.1/include/datalib/graph/process_corrs.h' not in names
(e/'package-check.json').write_text(json.dumps({'status':'passed','external_filedb':rows,'source_archive':str(archive),'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'files':len(names),'note':'Distribution-only sandbox grants source/dist docs read access; production builds use the unchanged dependency-denying sandbox.'},indent=2)+'\n')
print('External FILEDB mode and complete source distribution passed.')
