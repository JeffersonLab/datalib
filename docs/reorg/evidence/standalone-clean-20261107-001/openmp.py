import json
from pathlib import Path
import shlex
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
env=json.loads((e/'environments.json').read_text())['datalib']
source=e/'openmp.cc'
source.write_text('#include <omp.h>\n#include <iostream>\nint main(){int threads=0;\n#pragma omp parallel\n{\n#pragma omp single\nthreads=omp_get_num_threads();}\nstd::cout<<threads<<"\\n";}\n')
config=e/'install/datalib-core/bin/datalib-config'
def flags(option):return shlex.split(subprocess.check_output([str(config),'--'+option],env=env,text=True))
args=['/usr/bin/clang++',*flags('cxxflags'),str(source),'-o',str(e/'openmp'),*flags('ldflags'),*flags('libs')]
with (e/'openmp-build.log').open('w') as log:
    subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*args],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
rows=[]
for threads in (1,4):
    observed=int(subprocess.check_output(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),str(e/'openmp')],env={**env,'OMP_NUM_THREADS':str(threads)},text=True))
    assert observed==threads
    rows.append({'requested':threads,'observed':observed})
(e/'openmp.json').write_text(json.dumps(rows,indent=2)+'\n')
print('OpenMP teams of 1 and 4 verified with fresh installed datalib flags.')
