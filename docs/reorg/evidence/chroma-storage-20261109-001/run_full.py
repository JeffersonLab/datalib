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
e=root/'evidence/chroma-storage-20261109-001'
stack=e
clean=e
env=json.loads((e/'environment.json').read_text())
env['PYTHONDONTWRITEBYTECODE']='1'
env.pop('NPT_MEMORY_AUDIT',None)
adat=stack/'install/adat'
redstar=stack/'install/redstar/bin'
graph_tools=e/'graph-tools'
graph_tools.mkdir()
for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib-filedb/bin/view_corr_graph')]:
    wrapper=graph_tools/tool
    wrapper.write_text('#!/bin/sh\nexec '+shlex.quote(str(path))+' "$@"\n')
    wrapper.chmod(0o755)
(e/'graph-executors.json').write_text(json.dumps([{'wrapper':str(graph_tools/tool),'wrapper_sha256':hashlib.sha256((graph_tools/tool).read_bytes()).hexdigest(),'executable':str(path),'executable_sha256':hashlib.sha256(path.read_bytes()).hexdigest()} for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib-filedb/bin/view_corr_graph')]],indent=2)+'\n')
real_chroma=clean/'install/chroma/bin/chroma'
chroma=e/'chroma-without-adat'
profile=clean/'chroma.sb'
chroma.write_text('#!/bin/sh\nexec '+shlex.join(['/usr/bin/sandbox-exec','-f',str(profile),str(real_chroma)])+' "$@"\n')
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
          '--dbutil',e/'install/datalib-filedb/bin/dbutil','--exporter',exporter,'--chain','no_1pt',
          '--qualification','SB_CACHEGB_CPU=1, unchanged 8192 MiB RSS cap and 2 GiB planner with the established allocation-cache cap. Standalone datalib lib layout, no QDP options; legacy Chroma producers use datalib tensor storage directly; no Chroma FILEDB classes or temporary wrappers; matched tensor-storage ABI; STOCH_HADRON removed, with ADAT-free Chroma, with fresh ADAT/ColorVec/Redstar rebuilt against that same datalib install; Apple Clang/libomp; cached native-double tensors, explicit DISCO supplement; one MPI rank; Chroma runs through an ADAT-denying sandbox executor whose real executable hash is recorded separately.',*extra]
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
