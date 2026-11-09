from pathlib import Path
import subprocess,json,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';old=r/'evidence/tools-filedb-20261109-001'
source=e/'codec_probe.cc';source.write_bytes((Path(__file__).parent/'codec_probe.cc').read_bytes());args=json.loads((e/'freeze-command.json').read_text());args[1:1]=['-I'+str(r/'chroma/lib'),'-I'+str(r/'datalib/include')];args=[str(source) if x==str(e/'freeze.cc') else str(e/'codec-probe') if x==str(e/'freeze') else x for x in args];env=json.loads((old/'environments.json').read_text())['chroma'];env['OMP_NUM_THREADS']='1'
with (e/'codec-probe-build.log').open('w') as f:subprocess.run(args,env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
results=[]
for n in (1,2):
 out=e/('codec-probe-final-'+str(n)+'-ranks');out.mkdir();cmd=['/opt/homebrew/bin/mpirun','-n',str(n),str(e/'codec-probe'),str(e/'frozen'),str(out)]
 with (e/('codec-probe-final-'+str(n)+'-ranks.log')).open('w') as f:p=subprocess.run(cmd,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=90)
 results.append({'ranks':n,'command':cmd,'returncode':p.returncode});(e/'codec-probe.json').write_text(json.dumps({'compile_command':args,'runs':results},indent=2)+'\n');p.check_returncode()
import struct,hashlib
layout=[]
for n in (1,2):
 for file in (e/('codec-probe-final-'+str(n)+'-ranks')).glob('*.sdb'):
  before=e/'frozen'/('local.sdb' if file.name.startswith('local-') else 'collective.sdb')
  a=before.read_bytes();b=file.read_bytes();assert len(a)==len(b)
  used=struct.unpack_from('<I',a,8192+28)[0];assert struct.unpack_from('<I',b,8192+28)[0]==used
  aa=bytearray(a);bb=bytearray(b)
  for x in (aa,bb):x[8192+20:8192+24]=b'\0'*4;x[8192+32+used:16384]=b'\0'*(8192-32-used)
  assert aa==bb,file
  layout.append({'file':str(file),'same_wire_bytes_except_unused_metadata_page_tail_and_its_crc':True,'metadata_bytes':used,'size':len(a),'sha256':hashlib.sha256(b).hexdigest()})
(e/'codec-layout.json').write_text(json.dumps(layout,indent=2)+'\n')
print('old/new record and wire-layout bytes matched at 1 and 2 ranks (unused metadata padding and CRC excluded)')
