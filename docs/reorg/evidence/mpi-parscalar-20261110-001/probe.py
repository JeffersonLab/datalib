from pathlib import Path
import subprocess,json,shlex,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';env=json.loads((e/'environments.json').read_text())['mpi'];rows=[]
source=e/'boundary_probe.cc';shutil.copy2('/tmp/reorg-mpi/boundary_probe.cc',source)
def flags(config,opt):return shlex.split(subprocess.check_output([config,'--'+opt],text=True))
qdp=e/'install/qdpxx/bin/qdp++-config';datalib=e/'install/datalib/bin/datalib-config'
cmd=['/opt/homebrew/bin/mpicxx',*flags(qdp,'cxxflags'),*flags(datalib,'cxxflags'),str(source),'-o',str(e/'boundary-probe'),*flags(qdp,'ldflags'),*flags(datalib,'ldflags'),*flags(qdp,'libs'),*flags(datalib,'libs')]
(e/'boundary-probe-build-command.json').write_text(json.dumps(cmd,indent=2)+'\n')
with (e/'boundary-probe-build.log').open('w') as log:subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
for ranks in (1,2):
 cmd=['/opt/homebrew/bin/mpirun','-n',str(ranks),str(e/'boundary-probe'),str(ranks)]
 p=subprocess.run(cmd,env=env,cwd=e,capture_output=True,text=True,timeout=120);(e/f'boundary-probe-{ranks}-ranks.log').write_text(p.stdout+p.stderr)
 assert p.returncode==0,(cmd,p.stdout,p.stderr)
 result=json.loads(next(line[7:] for line in p.stdout.splitlines() if line.startswith('RESULT:')));rows.append({'command':cmd,'returncode':p.returncode,'result':result})
(e/'boundary-probe.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(rows,indent=2))
