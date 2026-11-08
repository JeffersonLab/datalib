import json
from pathlib import Path
import shlex
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
env=json.loads((e/'environment.json').read_text())
source=e/'openmp.cc'
source.write_bytes((root/'evidence/standalone-clean-20261107-001/openmp.cc').read_bytes())
config=e/'install/redstar/bin/redstar-config'
def flags(option):return shlex.split(subprocess.check_output([str(config),'--'+option],env=env,text=True))
args=['/usr/bin/clang++',*flags('cxxflags'),str(source),'-o',str(e/'openmp'),*flags('ldflags'),*flags('libs')]
with (e/'openmp-build.log').open('w') as log:subprocess.run(args,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
rows=[]
for count in (1,4):
    actual=int(subprocess.check_output([str(e/'openmp')],env={**env,'OMP_NUM_THREADS':str(count),'OMP_DYNAMIC':'FALSE'},text=True))
    assert actual==count
    rows.append({'requested':count,'observed':actual})
(e/'openmp-check.json').write_text(json.dumps(rows,indent=2)+'\n')
for name,artifact in [('redstar',e/'install/redstar/bin/redstar_npt'),('chroma',e/'install/chroma/bin/chroma')]:
    output=subprocess.check_output(['/usr/bin/otool','-L',str(artifact)],text=True)
    (e/(name+'-dynamic-libraries.log')).write_text(output)
    assert 'libomp' in output and 'libgomp' not in output and 'libstdc++' not in output
print('Fresh Redstar flags produced OpenMP teams of one and four; both evaluators link libomp.')
