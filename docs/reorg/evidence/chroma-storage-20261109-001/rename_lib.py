from pathlib import Path
import json,subprocess,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';e=r/'evidence/chroma-storage-20261109-001'
assert (d/'src').is_dir() and not (d/'lib').exists()
before={str(p.relative_to(d/'src')):hashlib.sha256(p.read_bytes()).hexdigest() for p in (d/'src').rglob('*') if p.is_file()}
(d/'src').rename(d/'lib')
for name in ('Makefile.am','configure.ac'):
 p=d/name;p.write_text(p.read_text().replace('src/','lib/'))
def updated(v,key=''):
 if isinstance(v,dict):return {k:updated(x,k) for k,x in v.items()}
 if isinstance(v,list):return [updated(x,key) for x in v]
 if isinstance(v,str) and key=='destination' and v.startswith('src/'):return 'lib/'+v[4:]
 if isinstance(v,str) and v=='src/corr_graph.cc':return 'lib/corr_graph.cc'
 return v
changed=[]
for p in d.glob('*provenance.json'):
 old=json.loads(p.read_text());new=updated(old)
 if new!=old:p.write_text(json.dumps(new,indent=2)+'\n');changed.append(p.name)
p=d/'README.md';s=p.read_text();s=s.replace('Run `autoreconf -fi`','Implementation sources live in `lib/`, public headers in `include/datalib/`,\nand command-line tools in `main/`.\n\nRun `autoreconf -fi`');p.write_text(s)
subprocess.run(['git','-C',str(d),'add','--','src','lib','Makefile.am','configure.ac','README.md',*changed],check=True)
subprocess.run(['git','-C',str(d),'diff','--cached','--check'],check=True)
subprocess.run(['git','-C',str(d),'commit','-m','Rename implementation source directory to lib'],check=True)
after={str(p.relative_to(d/'lib')):hashlib.sha256(p.read_bytes()).hexdigest() for p in (d/'lib').rglob('*') if p.is_file()};assert before==after
(e/'lib-rename.json').write_text(json.dumps({'status':'passed','files_unchanged':before,'commit':subprocess.check_output(['git','-C',str(d),'rev-parse','HEAD'],text=True).strip()},indent=2)+'\n')
