from pathlib import Path
import subprocess,json,time,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';d=r/'datalib'
p=d/'include/datalib/storage/dbtype.h';s=p.read_text();assert '  std::vector<int> getLattSize(const std::string& dbfile)\n' in s;s=s.replace('  std::vector<int> getLattSize(const std::string& dbfile)\n','  inline std::vector<int> getLattSize(const std::string& dbfile)\n');p.write_text(s)
subprocess.run(['git','-C',str(d),'add','include/datalib/storage/dbtype.h'],check=True);subprocess.run(['git','-C',str(d),'commit','-m','Make header-only lattice metadata helper inline'],check=True)
envs=json.loads((e/'environments.json').read_text());old=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002';records=[]
def run(label,cmd,cwd,kind):
 cmd=list(map(str,cmd));executed=cmd if kind!='datalib' else ['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*cmd];print(label,flush=True);start=time.monotonic()
 with (e/(label+'.log')).open('w') as f:p=subprocess.run(executed,cwd=cwd,env=envs[kind],stdout=f,stderr=subprocess.STDOUT)
 records.append(dict(step=label,command=cmd,executed_command=executed,cwd=str(cwd),returncode=p.returncode,seconds=time.monotonic()-start));(e/'final-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
def clone(name):
 source=e/'sources'/(name+'-final');subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(r/name),str(source)],check=True)
 if (source/'.gitmodules').exists():
  for line in subprocess.check_output(['git','-C',str(source),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
   key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(r/name/path)],check=True)
  subprocess.run(['git','-C',str(source),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],check=True)
 assert not subprocess.check_output(['git','-C',str(source),'status','--short'],text=True)
 desc=json.loads((e/'stack-sources.json').read_text());desc[name]={'source':str(source),'commit':subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip(),'branch':'rge/devel-datalib-reorg','status_before_bootstrap':''};(e/'stack-sources.json').write_text(json.dumps(desc,indent=2)+'\n');return source
source=clone('datalib');run('final-datalib-bootstrap',['autoreconf','-fi'],source,'datalib')
for name,opts in [('datalib-core',['--without-filedb']),('datalib-filedb',['--with-filedb=bundled','--with-superbblas='+str(old/'install/superbblas')])]:
 b=e/'build'/(name+'-final');b.mkdir();run('final-'+name+'-configure',[source/'configure','--prefix='+str(e/'install'/name),'--without-qdp','--enable-openmp','--with-libxml2=/opt/homebrew/opt/libxml2',*opts],b,'datalib')
 for step,cmd in [('build',['make','-j3']),('check',['make','check','-j3']),('install',['make','install'])]:run('final-'+name+'-'+step,cmd,b,'datalib')
# Rebuild every CPU consumer into a fresh build and empty installation, retaining earlier prefixes.
for name in ('adat','colorvec','redstar'):
 if (e/'install'/name).exists():shutil.move(e/'install'/name,e/'install'/(name+'-prior'))
 source=clone(name);b=e/'build'/(name+'-final');b.mkdir();run('final-'+name+'-bootstrap',['autoreconf','-fi'],source,'downstream')
 opts={'adat':['--with-libxml2=/opt/homebrew/opt/libxml2','--with-datalib='+str(e/'install/datalib-filedb'),'--with-superbblas='+str(old/'install/superbblas')],'colorvec':['--with-adat='+str(e/'install/adat'),'--with-hadron='+str(old/'install/hadron')],'redstar':['--with-colorvec='+str(e/'install/colorvec')]}[name]
 run('final-'+name+'-configure',[source/'configure','--prefix='+str(e/'install'/name),*opts],b,'downstream')
 for step,cmd in [('build',['make','-j3']),('check',['make','check','-j3']),('install',['make','install'])]:run('final-'+name+'-'+step,cmd,b,'downstream')
(e/'downstream-build.exit').write_text('0\n');print('Final standalone and CPU builds/checks/installations passed.',flush=True)
