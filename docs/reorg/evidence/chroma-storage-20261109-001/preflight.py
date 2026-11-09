from pathlib import Path
import subprocess,json,shlex,os
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';old=r/'evidence/tools-filedb-20261109-001';envs=json.loads((old/'environments.json').read_text());b=e/'preflight';b.mkdir(exist_ok=True)
for name,cmd,cwd in [('bootstrap',['autoreconf','-fi'],r/'datalib'),('configure',[r/'datalib/configure','--prefix='+str(e/'preflight-install'),'--with-filedb='+str(old/'install/datalib-filedb'),'--with-libxml2=/opt/homebrew/opt/libxml2'],b),('build',['make','-j6'],b),('check',['make','check','-j6'],b),('install',['make','install'],b)]:
 print(name,flush=True)
 with (e/('preflight-'+name+'.log')).open('w') as f:p=subprocess.run(list(map(str,cmd)),cwd=cwd,env=envs['datalib'],stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:raise SystemExit(p.returncode)
c=r/'chroma';config=old/'install/chroma/bin/chroma-config'
def flags(n):return shlex.split(subprocess.check_output([str(config),'--'+n],text=True))
cmd=['/usr/bin/clang++','-fsyntax-only','-I'+str(c/'lib'),'-I'+str(r/'datalib/include'),'-I'+str(old/'install/datalib-filedb/include'),*flags('cxxflags')]
changed=subprocess.check_output(['git','-C',str(c),'diff','--name-only'],text=True).splitlines()
for rel in changed:
 if not rel.endswith('.cc'):continue
 print('syntax',rel,flush=True)
 with (e/('syntax-'+Path(rel).stem+'.log')).open('w') as f:p=subprocess.run(cmd+[str(c/rel)],env=envs['chroma'],stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:print('FAIL',flush=True);raise SystemExit(p.returncode)
print('all syntax passed',flush=True)
