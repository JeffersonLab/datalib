from pathlib import Path
import subprocess,shlex,json,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/dbtools-extract-20261109-001';new=e/'install/datalib-filedb';old=r/'evidence/chroma-storage-20261109-001/install/adat/bin';work=e/'extra-tool-checks-002';work.mkdir()
commands=[]
def run(args,cwd=work):
 args=list(map(str,args));p=subprocess.run(args,cwd=cwd,capture_output=True,text=True,timeout=180)
 i=len(commands);commands.append({'command':args,'returncode':p.returncode});(work/f'command-{i:03}.log').write_text(p.stdout+p.stderr);(work/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 assert p.returncode==0,(args,(p.stdout+p.stderr)[-3000:]);return p.stdout
fixture=e/'standalone-tools-fixture'
def dump(p):return json.loads(next(s[7:] for s in run([fixture,'dump',p]).splitlines() if s.startswith('RESULT:')))
results={}
for width in ('0','0.125'):
 for label,prefix in [('old',old),('new',new/'bin')]:run([prefix/'dbgaus_corr',work/('noise-'+width+'-'+label+'.sdb'),e/'baseline-inputs/sun-single.sdb',4,width])
 a,b=[dump(work/('noise-'+width+'-'+label+'.sdb')) for label in ('old','new')];assert a==b
 results['dbgaus_corr-width-'+width]={'identical':True,'keys':len(a['records'])}
# The optional key-listing utility is standalone and reads populated SUN keys.
text=run([e/'build/datalib-filedb/dbcorr_read',e/'baseline-inputs/sun.sdb']);assert text.count('Npt[1]')==9
results['dbcorr_read']={'keys':9,'standalone':True}
source=r/'evidence/chroma-storage-20261109-001/paired/suite/no_1pt/pion_3pt/output.sdb1';assert source.is_file()
before=hashlib.sha256(source.read_bytes()).hexdigest()
cfg=new/'bin/datalib-config'
def flags(n):return shlex.split(subprocess.check_output([cfg,'--'+n],text=True))
compiler=['/usr/bin/clang++',*flags('cxxflags'),*flags('storage-cxxflags')]
link=[*flags('ldflags'),*flags('storage-ldflags'),*flags('libs'),*flags('storage-libs')]
run([*compiler,r/'datalib/tests/dbreplicate_fixture.cc','-o',work/'replica-fixture',*link])
run([work/'replica-fixture',source,work/'replica-input.sdb'])
run(['/usr/bin/python3',r/'datalib/tests/test_dbreplicate.py',new/'bin/dbreplicate',new/'bin/dbmerge',new/'bin/dbutil',work/'replica-input.sdb',work/'replicate-regression'])
results['dbreplicate']=json.loads((work/'replicate-regression/results.json').read_text())
assert hashlib.sha256(source.read_bytes()).hexdigest()==before
(work/'results.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results,indent=2))
