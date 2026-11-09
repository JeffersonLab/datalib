from pathlib import Path
import subprocess,json,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';old=r/'evidence/tools-filedb-20261109-001';envs=json.loads((old/'environments.json').read_text());c=r/'chroma';config=old/'install/chroma/bin/chroma-config'
def flags(n):return shlex.split(subprocess.check_output([str(config),'--'+n],text=True))
cmd=['/opt/homebrew/bin/mpicxx','-fsyntax-only','-I'+str(c/'lib'),'-I'+str(r/'datalib/include'),'-I'+str(old/'install/datalib-filedb/include'),'-I'+str(r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/install/superbblas-mpi/include'),'-I'+str(r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/sources/primme/include'),*flags('cxxflags')]
changed=subprocess.check_output(['git','-C',str(c),'diff','--name-only'],text=True).splitlines()
for rel in changed:
 if not rel.endswith('.cc'):continue
 print('syntax',rel,flush=True)
 with (e/('syntax-'+Path(rel).stem+'.log')).open('w') as f:p=subprocess.run(cmd+[str(c/rel)],env=envs['chroma'],stdout=f,stderr=subprocess.STDOUT)
 if p.returncode:print('FAIL',flush=True);raise SystemExit(p.returncode)
print('all syntax passed',flush=True)
