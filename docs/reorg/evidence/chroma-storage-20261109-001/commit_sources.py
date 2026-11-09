from pathlib import Path
import subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001'
results={}
for name,msg in [('datalib','Add serialized writes to tensor storage and remove obsolete QDP options'),('chroma','Use datalib storage directly and remove legacy database wrappers'),('adat','Rename remaining analysis utility directory to dbtools')]:
 p=r/name;assert subprocess.check_output(['git','-C',str(p),'branch','--show-current'],text=True).strip()=='rge/devel-datalib-reorg'
 names=subprocess.check_output(['git','-C',str(p),'ls-files','--modified','--deleted','--others','--exclude-standard'],text=True).splitlines();selected=[]
 for rel in names:
  if rel.startswith('other_libs/') or rel.endswith('~') or rel in ('configure','INSTALL') or 'autom4te.cache/' in rel or Path(rel).name=='Makefile.in':continue
  if rel.startswith(('include/','src/','tests/','main/','lib/')) or rel in ('Makefile.am','configure.ac','datalib-config.in','chroma-config.in','README.md'):selected.append(rel)
 for rel in selected:
  f=p/rel
  if f.is_file() and f.suffix in ('.h','.cc','.am','.ac','.in'):
   # Only strip trailing whitespace from changed lines, preserving unrelated old formatting.
   before=subprocess.run(['git','-C',str(p),'show','HEAD:'+rel],stdout=subprocess.PIPE,stderr=subprocess.DEVNULL)
   oldlines=set(before.stdout.decode().splitlines()) if before.returncode==0 else set()
   f.write_text('\n'.join(line if line in oldlines else line.rstrip() for line in f.read_text().splitlines())+'\n')
 subprocess.run(['git','-C',str(p),'add','--',*selected],check=True)
 subprocess.run(['git','-C',str(p),'diff','--cached','--check'],check=True)
 subprocess.run(['git','-C',str(p),'commit','-m',msg],check=True)
 results[name]={'commit':subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip(),'paths':selected}
(e/'source-commits.json').write_text(json.dumps(results,indent=2)+'\n')
