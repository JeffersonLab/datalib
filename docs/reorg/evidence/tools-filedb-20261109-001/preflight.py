from pathlib import Path
import json,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';e=r/'evidence/tools-filedb-20261109-001'
p=d/'src/legacy_particle_op.cc';p.write_text(p.read_text().replace('#include "irrep_util.h"','#include <datalib/graph/naming.h>'))
p=d/'main/view_corr_graph.cc';p.write_text(p.read_text().replace('<hadron/ensem_filenames.h>','<datalib/graph/filenames.h>'))
env=json.loads((r/'evidence/arrays-owned-20261108-001/environments.json').read_text())['datalib']
b=e/'preflight';b.mkdir(exist_ok=True)
for name,cmd,cwd in [('bootstrap',['autoreconf','-fi'],d),('configure',[str(d/'configure'),'--prefix='+str(e/'preflight-install'),'--without-qdp','--with-libxml2=/opt/homebrew/opt/libxml2'],b),('build',['make','-j6'],b)]:
 print(name,flush=True)
 with (e/('preflight-'+name+'.log')).open('w') as f:p=subprocess.run(cmd,cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:print('FAILED',name,flush=True);raise SystemExit(p.returncode)
