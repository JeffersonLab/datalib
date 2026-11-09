from pathlib import Path
import subprocess,json,sys,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';assert (e/'build.exit').read_text().strip()=='0'
phases=[]
for name,args in [('storage_abi_after.py',[]),('verify_stack.py',[]),('check_aliases.py',[]),('openmp_check.py',[]),('clean_codec.py',[]),('producer_runs.py',['new']),('producer_compare.py',[]),('run_full.py',[]),('audit.py',[])]:
 src=Path(__file__).parent/name
 if src.exists():(e/name).write_bytes(src.read_bytes())
 if name=='producer_compare.py':
  old=e/'producer-readback-build.log'
  if old.exists():old.rename(e/'producer-readback-initial-build-error.log')
 print('Running '+name+' '+' '.join(args),flush=True)
 with (e/(Path(name).stem+'-driver.log')).open('w') as f:p=subprocess.run([sys.executable,str(e/name),*args],stdout=f,stderr=subprocess.STDOUT)
 phases.append({'phase':name,'args':args,'returncode':p.returncode});(e/'phases.json').write_text(json.dumps(phases,indent=2)+'\n');p.check_returncode()
(e/'finish.exit').write_text('0\n');print('Clean builds, producers and full regression/audit passed',flush=True)
