from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,json,shutil,threading,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';e.mkdir();(e/'build').mkdir();(e/'sources').mkdir();reference=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002';prior=r/'evidence/dbtools-extract-20261109-001'
envs=json.loads((prior/'environments.json').read_text());mpi=envs['chroma'];mpi['OMP_NUM_THREADS']='2';mpi['SB_CACHEGB_CPU']='1';serial=envs['datalib'];serial['OMP_NUM_THREADS']='2';envs['mpi']=mpi;envs['serial']=serial
(e/'environments.json').write_text(json.dumps(envs,indent=2)+'\n')
for file in ('datalib.sb','chroma.sb'):shutil.copy2(prior/file,e/file)
records=[];lock=threading.Lock()
def run(label,args,cwd,kind='mpi',sandbox=None):
 args=list(map(str,args));cmd=(['/usr/bin/sandbox-exec','-f',str(e/(sandbox+'.sb'))]+args) if sandbox else args;start=time.monotonic();print(label,flush=True)
 with (e/(label+'.log')).open('w') as out:p=subprocess.run(cmd,cwd=cwd,env=envs[kind],stdout=out,stderr=subprocess.STDOUT)
 row={'step':label,'command':cmd,'cwd':str(cwd),'environment':kind,'returncode':p.returncode,'seconds':time.monotonic()-start}
 with lock:records.append(row);(e/'build.json').write_text(json.dumps(records,indent=2)+'\n')
 p.check_returncode()
def git(path,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(path),*args],text=True)
# New dependency checkouts use the requested reorg branch without changing archived sources.
for name in ('qmp','superbblas'):
 dest=r/name;assert not dest.exists()
 run(name+'-root-clone',['git','clone','--no-local',reference/'sources'/name,dest],r,'serial')
 subprocess.run(['git','-C',str(dest),'checkout','-b','rge/devel-datalib-reorg'],check=True,stdout=subprocess.DEVNULL)
 origin=git(reference/'sources'/name,'remote','get-url','origin').strip();subprocess.run(['git','-C',str(dest),'remote','set-url','origin',origin],check=True)
sources={}
for name in ('qmp','superbblas','qdpxx','datalib','chroma'):
 source=e/'sources'/name
 run(name+'-clone',['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',r/name,source],r,'serial')
 if (source/'.gitmodules').exists():
  for line in git(source,'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$').splitlines():
   key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(r/name/path)],check=True)
  run(name+'-submodules',['git','-c','protocol.file.allow=always','submodule','update','--init','--recursive'],source,'serial')
 assert not git(source,'status','--porcelain')
 sources[name]={'head':git(source,'rev-parse','HEAD').strip(),'branch':git(source,'branch','--show-current').strip(),'submodules':git(source,'submodule','status','--recursive') if (source/'.gitmodules').exists() else ''}
(e/'sources.json').write_text(json.dumps(sources,indent=2)+'\n')
def autotools(name,opts,kind='mpi',sandbox=None,jobs=3,check=True):
 source=e/'sources'/name;build=e/'build'/name;build.mkdir();prefix=e/'install'/name
 run(name+'-bootstrap',['autoreconf','-fi'],source,kind,sandbox)
 run(name+'-configure',[source/'configure','--prefix='+str(prefix),*opts],build,kind,sandbox)
 run(name+'-build',['make','-j'+str(jobs)],build,kind,sandbox)
 if check:run(name+'-check',['make','check','-j'+str(jobs)],build,kind,sandbox)
 run(name+'-install',['make','install'],build,kind,sandbox)
def build_qdp():
 autotools('qmp',['--with-qmp-comms-type=MPI'])
 autotools('qdpxx',['--enable-parallel-arch=parscalar','--enable-Nd=4','--enable-precision=double','--enable-openmp','--with-qmp='+str(e/'install/qmp'),'--with-libxml2=/opt/homebrew/opt/libxml2'])
 assert subprocess.check_output([e/'install/qmp/bin/qmp-config','--comms'],text=True).strip()=='MPI'
def build_superb():
 for profile,kind in [('superbblas-serial','serial'),('superbblas-mpi','mpi')]:
  build=e/'build'/profile;build.mkdir();flags=envs[kind]['CXXFLAGS']+' -DSUPERBBLAS_USE_CBLAS'
  args=['TARGET=cpu','BUILDDIR='+str(build),'PREFIX='+str(e/'install'/profile),'LIBRARY='+str(build/'libsuperbblas.a'),'CXX='+envs[kind]['CXX'],'CXXFLAGS='+flags,'SUPERBBLAS_WITH_MPI='+('yes' if kind=='mpi' else 'no'),'INSTALL_LINK_SOURCE=no','PYTHON=/opt/homebrew/bin/python3']
  source=e/'sources/superbblas/src';run(profile+'-generate',['make',str(build/'superbblas_lib'),*args],source,kind)
  lines=(build/'superbblas_lib').read_text().splitlines(True);chunks=[]
  for i in range(10):
   part=''.join(lines[len(lines)*i//10:len(lines)*(i+1)//10]);chunks.append(part);(build/f'superbblas_lib_{i}.cpp').write_text('#include "superbblas.h"\nusing namespace superbblas; using namespace superbblas::detail;\n'+part)
  assert ''.join(chunks)==(build/'superbblas_lib').read_text();(build/'superbblas_lib.split').touch()
  run(profile+'-build-install',['make','install','-j2',*args],source,kind)
with ThreadPoolExecutor(max_workers=2) as pool:
 futures=[pool.submit(build_qdp),pool.submit(build_superb)]
 for future in futures:future.result()
autotools('datalib',['--enable-openmp','--with-filedb=bundled','--with-superbblas='+str(e/'install/superbblas-serial'),'--with-libxml2=/opt/homebrew/opt/libxml2'],'serial','datalib',3)
autotools('chroma',['--with-qdp='+str(e/'install/qdpxx'),'--with-superbblas='+str(e/'install/superbblas-mpi'),'--with-datalib='+str(e/'install/datalib'),'--with-primme='+str(reference/'sources/primme'),'--enable-mgproton','--disable-cpp-wilson-dslash','--enable-testcase-runner=trivial','--with-libxml2=/opt/homebrew/opt/libxml2','--with-gmp=/opt/homebrew/opt/gmp'],'mpi','chroma',3,False)
(e/'build.exit').write_text('0\n');print('Fresh MPI QMP, parscalar QDP, serial/MPI SuperbBLAS, standalone datalib and ADAT-free Chroma built.',flush=True)
