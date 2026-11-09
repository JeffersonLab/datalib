from pathlib import Path
import subprocess,json,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';config=e/'install/chroma/bin/chroma-config';env=json.loads((e/'environments.json').read_text())['chroma'];env['OMP_NUM_THREADS']='1'
def flags(n):return shlex.split(subprocess.check_output([str(config),'--'+n],text=True))
cmd=['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),'/opt/homebrew/bin/mpicxx',*flags('cxxflags'),str(e/'codec_probe.cc'),'-o',str(e/'codec-clean-probe'),*flags('ldflags'),*flags('libs')]
with (e/'codec-clean-build.log').open('w') as f:subprocess.run(cmd,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
results=[]
for n in (1,2):
 out=e/('codec-clean-'+str(n)+'-ranks');out.mkdir();cmd2=['/opt/homebrew/bin/mpirun','-n',str(n),'/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),str(e/'codec-clean-probe'),str(e/'frozen'),str(out)]
 with (e/('codec-clean-'+str(n)+'-ranks.log')).open('w') as f:p=subprocess.run(cmd2,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=90)
 results.append({'ranks':n,'command':cmd2,'returncode':p.returncode});(e/'codec-clean.json').write_text(json.dumps({'status':'passed' if all(x['returncode']==0 for x in results) else 'failed','compile_command':cmd,'runs':results,'uses_only_exported_chroma_flags':True},indent=2)+'\n');p.check_returncode()
print('Clean installed Chroma API matches frozen records at 1/2 ranks, ADAT access blocked')
