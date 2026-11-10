from pathlib import Path
import subprocess,json,hashlib,re
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/dbtools-extract-20261109-001';changes=json.loads((e/'extraction.json').read_text());old=json.loads((e/'before.json').read_text())
def git(repo,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)
checks={}
for name in ('adat','datalib','redstar','chroma','colorvec','qdpxx'):
 repo=r/name;assert git(repo,'branch','--show-current').strip()=='rge/devel-datalib-reorg';assert git(repo,'rev-parse','HEAD').strip()==old[name]['head'];assert git(repo,'diff','--cached','--binary')==old[name]['index']
 if old[name]['devel'] is not None:assert git(repo,'show-ref','--verify','refs/heads/devel')==old[name]['devel']
 checks[name]={'head':old[name]['head'],'branch':'rge/devel-datalib-reorg','index_unchanged':True,'devel_unchanged':True}
 if name in ('chroma','colorvec','qdpxx'):assert git(repo,'status','--short')==old[name]['status']
repo=r.parent/'redstar_kanoni.devel';assert git(repo,'status','--short')==old['kanoni']['status'];assert hashlib.sha256(git(repo,'diff','--binary','HEAD').encode()).hexdigest()==old['kanoni']['diff'];assert git(repo,'rev-parse','HEAD').strip()==old['kanoni']['head'];checks['kanoni_unchanged']=True
checks['programs']={}
for tool in changes['tools']:
 assert not (r/'adat/main/dbtools'/(tool+'.cc')).exists();assert (r/'datalib/main'/(tool+'.cc')).exists()
 assert (e/'install/datalib-filedb/bin'/tool).is_file();assert (e/'install/datalib-basic/bin'/tool).is_file();assert not (e/'install/adat/bin'/tool).exists()
 binary=e/'install/datalib-filedb/bin'/tool
 nm=subprocess.check_output(['/usr/bin/nm','-C',str(binary)],text=True)
 assert not re.search(r'\b(?:ENSEM|ADAT|ADATIO|ADATXML|QDP)::',nm),tool
 checks['programs'][tool]={'standalone':True,'sha256':hashlib.sha256(binary.read_bytes()).hexdigest()}
for model in changes['models']:
 assert not (r/'adat/lib/hadron'/(model+'.h')).exists();assert not (r/'adat/lib/hadron'/(model+'.cc')).exists();assert not (e/'install/adat/include/hadron'/(model+'.h')).exists()
 assert (e/'install/datalib-core/include/datalib/legacy'/(model+'.h')).exists()
assert (r/'adat/main/dbtools/extract_corr.cc').is_file();checks['extract_corr_retained_in_adat']=True
paths=[]
for repo in ('datalib','adat','redstar'):
 for subtree in ('lib','include','main','src','tests'):
  for p in (r/repo/subtree).rglob('*'):
   if not p.is_file() or '/fixtures/' in str(p) or '/do_not_use/' in str(p) or p.suffix not in ('.cc','.h'):continue
   text=p.read_text()
   if repo=='datalib' and p.name in [x+'.cc' for x in changes['tools']]+[x+'.h' for x in changes['models']]+[x+'.cc' for x in changes['models']]:
    assert not re.search(r'#include\s*[<"](?:ensem|adat|hadron|io)/',text),str(p)
    paths.append(p)
# Verify the exact final source implementations match those compiled in isolation.
for repo in ('datalib','adat','redstar'):
 for relative in set(git(r/repo,'diff','--name-only','HEAD').splitlines()+git(r/repo,'ls-files','--others','--exclude-standard').splitlines()):
  p=r/repo/relative
  if p.is_file() and p.suffix in ('.cc','.h') and not relative.startswith(('docs/','other_libs/')):
   snapshot=e/'sources'/repo/relative
   assert snapshot.is_file() and p.read_bytes()==snapshot.read_bytes(),str(p)
checks['source_implementations_match_built_snapshots']=True
subprocess.run(['/opt/homebrew/bin/clang-format','-style=file:/Users/admin/.clang-format','--dry-run','--Werror',*map(str,paths)],check=True)
for repo in ('adat','datalib','redstar'):subprocess.run(['git','diff','--check'],cwd=r/repo,check=True)
(e/'source-audit.json').write_text(json.dumps(checks,indent=2)+'\n');print('All 20 tools standalone, one owner for extracted files, compiled-source match, formatting and protected-state checks passed.')
