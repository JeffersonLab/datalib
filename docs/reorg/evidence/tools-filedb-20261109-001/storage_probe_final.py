from pathlib import Path
import subprocess,shlex,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';cfg=e/'install/datalib-filedb/bin/datalib-config'
def flags(name):return shlex.split(subprocess.check_output([str(cfg),'--'+name],text=True))
source=e/'storage_probe_final.cc';source.write_bytes((Path(__file__).parent/'storage_probe.cc').read_bytes());exe=e/'storage-probe-final'
cmd=['/opt/homebrew/bin/mpicxx',*flags('cxxflags'),*flags('filedb-cxxflags'),str(source),'-o',str(exe),*flags('ldflags'),*flags('filedb-ldflags'),*flags('libs'),*flags('filedb-libs')]
with (e/'storage-probe-final-build.log').open('w') as out:subprocess.run(cmd,stdout=out,stderr=subprocess.STDOUT,check=True)
results=[];env=json.loads((e/'environments.json').read_text())['chroma'];env['OMP_NUM_THREADS']='1'
for n in (1,2):
 out=e/f'storage-probe-final-{n}-ranks';out.mkdir();args=['/opt/homebrew/bin/mpirun','-n',str(n),str(exe),str(r.parent/'redstar_kanoni.devel/data/meson.sdb'),str(out)]
 with (e/f'storage-probe-final-{n}-ranks.log').open('w') as log:p=subprocess.run(args,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=60)
 results.append({'ranks':n,'command':args,'returncode':p.returncode});p.check_returncode()
(e/'eloy-filedb-final-check.json').write_text(json.dumps({'status':'passed','compile_command':cmd,'runs':results,'scope':'Eloy StorageMeson FILEDB backend on real read-only data; standalone legacy correlator read/write, independent per-rank files. No QDP/ADAT APIs or includes.'},indent=2)+'\n')
print('Eloy FILEDB storage passed at one and two ranks without QDP or ADAT.')
