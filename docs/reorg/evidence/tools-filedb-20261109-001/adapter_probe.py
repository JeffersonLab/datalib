from pathlib import Path
import subprocess,shlex,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001'
q=e/'install/qdpxx-mpi/bin/qdp++-config';d=e/'install/datalib-filedb/bin/datalib-config'
def flags(c,name):return shlex.split(subprocess.check_output([str(c),'--'+name],text=True))
source=e/'adapter_probe.cc';source.write_bytes((Path(__file__).parent/'adapter_probe.cc').read_bytes())
cmd=['/opt/homebrew/bin/mpicxx','-I'+str(e/'sources/chroma/lib'),'-I'+str(e/'build/chroma/lib'),*flags(q,'cxxflags'),*flags(d,'filedb-cxxflags'),str(source),'-o',str(e/'adapter-probe'),*flags(q,'ldflags'),*flags(d,'filedb-ldflags'),*flags(d,'filedb-libs'),*flags(q,'libs')]
with (e/'adapter-build.log').open('w') as out:subprocess.run(cmd,stdout=out,stderr=subprocess.STDOUT,check=True)
records=[]
env=json.loads((e/'environments.json').read_text())['chroma'];env['OMP_NUM_THREADS']='1'
for ranks in (1,2):
 folder=e/f'adapter-{ranks}-ranks';folder.mkdir();args=['/opt/homebrew/bin/mpirun','-n',str(ranks),str(e/'adapter-probe'),str(folder)]
 with (e/f'adapter-{ranks}-ranks.log').open('w') as out:p=subprocess.run(args,env=env,stdout=out,stderr=subprocess.STDOUT,timeout=60)
 records.append({'ranks':ranks,'command':args,'returncode':p.returncode});p.check_returncode()
(e/'adapter-check.json').write_text(json.dumps({'status':'passed','compile_command':cmd,'runs':records,'limitation':'The upstream collective get/keys paths remain single-rank only. Two-rank coverage is local per-rank read/write and collective insertion/metadata broadcast.'},indent=2)+'\n')
print('Relocated adapter passed one- and two-rank targeted checks.')
