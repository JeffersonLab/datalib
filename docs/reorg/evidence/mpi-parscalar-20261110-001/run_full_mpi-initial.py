import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
root=Path.home()/'Downloads/macos-datalib-reorg'
kanoni=root.parent/'redstar_kanoni.devel'
e=root/'evidence/mpi-parscalar-20261110-001'
stack=root/'evidence/dbtools-extract-20261109-001'
clean=e
env=json.loads((e/'environment.json').read_text())
env['PYTHONDONTWRITEBYTECODE']='1'
env.pop('NPT_MEMORY_AUDIT',None)
adat=stack/'install/adat'
redstar=stack/'install/redstar/bin'
graph_tools=e/'graph-tools'
graph_tools.mkdir()
for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib/bin/view_corr_graph')]:
    wrapper=graph_tools/tool
    wrapper.write_text('#!/bin/sh\nexec '+shlex.quote(str(path))+' "$@"\n')
    wrapper.chmod(0o755)
(e/'graph-executors.json').write_text(json.dumps([{'wrapper':str(graph_tools/tool),'wrapper_sha256':hashlib.sha256((graph_tools/tool).read_bytes()).hexdigest(),'executable':str(path),'executable_sha256':hashlib.sha256(path.read_bytes()).hexdigest()} for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib/bin/view_corr_graph')]],indent=2)+'\n')
real_chroma=clean/'install/chroma/bin/chroma' 
chroma=e/'chroma-without-adat'
profile=clean/'chroma.sb'
chroma.write_text('#!/bin/sh\nexec '+shlex.join(['/opt/homebrew/bin/mpirun','--bind-to','none','-n','2','/usr/bin/python3',str(e/'rank_guard.py'),'/usr/bin/sandbox-exec','-f',str(profile),str(real_chroma)])+' "$@"\n')
chroma.chmod(0o755)
(e/'chroma-executor.json').write_text(json.dumps({'wrapper':str(chroma),'executable':str(real_chroma),'profile':str(profile),'executable_sha256':hashlib.sha256(real_chroma.read_bytes()).hexdigest()},indent=2)+'\n')
source=e/'export_correlators.cc'
config=adat/'bin/adat-config'
def flags(name):return shlex.split(subprocess.check_output([str(config),'--'+name],text=True,env=env))
exporter=e/'export-correlators'
cmd=['/usr/bin/clang++',*flags('cxxflags'),str(source),'-o',str(exporter),*flags('ldflags'),*flags('libs')]
(e/'exporter-command.json').write_text(json.dumps(cmd,indent=2)+'\n')
with (e/'exporter-build.log').open('w') as log:subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
def git(name,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(root.parent/name),*args],text=True)
state=git('redstar_kanoni.devel','status','--porcelain=v1','--branch')
for name in ('redstar.devel','adat.devel','chroma.devel','colorvec.devel','redstar_kanoni.devel'):
    state+=name+' '+git(name,'rev-parse','refs/heads/devel')
(e/'devel-before.log').write_text(state)
paths=[kanoni/'docs/regression-policy.json',kanoni/'scripts/compare_regression_chains.py',
       clean/'install/datalib-filedb/lib/libdatalib_serial.a',real_chroma,profile,
       stack/'install/adat/lib/libadat.a',stack/'install/colorvec/lib/libcolorvec.a',stack/'install/redstar/lib/libredstar.a']
(e/'policy-runner-library.sha256').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+str(p)+'\n' for p in paths))
statuses={}
for lane,extra in [('paired',[]),('npt-only',['--npt-only','--cases','fsq_3pt','glue','tetra_F27_S2'])]:
    folder=e/lane
    folder.mkdir()
    args=[sys.executable,kanoni/'scripts/compare_regression_chains.py',
          '--dataset',kanoni/'runs/native-superb-rsd14-20261101-001','--output',folder/'suite',
          '--disco-dataset',kanoni/'data/disco-native-direct-20261101-001',
          '--baseline-graph-bin',graph_tools,'--feature-graph-bin',graph_tools,
          '--baseline-npt',redstar/'redstar_npt','--feature-npt',redstar/'redstar_npt',
          '--baseline-chroma',chroma,'--feature-chroma',chroma,
          '--dbutil',e/'install/datalib/bin/dbutil','--exporter',exporter,'--chain','no_1pt',
          '--qualification','Fresh MPI QMP, parscalar double QDP/OpenMP, serial and MPI SuperbBLAS, standalone datalib and ADAT-free Chroma from matching reorg branches. Chroma executes with two MPI ranks and two OpenMP threads per rank, explicitly set by a per-rank executor; the unchanged runner sets the single-process NPT reference to one OpenMP worker. Each real Chroma rank is independently RSS-monitored, not just mpirun. Cached native-double tensors and explicit DISCO supplement; zero CG solves; original deferrals, 8192 MiB RSS cap per actual rank process, 2 GiB planner and SB_CACHEGB_CPU=1 unchanged. Legacy QDP SIMD unit-program compile failure is preserved separately and does not establish an MPI runtime failure.',*extra]
    args=list(map(str,args))
    (folder/'command.json').write_text(json.dumps(args,indent=2)+'\n')
    print('Running '+lane,flush=True)
    with (folder/'suite.log').open('w') as log:
        status=subprocess.run(args,cwd=kanoni,env=env,stdout=log,stderr=subprocess.STDOUT)
    statuses[lane]=status.returncode
    (e/(lane+'.exit')).write_text(str(status.returncode)+'\n')
    print(lane,status.returncode,flush=True)
assert all(value==0 for value in statuses.values()),statuses
(e/'regression.exit').write_text('0\n')
