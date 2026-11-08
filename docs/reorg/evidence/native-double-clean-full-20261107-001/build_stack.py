from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/native-double-clean-full-20261107-001'
e.mkdir()
for name in ('sources','build','install'):(e/name).mkdir()
clean=root/'evidence/standalone-clean-20261107-001'
datalib=clean/'install/datalib-filedb'
chroma=clean/'install/chroma/bin/chroma'
assert json.loads((clean/'verification.json').read_text())['status']=='passed'
shutil.copy2(Path(__file__).parent/'push.json',e/'datalib-documentation-push.json')

def git(repo,*args):
    return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)
protected={}
for name in ('adat.devel','chroma.devel','redstar.devel','colorvec.devel','redstar_kanoni.devel'):
    repo=root.parent/name
    protected[name]={'devel':git(repo,'rev-parse','refs/heads/devel')}
    if name=='redstar_kanoni.devel':
        protected[name]['status']=git(repo,'status','--porcelain=v1','--branch')
        protected[name]['diff']=hashlib.sha256(git(repo,'diff','--binary','HEAD').encode()).hexdigest()
(e/'protected-before.json').write_text(json.dumps(protected,indent=2)+'\n')
reorg={}
for name in ('datalib','adat','colorvec','redstar','chroma'):
    repo=root/name
    assert git(repo,'branch','--show-current').strip()=='rge/devel-datalib-reorg'
    reorg[name]={'head':git(repo,'rev-parse','HEAD').strip(),'status':git(repo,'status','--short'),
                 'local_devel':git(repo,'for-each-ref','--format=%(objectname)','refs/heads/devel')}
(e/'reorg-before.json').write_text(json.dumps(reorg,indent=2)+'\n')
# The newly pushed datalib commit contains documentation only; the installed
# archive is from the clean source checkout of the same code.
subprocess.run(['git','-C',str(root/'datalib'),'diff','--exit-code',git(clean/'sources/datalib','rev-parse','HEAD').strip(),'HEAD','--','src','include','tests','configure.ac','datalib-config.in'],check=True)
env=json.loads((root/'evidence/adat-direct-20261107-001/build.json').read_text())['environment']
(e/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
records=[]
def run(label,args,directory):
    args=list(map(str,args))
    print(label,flush=True)
    start=time.monotonic()
    with (e/(label+'.log')).open('w') as log:
        status=subprocess.run(args,cwd=directory,env=env,stdout=log,stderr=subprocess.STDOUT)
    records.append({'step':label,'command':args,'cwd':str(directory),'returncode':status.returncode,
                    'seconds':time.monotonic()-start,'finished_utc':datetime.now(timezone.utc).isoformat()})
    (e/'build.json').write_text(json.dumps(records,indent=2)+'\n')
    status.check_returncode()

reference=root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002'
for name in ('adat','colorvec','redstar'):
    source=e/'sources'/name
    run(name+'-clone',['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',root/name,source],e)
    assert git(source,'rev-parse','HEAD').strip()==reorg[name]['head']
    assert not git(source,'status','--porcelain=v1')
    (e/(name+'-clean-checkout.json')).write_text(json.dumps({'head':reorg[name]['head'],'branch':'rge/devel-datalib-reorg','status_before_bootstrap':''},indent=2)+'\n')
    if (source/'.gitmodules').is_file():
        for line in git(source,'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$').splitlines():
            key,path=line.split(None,1)
            subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(root/name/path)],check=True)
        run(name+'-submodules',['git','-c','protocol.file.allow=always','submodule','update','--init','--recursive'],source)
        assert not git(source,'status','--porcelain=v1')
        (e/(name+'-submodule-commits.log')).write_text(git(source,'submodule','status','--recursive'))
    run(name+'-bootstrap',['autoreconf','-fi'],source)
    build=e/'build'/name
    build.mkdir()
    prefix=e/'install'/name
    assert not prefix.exists()
    options={
      'adat':['--with-libxml2=/opt/homebrew/opt/libxml2','--with-datalib='+str(datalib),'--with-superbblas='+str(reference/'install/superbblas')],
      'colorvec':['--with-adat='+str(e/'install/adat'),'--with-hadron='+str(reference/'install/hadron')],
      'redstar':['--with-colorvec='+str(e/'install/colorvec')],
    }[name]
    run(name+'-configure',[source/'configure','--prefix='+str(prefix),*options],build)
    run(name+'-build',['make','-j6'],build)
    run(name+'-check',['make','check','-j6'],build)
    run(name+'-install',['make','install'],build)

sources={}
for name in ('datalib','adat','colorvec','redstar','chroma'):
    source=(clean/'sources'/name) if name in ('datalib','chroma') else (e/'sources'/name)
    sources[name]={'commit':git(source,'rev-parse','HEAD').strip(),'source':str(source),
                   'branch_head':reorg[name]['head']}
(e/'stack-sources.json').write_text(json.dumps(sources,indent=2)+'\n')
(e/'reorg-sources.log').write_text(''.join(name+' '+item['commit']+'\n' for name,item in sources.items()))
(e/'build.exit').write_text('0\n')
print('Fresh downstream builds completed against the clean datalib installation.',flush=True)
