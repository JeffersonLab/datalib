from pathlib import Path
import subprocess,json,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001'
def git(p,*a):return subprocess.check_output(['git','--no-optional-locks','-C',str(p),*a],text=True)
for name,expected in json.loads((e/'protected-before.json').read_text()).items():
 p=r.parent/name;assert git(p,'rev-parse','refs/heads/devel')==expected['devel'],name
 if 'status' in expected:assert git(p,'status','--porcelain=v1','--branch')==expected['status'];assert hashlib.sha256(git(p,'diff','--binary','HEAD').encode()).hexdigest()==expected['diff']
rows=json.loads((e/'stack-sources.json').read_text());code_paths={'datalib':['lib','include','main','configure.ac','datalib-config.in','tests/records_probe.cc','tests/check-records.sh'],'chroma':['lib','mainprogs','configure.ac','chroma-config.in','Makefile.am'],'adat':['lib','main','configure.ac'],'redstar':['lib','src'],'colorvec':['src']}
results={}
for name,row in rows.items():
 p=r/name;assert git(p,'branch','--show-current').strip()=='rge/devel-datalib-reorg';assert not git(p,'diff',row['commit'],'HEAD','--',*code_paths[name]),name;results[name]={'head':git(p,'rev-parse','HEAD').strip(),'tested_code_commit':row['commit'],'status':git(p,'status','--short')}
for name in ('qdpxx','xpath_reader','filedb'):assert git(r/name,'branch','--show-current').strip()=='rge/devel-datalib-reorg'
assert json.loads((e/'publication-package.json').read_text())['status']=='passed';assert json.loads((e/'independent-audit.json').read_text())['status']=='passed'
(e/'final-verification.json').write_text(json.dumps({'status':'passed','protected_devel_refs_unchanged':True,'kanoni_status_and_diff_unchanged':True,'tested_code_unchanged_after_docs':True,'repositories':results},indent=2)+'\n');print('Protected refs and Kanoni preserved; final code matches tested stack')
