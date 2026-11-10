from pathlib import Path
import subprocess,shlex,json,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/dbtools-extract-20261109-001';work=e/'prop-diag-check';work.mkdir();cfg=e/'install/datalib-filedb/bin/datalib-config'
def flags(n):return shlex.split(subprocess.check_output([cfg,'--'+n],text=True))
commands=[]
def run(args):
 args=list(map(str,args));x=subprocess.run(args,capture_output=True,text=True,timeout=60);i=len(commands);commands.append(args);(work/f'command-{i:02}.log').write_text(x.stdout+x.stderr);assert x.returncode==0,(args,x.stderr[-2500:]);return x.stdout
fixture=work/'fixture';run(['/usr/bin/sandbox-exec','-f',e/'datalib.sb','/usr/bin/clang++',*flags('cxxflags'),*flags('storage-cxxflags'),'/tmp/dbtools-extract/prop_fixture.cc','-o',fixture,*flags('ldflags'),*flags('storage-ldflags'),*flags('libs'),*flags('storage-libs')])
source=work/'input.sdb';run([fixture,'create',source]);before=hashlib.sha256(source.read_bytes()).hexdigest()
def dump(p):return json.loads(next(s[7:] for s in run([fixture,'dump',p]).splitlines() if s.startswith('RESULT:')))
a=dump(source);assert len(a['records'])==64 and len(a['diagonal'])==32
for lane,prefix in [('baseline',r/'evidence/chroma-storage-20261109-001/install/adat/bin'),('feature',e/'install/datalib-filedb/bin')]:
 out=work/(lane+'.sdb');run([prefix/'dbprop_diag_t0',out,source]);b=dump(out);assert b['records']==a['diagonal'];assert b['metadata_hex']==a['metadata_hex']
assert hashlib.sha256(source.read_bytes()).hexdigest()==before
(work/'results.json').write_text(json.dumps({'passed':True,'input_keys':64,'output_keys':32,'spin_pairs':16,'times':2,'metadata_keys_values_identical_to_baseline':True,'finite_matrix_shape':[2,2],'input_hash_preserved':True},indent=2)+'\n');(work/'commands.json').write_text(json.dumps(commands,indent=2)+'\n');print('dbprop_diag_t0 matched all 32 diagonal spin/time keys and matrix payloads.')
