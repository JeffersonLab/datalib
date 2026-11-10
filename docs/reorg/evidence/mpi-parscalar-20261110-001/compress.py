from pathlib import Path
import json,gzip,hashlib,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';state=e/'compressed-diagnostics.json';rows=json.loads(state.read_text()) if state.exists() else []
for lane in ('paired','npt-only'):
 result=e/lane/'suite/results.json'
 if not result.exists():continue
 suite=json.loads(result.read_text())
 for case in suite.get('cases',[]):
  if case['status']!='passed':continue
  for p in (e/lane/'suite/no_1pt'/case['case']).glob('*.log'):
   if p.stat().st_size<64*1024*1024:continue
   size=p.stat().st_size;sha=hashlib.sha256(p.read_bytes()).hexdigest();target=p.with_suffix('.log.gz')
   with p.open('rb') as inp,gzip.open(target,'xb',compresslevel=3) as out:shutil.copyfileobj(inp,out)
   with gzip.open(target,'rb') as inp:
    digest=hashlib.sha256()
    for block in iter(lambda:inp.read(1024*1024),b''):digest.update(block)
   assert digest.hexdigest()==sha
   rows.append({'archive':str(target.relative_to(e)),'original_bytes':size,'original_sha256':sha,'archive_bytes':target.stat().st_size})
   p.unlink()
   state.write_text(json.dumps(rows,indent=2)+'\n');print('Verified archive:',target.relative_to(e),flush=True)
