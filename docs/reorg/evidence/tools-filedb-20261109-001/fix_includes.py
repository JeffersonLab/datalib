from pathlib import Path
import subprocess,json,re,os,signal
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';mapping=json.loads((e/'extraction-map.json').read_text())
changed={}
for repo in ('adat','redstar','chroma','colorvec'):
 p=r/repo;files=[]
 for rel in subprocess.check_output(['git','-C',str(p),'ls-files'],text=True).splitlines():
  f=p/rel
  if not f.is_file() or f.suffix not in ('.h','.cc') or rel.startswith('other_libs/'):continue
  s=f.read_text();t=s
  for old,new in mapping.items():
   if old.startswith('formfac/'):continue
   t=re.sub(r'(#\s*include\s*)[<"]'+re.escape(old)+r'[>"]',r'\1<'+new+'>',t)
  if s!=t:f.write_text(t);files.append(rel)
 if files:
  subprocess.run(['git','-C',str(p),'add','--',*files],check=True);subprocess.run(['git','-C',str(p),'commit','-m','Update remaining canonical datalib include paths'],check=True)
 changed[repo]=files
(e/'include-path-completion.json').write_text(json.dumps(changed,indent=2)+'\n')
# Stop only this now-obsolete build process tree; all logs are retained.
rows=[]
for line in subprocess.check_output(['ps','-axo','pid=,ppid=,args='],text=True).splitlines():
 parts=line.strip().split(None,2)
 if len(parts)==3:rows.append((int(parts[0]),int(parts[1]),parts[2]))
parents={pid for pid,ppid,args in rows if args.endswith('/tmp/datalib-tools-work/resume_downstream3.py')}
allp=set(parents)
while True:
 more={pid for pid,ppid,args in rows if ppid in allp}
 if more<=allp:break
 allp|=more
for pid in sorted(allp,reverse=True):
 try:os.kill(pid,signal.SIGTERM)
 except ProcessLookupError:pass
print('Canonical includes completed; obsolete attempt stopped:',sorted(allp))
s=Path('/tmp/datalib-tools-work/resume_downstream3.py').read_text().replace('adat-003','adat-004').replace("name+'-003' if name=='adat'","name+'-004' if name=='adat'").replace("name+'-003-", "name+'-004-").replace('downstream-retry3-build.json','downstream-retry4-build.json')
Path('/tmp/datalib-tools-work/resume_downstream4.py').write_text(s)
