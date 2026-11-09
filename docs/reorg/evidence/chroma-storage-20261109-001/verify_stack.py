from pathlib import Path
import subprocess,json,re,hashlib
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';results={};artifacts={}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for name,count in [('datalib-core',5),('datalib-filedb',8)]:
 b=e/'build'/name;p=e/'install'/name;s=(b/'test-suite.log').read_text();assert re.search(r'# TOTAL:\s*'+str(count)+r'\b',s) and re.search(r'# PASS:\s*'+str(count)+r'\b',s)
 cfg=p/'bin/datalib-config';flags={n:subprocess.check_output([str(cfg),'--'+n],text=True).strip() for n in ('cxxflags','ldflags','libs')};assert not any(re.search('xpath_reader|XPathReader|xmlWriter|libadat|lqdp|lqmp',s) for s in flags.values())
 deps=[x for x in b.rglob('*.Po') if x.read_text().strip() not in ('','# dummy')]
 for x in deps:assert not re.search(r'/(adat[^/\s]*|qdp[^/\s]*|qmp[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',x.read_text()),x
 help=subprocess.check_output([str(e/'sources/datalib/configure'),'--help'],text=True);assert not re.search(r'--(?:with|without)-(?:qdp|qdpxx)',help)
 assert not (e/'sources/datalib/src').exists();assert (e/'sources/datalib/lib/binary.cc').is_file()
 results[name]={'checks':count,'dependency_records':len(deps),'flags':flags};f=p/'lib/libdatalib_serial.a';artifacts[name]={'path':str(f),'sha256':sha(f)}
for flag in ('cxxflags','ldflags','libs'):
 cfg=e/'install/datalib-filedb/bin/datalib-config'
 assert subprocess.check_output([str(cfg),'--storage-'+flag])==subprocess.check_output([str(cfg),'--filedb-'+flag])
c=e/'sources/chroma'
assert not (e/'install/chroma/include/util/ferm/key_val_db.h').exists();assert not (e/'install/chroma/include/util/ferm/filedb_qdp.h').exists();assert (e/'install/chroma/include/util/ferm/storage_io.h').is_file()
for rel in subprocess.check_output(['git','-C',str(c),'ls-files','lib','configure.ac','chroma-config.in'],text=True).splitlines():
 f=c/rel
 if f.is_file() and f.suffix in ('.cc','.h','.ac','.in'):
  assert not re.search(r'FILEDB|BinaryStoreDB|LocalSerialDB|key_val_db|filedb_qdp',f.read_text()),rel
f=e/'install/chroma/bin/chroma';syms=subprocess.check_output(['/usr/bin/nm','-C',str(f)],text=True);assert not re.search(r'\b(ADAT|ADATIO|ADATXML|ADATUtil|ENSEM)::',syms);artifacts['chroma']={'path':str(f),'sha256':sha(f)}
deps=[p for p in (e/'build/chroma').rglob('*.Po') if p.read_text().strip() not in ('','# dummy')]
for p in deps:assert not re.search(r'/(adat[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',p.read_text()),p
links=[s for s in ((e/'chroma-build.log').read_text()+'\n'+(e/'chroma-install.log').read_text()+'\n'+(e/'chroma-profile-build.log').read_text()+'\n'+(e/'chroma-profile-install.log').read_text()+'\n'+(e/'chroma-removal-build.log').read_text()+'\n'+(e/'chroma-removal-install.log').read_text()).splitlines() if re.search(r' -o chroma(?:\s|$)',s)];assert links and '-ldatalib_serial' in links[-1] and '-ladat' not in links[-1];(e/'chroma-link.log').write_text(links[-1]+'\n')
flags=subprocess.check_output([str(e/'install/chroma/bin/chroma-config'),'--cxxflags'],text=True);assert str(e/'install/datalib-filedb/include') in flags
assert json.loads((e/'stoch-hadron-removal.json').read_text())['status']=='removed and verified'
assert json.loads((e/'storage-abi-after.json').read_text())['status']=='passed'
assert not (e/'sources/adat/main/dbutil').exists();assert (e/'sources/adat/main/dbtools').is_dir();assert not (e/'install/adat/bin/dbutil').exists();assert (e/'install/datalib-filedb/bin/dbutil').is_file()
for name in ('adat','qdpxx'):assert not subprocess.check_output(['git','-C',str(r/name),'ls-tree','HEAD','other_libs/filedb'],text=True)
for name in ('view_corr_graph','dump_corr_graph'):assert not (e/'install/redstar/bin'/name).exists()
sources=json.loads((e/'stack-sources.json').read_text())
for name,row in sources.items():
 p=Path(row['source']);assert subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip()==row['commit'];assert row['status_before_bootstrap']==''
 directories={'datalib':['lib','include','main','tests'],'adat':['lib','main'],'chroma':['lib','mainprogs'],'colorvec':['src'],'redstar':['lib','src']}[name]
 assert not subprocess.check_output(['git','-C',str(p),'diff','HEAD','--',*directories],text=True),name
(e/'stack-verification.json').write_text(json.dumps({'status':'passed','artifacts':artifacts,'checks':results,'chroma_no_adat_symbols':True,'chroma_dependency_records':len(deps),'chroma_no_explicit_filedb_api':True,'temporary_wrappers_removed':True,'datalib_lib_layout':True,'qdp_build':'unchanged validated FILEDB-free installation from tools-filedb-20261109-001; ARM/SIMD check limitation remains unchanged','sources':sources},indent=2)+'\n');print('Clean sources, standalone datalib, ADAT-free Chroma, storage API and tool ownership verified')
