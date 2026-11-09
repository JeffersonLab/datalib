from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import threading
import time
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/chroma-storage-20261109-001'
reference=root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002'
prior=root/'evidence/standalone-clean-20261107-001'
environments=json.loads((prior/'environments.json').read_text())
environments['downstream']=json.loads((root/'evidence/native-double-clean-full-20261107-001/environment.json').read_text())
for env in environments.values():env['SB_CACHEGB_CPU']='1';env['PYTHONDONTWRITEBYTECODE']='1'
environments['datalib']['LDFLAGS']+=' -framework Accelerate'
(e/'environments.json').write_text(json.dumps(environments,indent=2)+'\n')
adat=r'.*/(adat[^/]*|libadat[^/]*)(/|$)'
qdp=r'.*/(qdp[^/]*|qmp[^/]*|libqdp[^/]*|libqmp[^/]*)(/|$)'
xpath=r'.*/xpath_reader[^/]*(/|$)'
for name,expressions in [('datalib',[adat,qdp,xpath,r'.*/libXPathReader[^/]*(/|$)',r'.*/libxmlWriter[^/]*(/|$)']),('chroma',[adat,xpath])]:
    (e/(name+'.sb')).write_text('(version 1)\n(allow default)\n'+''.join('(deny file-read* (regex "'+pattern+'"))\n' for pattern in expressions))
canaries=[]
for profile,file,allowed in [('datalib',root/'install/xpath_reader/include/xml_array.h',False),('datalib',reference/'install/qdpxx-mpi/include/qdp.h',False),('datalib',root/'install/adat-direct-001/include/io/superb_kv.h',False),('chroma',root/'install/xpath_reader/include/xml_array.h',False),('chroma',reference/'install/qdpxx-mpi/include/xml_array.h',True)]:
    assert file.is_file()
    result=subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/(profile+'.sb')),'/bin/cat',str(file)],env=environments[profile],stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
    assert (result.returncode==0)==allowed,(profile,file,result.stderr)
    canaries.append({'profile':profile,'file':str(file),'allowed':allowed,'returncode':result.returncode,'stderr':result.stderr})
(e/'sandbox-canaries.json').write_text(json.dumps(canaries,indent=2)+'\n')
records=[]
lock=threading.Lock()
def run(label,args,directory,kind):
    args=list(map(str,args));command=args
    if kind in ('datalib','chroma'):command=['/usr/bin/sandbox-exec','-f',str(e/(kind+'.sb')),*args]
    print(label,flush=True);start=time.monotonic()
    with (e/(label+'.log')).open('w') as log:status=subprocess.run(command,cwd=directory,env=environments[kind],stdout=log,stderr=subprocess.STDOUT)
    row={'step':label,'command':args,'executed_command':command,'cwd':str(directory),'environment':kind,'returncode':status.returncode,'seconds':time.monotonic()-start,'finished_utc':datetime.now(timezone.utc).isoformat()}
    with lock:
        records.append(row);(e/'build.json').write_text(json.dumps(records,indent=2)+'\n')
    status.check_returncode()
def git(repo,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)
sources={}
for name in ('datalib','chroma','adat','colorvec','redstar'):
    source=e/'sources'/name
    with (e/(name+'-clone.log')).open('w') as log:
        subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(root/name),str(source)],stdout=log,stderr=subprocess.STDOUT,check=True)
    assert not git(source,'status','--porcelain=v1')
    assert git(source,'rev-parse','HEAD')==git(root/name,'rev-parse','HEAD')
    if (source/'.gitmodules').exists():
        for line in git(source,'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$').splitlines():
            key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(root/name/path)],check=True)
        with (e/(name+'-submodules.log')).open('w') as log:
            subprocess.run(['git','-C',str(source),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],stdout=log,stderr=subprocess.STDOUT,check=True)
        assert not git(source,'status','--porcelain=v1')
        (e/(name+'-submodule-commits.log')).write_text(git(source,'submodule','status','--recursive'))
    sources[name]={'source':str(source),'commit':git(source,'rev-parse','HEAD').strip(),'branch':git(source,'branch','--show-current').strip(),'status_before_bootstrap':git(source,'status','--porcelain=v1')}
(e/'stack-sources.json').write_text(json.dumps(sources,indent=2)+'\n')
(e/'reorg-sources.log').write_text(''.join(name+' '+item['commit']+'\n' for name,item in sources.items()))
(e/'reorg-validation-heads.json').write_text(json.dumps({name:{'head':git(root/name,'rev-parse','HEAD').strip(),'status':git(root/name,'status','--short')} for name in ('datalib','chroma','adat','colorvec','redstar','xpath_reader','qdpxx')},indent=2)+'\n')
(e/'build').mkdir(exist_ok=True)
run('datalib-bootstrap',['autoreconf','-fi'],e/'sources/datalib','datalib')
for label,options in [('datalib-core',['--without-filedb']),('datalib-filedb',['--with-filedb=bundled','--with-superbblas='+str(reference/'install/superbblas')])]:
    build=e/'build'/label;build.mkdir();prefix=e/'install'/label;assert not prefix.exists()
    run(label+'-configure',[e/'sources/datalib/configure','--prefix='+str(prefix),'--enable-openmp','--with-libxml2=/opt/homebrew/opt/libxml2',*options],build,'datalib')
    for step,args in [('build',['make','-j6']),('check',['make','check','-j6']),('install',['make','install'])]:run(label+'-'+step,args,build,'datalib')
(e/'datalib-build.exit').write_text('0\n')
def build_chroma():
    source=e/'sources/chroma';build=e/'build/chroma';build.mkdir()
    run('chroma-bootstrap',['autoreconf','-fi'],source,'chroma')
    run('chroma-configure',[source/'configure','--prefix='+str(e/'install/chroma'),'--with-qdp='+str(root/'evidence/tools-filedb-20261109-001/install/qdpxx-mpi'),'--with-superbblas='+str(reference/'install/superbblas-mpi'),'--with-datalib='+str(e/'install/datalib-filedb'),'--with-primme='+str(reference/'sources/primme'),'--enable-mgproton','--disable-cpp-wilson-dslash','--enable-testcase-runner=trivial','--with-libxml2=/opt/homebrew/opt/libxml2','--with-gmp=/opt/homebrew/opt/gmp'],build,'chroma')
    run('chroma-build',['make','-j3'],build,'chroma');run('chroma-install',['make','install'],build,'chroma')
    (e/'chroma-build.exit').write_text('0\n')
def build_downstream():
    for name in ('adat','colorvec','redstar'):
        source=e/'sources'/name;build=e/'build'/name;build.mkdir()
        run(name+'-bootstrap',['autoreconf','-fi'],source,'downstream')
        options={'adat':['--with-libxml2=/opt/homebrew/opt/libxml2','--with-datalib='+str(e/'install/datalib-filedb'),'--with-superbblas='+str(reference/'install/superbblas')],'colorvec':['--with-adat='+str(e/'install/adat'),'--with-hadron='+str(reference/'install/hadron')],'redstar':['--with-colorvec='+str(e/'install/colorvec')]}[name]
        run(name+'-configure',[source/'configure','--prefix='+str(e/'install'/name),*options],build,'downstream')
        for step,args in [('build',['make','-j3']),('check',['make','check','-j3']),('install',['make','install'])]:run(name+'-'+step,args,build,'downstream')
    (e/'downstream-build.exit').write_text('0\n')
with ThreadPoolExecutor(max_workers=2) as pool:
    futures=[pool.submit(build_chroma),pool.submit(build_downstream)]
    for future in futures:future.result()
(e/'build.exit').write_text('0\n')
print('FILEDB/tools datalib and complete fresh downstream stack built.',flush=True)
