from pathlib import Path
import subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001'
for rel in ('adat/CMakeLists.txt','adat/adat-config.in','adat/main/dbutil/Makefile.am'):
 p=r/rel;p.write_text('\n'.join(line.rstrip() for line in p.read_text().splitlines())+'\n')
messages={'datalib':'Own FILEDB and standalone graph and database inspection tools','adat':'Use datalib storage metadata and legacy models directly','redstar':'Use datalib graph tools and separate graph construction API','chroma':'Use datalib FILEDB and own QDP communication adapter','colorvec':'Use canonical datalib naming and correlator storage APIs'}
results={}
for name,msg in messages.items():
 p=r/name
 # Stage only changed canonical text files and explicit gitlinks; no generated metadata or prior work.
 names=subprocess.check_output(['git','-C',str(p),'ls-files','--modified','--deleted','--others','--exclude-standard'],text=True).splitlines()
 select=[]
 for rel in names:
  f=p/rel
  if rel.startswith('other_libs/') or rel.endswith('~') or rel in ('configure','INSTALL') or rel.startswith('autom4te.cache/'):continue
  if f.suffix in ('.h','.cc','.am','.ac','.in','.md','.json','.py','.sh','.bin','.txt') or rel in ('.gitmodules','CMakeLists.txt'):
   # Makefile.in is generated; only explicit config template edits should enter.
   if f.name=='Makefile.in':continue
   select.append(rel)
 if name=='datalib':select+=['.gitmodules','other_libs/filedb']
 if select:subprocess.run(['git','-C',str(p),'add','--',*select],check=True)
 subprocess.run(['git','-C',str(p),'diff','--cached','--check'],check=True)
 diff=subprocess.check_output(['git','-C',str(p),'diff','--cached','--name-only'],text=True)
 if diff.strip():subprocess.run(['git','-C',str(p),'commit','-m',msg],check=True)
 results[name]={'commit':subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip(),'status':subprocess.check_output(['git','-C',str(p),'status','--short'],text=True)}
(e/'source-commits.json').write_text(json.dumps(results,indent=2)+'\n')
