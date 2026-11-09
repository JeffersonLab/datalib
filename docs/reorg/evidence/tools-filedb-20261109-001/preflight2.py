from pathlib import Path
import subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';d=r/'datalib'
env=json.loads((r/'evidence/arrays-owned-20261108-001/environments.json').read_text())['datalib']
def run(label,cmd,cwd):
 print(label,flush=True)
 with (e/(label+'.log')).open('w') as f:p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:print('FAILED '+label,flush=True);raise SystemExit(p.returncode)
run('preflight2-bootstrap',['autoreconf','-fi'],d)
for name,opts in [('core',['--without-filedb']),('bundled',[]),('tensor',['--with-superbblas='+str(r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/install/superbblas')])]:
 b=e/('preflight2-'+name);b.mkdir()
 if name=='tensor':env['LDFLAGS']+=' -framework Accelerate'
 run(name+'-configure',[d/'configure','--prefix='+str(e/('preflight2-install-'+name)),'--without-qdp','--with-libxml2=/opt/homebrew/opt/libxml2',*opts],b)
 for label,cmd in [('build',['make','-j6']),('check',['make','check','-j6']),('install',['make','install'])]:run(name+'-'+label,cmd,b)
print('Preflight build/check/install core, bundled FILEDB and tensor tools passed.',flush=True)
