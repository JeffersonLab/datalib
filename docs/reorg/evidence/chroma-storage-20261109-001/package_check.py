from pathlib import Path
import json,subprocess,tarfile,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';b=e/'build/datalib-filedb';src=e/'sources/datalib';env=json.loads((e/'environments.json').read_text())['datalib']
profile=(e/'datalib.sb').read_text()+f'\n(allow file-read* (subpath "{src}/docs") (subpath "{b}/datalib-0.1/docs"))\n';(e/'datalib-dist.sb').write_text(profile)
cmd=['/usr/bin/sandbox-exec','-f',str(e/'datalib-dist.sb'),'make','dist']
with (e/'datalib-dist.log').open('w') as f:subprocess.run(cmd,cwd=b,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
archive=b/'datalib-0.1.tar.gz'
with tarfile.open(archive) as f:
 names=set(f.getnames());assert not any(x.startswith('datalib-0.1/src/') for x in names)
 for rel in ('lib/binary.cc','lib/xml/basic_xpath_reader.cc','main/mod_reader.h','main/dbutil.cc','include/datalib/storage/record_store.h','tests/records_probe.cc'):
  assert 'datalib-0.1/'+rel in names,rel
 out=e/'dist-source';out.mkdir();f.extractall(out)
source=out/'datalib-0.1';build=e/'build/dist-core';build.mkdir()
commands=[]
for label,cmd in [('configure',[str(source/'configure'),'--prefix='+str(e/'install/dist-core'),'--without-filedb','--with-libxml2=/opt/homebrew/opt/libxml2']),('build',['make','-j3']),('check',['make','check','-j3'])]:
 command=['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*cmd]
 with (e/('dist-core-'+label+'.log')).open('w') as f:p=subprocess.run(command,cwd=build,env=env,stdout=f,stderr=subprocess.STDOUT)
 commands.append({'step':label,'command':command,'returncode':p.returncode});p.check_returncode()
(e/'package-check.json').write_text(json.dumps({'status':'passed','archive':str(archive),'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'files':len(names),'lib_layout':True,'no_src':True,'build_from_unpacked_distribution':commands},indent=2)+'\n');print('Source distribution uses lib and builds independently from the unpacked archive')
