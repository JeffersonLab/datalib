from pathlib import Path
import subprocess,shlex,json,hashlib,math
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/dbtools-extract-20261109-001';new=e/'install/datalib-filedb';old=r/'evidence/chroma-storage-20261109-001/install/adat/bin';work=e/'tool-regressions-004';work.mkdir();cfg=new/'bin/datalib-config'
def flags(opt):return shlex.split(subprocess.check_output([cfg,'--'+opt],text=True))
commands=[]
def run(args,success=True):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=120,cwd=work)
 idx=len(commands);commands.append({'command':args,'returncode':p.returncode})
 (work/f'command-{idx:03}.log').write_text(p.stdout+p.stderr)
 (work/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert (p.returncode==0)==success,(args,p.returncode,(p.stdout+p.stderr)[-3500:])
 return p.stdout
cmd=['/usr/bin/sandbox-exec','-f',e/'datalib.sb','/usr/bin/clang++',*flags('cxxflags'),*flags('storage-cxxflags'),'/tmp/dbtools-extract/tools_fixture.cc','-o',e/'standalone-tools-fixture',*flags('ldflags'),*flags('storage-ldflags'),*flags('libs'),*flags('storage-libs')]
run(cmd)
run([e/'standalone-tools-fixture','create',work/'standalone-inputs',r/'datalib/tests/fixtures/dbtools'])
fixture=e/'standalone-tools-fixture'
def dump(path):
 text=run([fixture,'dump',path]);return json.loads(next(line[7:] for line in text.splitlines() if line.startswith('RESULT:')))
for name in ('legacy-sun','su2-populated','converted'):
 for ext in ('.bin','.xml'):
  assert (e/'baseline-inputs'/(name+ext)).read_bytes()==(work/'standalone-inputs'/(name+ext)).read_bytes(),name+ext
inputs=e/'baseline-inputs';hashes={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs.glob('*')}
comparisons=[]
def compare(label,baseline,feature):
 a,b=dump(baseline),dump(feature)
 assert a['metadata']==b['metadata'],label+' metadata'
 assert a['configurations']==b['configurations'],label+' configurations'
 assert a['records'].keys()==b['records'].keys(),label+' keys'
 maxerr=0.;count=0
 scale=max(abs(complex(*z)) for rows in a['records'].values() for row in rows for z in row)
 for key,rows in a['records'].items():
  assert len(rows)==len(b['records'][key])
  for x,y in zip(rows,b['records'][key]):
   assert len(x)==len(y)
   for za,zb in zip(x,y):
    za,zb=complex(*za),complex(*zb);assert math.isfinite(zb.real) and math.isfinite(zb.imag)
    err=abs(za-zb);assert err<=1e-9*abs(za)+1e-12*max(scale,1e-30),(label,za,zb,err)
    maxerr=max(maxerr,err);count+=1
 comparisons.append({'case':label,'keys':len(a['records']),'values':count,'max_absolute_error':maxerr,'metadata_and_encoded_keys_equal':True})
for name in ('one','two','three','su2','sun'):
 source=inputs/(name+'.sdb')
 for tool,extra in [('dbavgcors',[source,source]),('dbbin',[2])]:
  for lane,binpath in [('baseline',old/tool if tool!='dbbin' else r/'evidence/dbbin-mozilla-20261109-001/bin/dbbin'),('feature',new/'bin'/tool)]:
   run([binpath,work/f'{tool}-{name}-{lane}.sdb',source,*extra])
  compare(tool+'-'+name,work/f'{tool}-{name}-baseline.sdb',work/f'{tool}-{name}-feature.sdb')
for tool in ('dbwick_rescale','dbiso_wick_rescale','dbcanon_mom','dbconvert_corr_util'):
 for lane,prefix in [('baseline',old),('feature',new/'bin')]:run([e/'baseline-dbconvert-fixed' if tool=='dbconvert_corr_util' and lane=='baseline' else prefix/tool,work/f'{tool}-{lane}.sdb',inputs/'su2.sdb'])
 compare(tool,work/f'{tool}-baseline.sdb',work/f'{tool}-feature.sdb')
for tool in ('db_transform_corrs',):
 for lane,prefix in [('baseline',old),('feature',new/'bin')]:run([prefix/tool,work/f'{tool}-{lane}.sdb',inputs/'sun.sdb'])
 compare(tool,work/f'{tool}-baseline.sdb',work/f'{tool}-feature.sdb')
# Fixture hashes also protect immutable expected codecs/data against accidental writes.
assert all(hashlib.sha256(Path(p).read_bytes()).hexdigest()==sha for p,sha in hashes.items())
(work/'results.json').write_text(json.dumps({'passed':True,'populated_binary_xml_codecs_identical':True,'input_hashes_preserved':True,'comparisons':comparisons,'baseline_db_bin':'Corrected ENSEM dbbin from preceding checkpoint'},indent=2)+'\n')
print(json.dumps(comparisons,indent=2))
