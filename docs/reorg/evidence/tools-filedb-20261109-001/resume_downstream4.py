from pathlib import Path
import subprocess,json,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';old=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002';env=json.loads((e/'environments.json').read_text())['downstream'];records=[]
def run(label,args,cwd):
 args=list(map(str,args));print(label,flush=True);start=time.monotonic()
 with (e/(label+'.log')).open('w') as f:p=subprocess.run(args,cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 records.append(dict(step=label,command=args,cwd=str(cwd),returncode=p.returncode,seconds=time.monotonic()-start));(e/'downstream-retry4-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
for name in ('adat','colorvec','redstar'):
 source=e/'sources'/(name+'-004' if name=='adat' else name);build=e/'build'/(name+'-004' if name=='adat' else name);build.mkdir()
 if name=='adat':
  subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(r/name),str(source)],check=True)
  for line in subprocess.check_output(['git','-C',str(source),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
   key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(r/name/path)],check=True)
  subprocess.run(['git','-C',str(source),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],check=True)
  assert not subprocess.check_output(['git','-C',str(source),'status','--short'],text=True)
  desc=json.loads((e/'stack-sources.json').read_text());desc[name]={'source':str(source),'commit':subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip(),'branch':'rge/devel-datalib-reorg','status_before_bootstrap':''};(e/'stack-sources.json').write_text(json.dumps(desc,indent=2)+'\n')
 run(name+'-004-bootstrap',['autoreconf','-fi'],source)
 opts={'adat':['--with-libxml2=/opt/homebrew/opt/libxml2','--with-datalib='+str(e/'install/datalib-filedb'),'--with-superbblas='+str(old/'install/superbblas')],'colorvec':['--with-adat='+str(e/'install/adat'),'--with-hadron='+str(old/'install/hadron')],'redstar':['--with-colorvec='+str(e/'install/colorvec')]}[name]
 run(name+'-004-configure',[source/'configure','--prefix='+str(e/'install'/name),*opts],build)
 for step,args in [('build',['make','-j3']),('check',['make','check','-j3']),('install',['make','install'])]:run(name+'-004-'+step,args,build)
(e/'downstream-build.exit').write_text('0\n');print('Clean downstream retry finished.',flush=True)
