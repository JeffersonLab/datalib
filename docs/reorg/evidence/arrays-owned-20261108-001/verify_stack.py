import hashlib
import json
from pathlib import Path
import re
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
owned=json.loads((e/'owned-array-verification.json').read_text());assert owned['status']=='passed'
result={'status':'passed','owned_arrays':owned,'artifacts':owned['artifacts']}
chroma=e/'install/chroma/bin/chroma'
text=subprocess.check_output(['/usr/bin/nm','-C',str(chroma)],text=True)
assert not re.search(r'\b(ADAT|ADATIO|ADATXML|ADATUtil|ENSEM)::',text)
files=[p for p in (e/'build/chroma').rglob('*.Po') if p.read_text().strip() not in ('','# dummy')]
for p in files:
    assert not re.search(r'/(adat[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',p.read_text()),p
link=[line for line in (e/'chroma-build.log').read_text().splitlines() if re.search(r' -o chroma(?:\s|$)',line)]
assert len(link)==1 and '-ldatalib_serial' in link[0]
assert str(e/'install/datalib-filedb/lib') in link[0]
assert '-ladat' not in link[0] and '/xpath_reader/' not in link[0]
(e/'chroma-link.log').write_text(link[0]+'\n')
result['artifacts']['chroma']={'path':str(chroma),'sha256':hashlib.sha256(chroma.read_bytes()).hexdigest()}
result['chroma_dependency_records']=len(files)
result['chroma_adat_symbols']=False
sources=json.loads((e/'stack-sources.json').read_text())
for name,row in sources.items():
    source=Path(row['source'])
    assert subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip()==row['commit']
    assert not row['status_before_bootstrap']
    paths={'datalib':['src','include','tests'],'chroma':['lib','mainprogs'],'adat':['lib','main'],'colorvec':['src'],'redstar':['lib','src']}[name]
    assert not subprocess.check_output(['git','-C',str(source),'diff','HEAD','--',*paths],text=True),name
result['sources']=sources
(e/'stack-verification.json').write_text(json.dumps(result,indent=2)+'\n')
print('Complete rebuilt stack uses the owned-array datalib; source integrity and Chroma dependency checks passed.')
