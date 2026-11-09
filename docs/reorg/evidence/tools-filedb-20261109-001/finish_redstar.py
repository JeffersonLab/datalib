from pathlib import Path
import subprocess,json,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');repo=r/'redstar';e=r/'evidence/tools-filedb-20261109-001'
p=repo/'lib/corr_graph_build.cc';s=p.read_text();assert '#include "process_corrs.h"' in s;s=s.replace('#include "process_corrs.h"','#include "corr_graph_build.h"');p.write_text(s)
p=repo/'lib/corr_graph_build.h';p.write_text(p.read_text().replace('__process_corrs_h__','REDSTAR_CORR_GRAPH_BUILD_H'))
subprocess.run(['git','-C',str(repo),'add','lib/corr_graph_build.cc','lib/corr_graph_build.h'],check=True);subprocess.run(['git','-C',str(repo),'commit','-m','Complete renamed graph-construction source include'],check=True)
src=e/'sources/redstar-006';subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(repo),str(src)],check=True)
for line in subprocess.check_output(['git','-C',str(src),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
 key,path=line.split(None,1);subprocess.run(['git','-C',str(src),'config',key[:-4]+'url',str(repo/path)],check=True)
subprocess.run(['git','-C',str(src),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],check=True)
assert not subprocess.check_output(['git','-C',str(src),'status','--short'],text=True)
desc=json.loads((e/'stack-sources.json').read_text());desc['redstar']={'source':str(src),'commit':subprocess.check_output(['git','-C',str(src),'rev-parse','HEAD'],text=True).strip(),'branch':'rge/devel-datalib-reorg','status_before_bootstrap':''};(e/'stack-sources.json').write_text(json.dumps(desc,indent=2)+'\n')
b=e/'build/redstar-006';b.mkdir();env=json.loads((e/'environments.json').read_text())['downstream'];rows=[]
for label,cmd,cwd in [('bootstrap',['autoreconf','-fi'],src),('configure',[str(src/'configure'),'--prefix='+str(e/'install/redstar'),'--with-colorvec='+str(e/'install/colorvec')],b),('build',['make','-j3'],b),('check',['make','check','-j3'],b),('install',['make','install'],b)]:
 print('redstar-006-'+label,flush=True);start=time.monotonic()
 with (e/('redstar-006-'+label+'.log')).open('w') as f:p=subprocess.run(cmd,cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 rows.append({'step':label,'command':cmd,'cwd':str(cwd),'returncode':p.returncode,'seconds':time.monotonic()-start});(e/'redstar-final-build.json').write_text(json.dumps(rows,indent=2)+'\n');p.check_returncode()
(e/'downstream-build.exit').write_text('0\n');print('Final clean ADAT/ColorVec/Redstar builds and checks passed.',flush=True)
