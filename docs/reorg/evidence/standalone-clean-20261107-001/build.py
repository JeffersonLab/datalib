from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import time

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
environments=json.loads((e/'environments.json').read_text())
records=[]

def run(label,argv,directory,profile):
    args=list(map(str,argv))
    command=['/usr/bin/sandbox-exec','-f',str(e/(profile+'.sb')),*args]
    print(label,flush=True)
    start=time.monotonic()
    with (e/(label+'.log')).open('w') as log:
        result=subprocess.run(command,cwd=directory,env=environments[profile],stdout=log,stderr=subprocess.STDOUT)
    records.append({'step':label,'command':args,'sandbox_command':command,'cwd':str(directory),
                    'profile':profile,'returncode':result.returncode,'seconds':time.monotonic()-start,
                    'finished_utc':datetime.now(timezone.utc).isoformat()})
    (e/'build.json').write_text(json.dumps(records,indent=2)+'\n')
    result.check_returncode()

for name in ('datalib','chroma'):
    run(name+'-bootstrap',['autoreconf','-fi'],e/'sources'/name,name)

for label,options in [('datalib-core',['--without-filedb']),('datalib-filedb',['--with-filedb='+str(root/'install/filedb-standalone-001')])]:
    build=e/'build'/label
    build.mkdir()
    assert not list(build.iterdir())
    prefix=e/'install'/label
    assert not prefix.exists()
    run(label+'-configure',[e/'sources/datalib/configure','--prefix='+str(prefix),
        '--without-qdp','--enable-openmp','--with-libxml2=/opt/homebrew/opt/libxml2',
        '--with-xpath='+str(root/'install/xpath_reader'),*options],build,'datalib')
    for step,args in [('build',['make','-j6']),('check',['make','check','-j6']),('install',['make','install'])]:
        run(label+'-'+step,args,build,'datalib')

reference=root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002'
label='chroma'
build=e/'build'/label
build.mkdir()
assert not list(build.iterdir())
prefix=e/'install'/label
assert not prefix.exists()
run('chroma-configure',[e/'sources/chroma/configure','--prefix='+str(prefix),
    '--with-qdp='+str(reference/'install/qdpxx-mpi'),
    '--with-superbblas='+str(reference/'install/superbblas-mpi'),
    '--with-datalib='+str(e/'install/datalib-filedb'),
    '--with-primme='+str(reference/'sources/primme'),
    '--enable-mgproton','--disable-cpp-wilson-dslash','--enable-testcase-runner=trivial',
    '--with-libxml2=/opt/homebrew/opt/libxml2','--with-gmp=/opt/homebrew/opt/gmp'],build,'chroma')
run('chroma-build',['make','-j6'],build,'chroma')
run('chroma-install',['make','install'],build,'chroma')
(e/'build.exit').write_text('0\n')
print('Clean standalone datalib and Chroma builds completed.',flush=True)
