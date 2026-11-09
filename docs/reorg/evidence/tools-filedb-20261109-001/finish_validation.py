from pathlib import Path
import subprocess,json,time,shutil,sys
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001'
# Wait only for the active builds, preserving every failed/interrupted earlier attempt.
while not ((e/'chroma-build.exit').exists() and (e/'downstream-build.exit').exists()):time.sleep(5)
# The complete stack source and protected-ref snapshots used by the audit.
def git(p,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(p),*args],text=True)
state={name:{'head':git(r/name,'rev-parse','HEAD').strip(),'status':git(r/name,'status','--short'),'local_devel':git(r/name,'for-each-ref','--format=%(objectname)','refs/heads/devel')} for name in ('datalib','adat','redstar','chroma','colorvec','qdpxx','xpath_reader','filedb')}
(e/'reorg-validation-heads.json').write_text(json.dumps(state,indent=2)+'\n')
phases=[]
for path in [Path(__file__).parent/'verify_tools.py',e/'check_api.py',e/'check_aliases.py',e/'openmp_check.py',e/'run_full.py',e/'audit.py']:
 print('Running '+path.name,flush=True)
 if path.parent!=e:shutil.copy2(path,e/path.name);path=e/path.name
 with (e/(path.stem+'-driver.log')).open('w') as f:p=subprocess.run([sys.executable,str(path)],stdout=f,stderr=subprocess.STDOUT)
 phases.append({'step':path.name,'returncode':p.returncode});(e/'phases.json').write_text(json.dumps(phases,indent=2)+'\n');p.check_returncode()
(e/'finish.exit').write_text('0\n');print('Complete tool/stack verification and native-double regression/audit passed.',flush=True)
