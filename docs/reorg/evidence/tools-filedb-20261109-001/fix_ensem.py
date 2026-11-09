from pathlib import Path
import subprocess,re,shlex,json,concurrent.futures
r=Path('/Users/admin/Downloads/macos-datalib-reorg');a=r/'adat';e=r/'evidence/tools-filedb-20261109-001'
p=a/'main/dbutil/dbwick_rescale.cc';s=p.read_text();s=s if '#include <ensem/ensem.h>' in s else s.replace('#include <recipes/nr.h>','#include <recipes/nr.h>\n#include <ensem/ensem.h>');s=re.sub(r'(?<![\w:])\b(VectorComplex|Double)\b',r'ENSEM::\1',s);p.write_text(s)
cfg=e/'build/adat-004/adat-config';flags=shlex.split(subprocess.check_output(['/bin/sh',str(cfg),'--cxxflags'],text=True));flags+=['-I'+str(a/'lib'),'-I'+str(e/'build/adat-004/include'),'-I'+str(a/'other_libs/xpath_reader/include')]
# Check all retained database utilities before another complete clean build.
files=list((a/'main/dbutil').glob('*.cc'))
def check(p):
 with (e/('syntax-'+p.stem+'.log')).open('w') as f:x=subprocess.run(['/usr/bin/clang++',*flags,'-fsyntax-only',str(p)],stdout=f,stderr=subprocess.STDOUT)
 return {'source':str(p.relative_to(a)),'returncode':x.returncode}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(check,files))
(e/'adat-utility-syntax.json').write_text(json.dumps(results,indent=2)+'\n')
print([x for x in results if x['returncode']])
if all(x['returncode']==0 for x in results):
 subprocess.run(['git','-C',str(a),'add','main/dbutil/dbwick_rescale.cc'],check=True);subprocess.run(['git','-C',str(a),'commit','-m','Qualify ENSEM types in retained Wick rescaling utility'],check=True)
