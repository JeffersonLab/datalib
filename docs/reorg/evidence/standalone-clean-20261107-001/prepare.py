import hashlib
import json
import os
from pathlib import Path
import subprocess

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
e.mkdir()
for folder in ('sources','build','install'):
    (e/folder).mkdir()

def git(repo,*args):
    return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)

snapshot={}
for name in ('adat.devel','redstar.devel','chroma.devel','colorvec.devel','redstar_kanoni.devel'):
    repo=root.parent/name
    snapshot[name]={'devel':git(repo,'rev-parse','refs/heads/devel')}
    if name=='redstar_kanoni.devel':
        snapshot[name]['status']=git(repo,'status','--porcelain=v1','--branch')
        snapshot[name]['diff_sha256']=hashlib.sha256(git(repo,'diff','--binary','HEAD').encode()).hexdigest()
for name in ('datalib','adat','colorvec','redstar','chroma'):
    repo=root/name
    assert git(repo,'branch','--show-current').strip()=='rge/devel-datalib-reorg'
    snapshot['reorg/'+name]={'head':git(repo,'rev-parse','HEAD'),'status':git(repo,'status','--short'),
                            'local_devel':git(repo,'for-each-ref','--format=%(objectname)','refs/heads/devel')}
(e/'protected-before.json').write_text(json.dumps(snapshot,indent=2)+'\n')

for name in ('datalib','chroma'):
    with (e/(name+'-clone.log')).open('w') as log:
        subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(root/name),str(e/'sources'/name)],stdout=log,stderr=subprocess.STDOUT,check=True)
    source=e/'sources'/name
    assert git(source,'rev-parse','HEAD')==snapshot['reorg/'+name]['head']
    assert not git(source,'status','--porcelain=v1')
    # Resolve only submodule URLs in this verification clone, using the existing
    # repositories as object stores and checking out the exact recorded gitlinks.
    if name=='chroma':
        paths=git(source,'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$').splitlines()
        for line in paths:
            key,path=line.split(None,1)
            subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(root/name/path)],check=True)
        with (e/'chroma-submodules.log').open('w') as log:
            subprocess.run(['git','-C',str(source),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],stdout=log,stderr=subprocess.STDOUT,check=True)
        assert not git(source,'status','--porcelain=v1')
        (e/'chroma-submodule-commits.log').write_text(git(source,'submodule','status','--recursive'))
    (e/(name+'-clean-checkout.json')).write_text(json.dumps({'head':git(source,'rev-parse','HEAD').strip(),'branch':git(source,'branch','--show-current').strip(),'status_before_bootstrap':git(source,'status','--porcelain=v1'),'tracked_files':len(git(source,'ls-files').splitlines())},indent=2)+'\n')

adat=r'.*/(adat[^/]*|libadat[^/]*)(/|$)'
qdp=r'.*/(qdp[^/]*|qmp[^/]*|libqdp[^/]*|libqmp[^/]*)(/|$)'
for label,expressions in (('datalib',[adat,qdp]),('chroma',[adat])):
    (e/(label+'.sb')).write_text('(version 1)\n(allow default)\n'+''.join('(deny file-read* (regex "'+x+'"))\n' for x in expressions))

sdk='/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX26.5.sdk'
base={'PATH':'/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin','HOME':str(Path.home()),
      'TMPDIR':os.environ['TMPDIR'],'CC':'/usr/bin/clang','CXX':'/usr/bin/clang++',
      'CFLAGS':'-O2 -isysroot '+sdk,'CXXFLAGS':'-O1 -std=c++20 -isysroot '+sdk,
      'CPPFLAGS':'-isysroot '+sdk,'LDFLAGS':'-isysroot '+sdk,'LIBS':'',
      'OPENMP_CXXFLAGS':'-Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include',
      'OPENMP_LIBS':'-L/opt/homebrew/opt/libomp/lib -Wl,-rpath,/opt/homebrew/opt/libomp/lib -lomp',
      'OMP_NUM_THREADS':'4','OMP_DYNAMIC':'FALSE','OMP_MAX_ACTIVE_LEVELS':'1',
      'VECLIB_MAXIMUM_THREADS':'1','OPENBLAS_NUM_THREADS':'1','PYTHONDONTWRITEBYTECODE':'1',
      'CONFIG_SHELL':'/bin/bash'}
chroma_env=json.loads((root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/mpi-build.json').read_text())['environment']
chroma_env['CXXFLAGS']=chroma_env['CXXFLAGS'].replace(' -DUSE_MPI=1','')
(e/'environments.json').write_text(json.dumps({'datalib':base,'chroma':chroma_env},indent=2)+'\n')

adat_file=root/'install/adat-direct-001/include/io/superb_kv.h'
qdp_file=root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/install/qdpxx-mpi/include/qdp.h'
canaries=[]
for label,filename,allowed in [('datalib',adat_file,False),('datalib',qdp_file,False),('chroma',adat_file,False),('chroma',qdp_file,True)]:
    assert filename.is_file()
    result=subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/(label+'.sb')),'/bin/cat',str(filename)],stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True,env=base)
    assert (result.returncode==0)==allowed,(label,filename,result.stderr)
    canaries.append({'profile':label,'path':str(filename),'allowed':allowed,'returncode':result.returncode,'stderr':result.stderr})
(e/'sandbox-canaries.json').write_text(json.dumps(canaries,indent=2)+'\n')
print(e)
print('Clean committed checkouts prepared; filesystem denial canaries passed.')
