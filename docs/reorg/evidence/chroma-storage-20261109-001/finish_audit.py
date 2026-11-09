from pathlib import Path
import subprocess,json,time,sys
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';start=time.monotonic()
while not (e/'regression.exit').exists():
 for lane in ('paired','npt-only'):
  p=e/(lane+'.exit')
  if p.exists():assert p.read_text().strip()=='0',(lane,p.read_text())
 if time.monotonic()-start>3600:raise RuntimeError('Full suite has not finished within one hour')
 time.sleep(3)
assert (e/'regression.exit').read_text().strip()=='0'
with (e/'audit-driver.log').open('w') as f:subprocess.run([sys.executable,str(e/'audit.py')],stdout=f,stderr=subprocess.STDOUT,check=True)
a=json.loads((e/'independent-audit.json').read_text());assert a['status']=='passed';producer=json.loads((e/'producer-comparison.json').read_text());assert len(producer)==6 and all(x['returncode']==0 for x in producer)
current={'status':'passed','core_checks':5,'storage_checks':8,'source_distribution':'passed','storage_abi':'passed for 7 reader/writer/direct-client types','obsolete_stoch_hadron':'removed and verified','old_new_producer_comparisons':6,'paired_cases':a['paired']['passed'],'npt_only_cases':a['npt_only']['passed'],'full_audit':'passed','superseded_attempts':'Initial metadata gate expected raw metadata equality; corrected to validate run date, MPI geometry and the documented automatic meson batch default separately. Numerical gate unchanged.'};(e/'validation-final.json').write_text(json.dumps(current,indent=2)+'\n');(e/'finish.exit').write_text('0\n');print(json.dumps(current,indent=2))
