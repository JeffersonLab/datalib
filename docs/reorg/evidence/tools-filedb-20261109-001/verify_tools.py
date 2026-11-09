from pathlib import Path
import subprocess,json,hashlib,re,shlex,tarfile
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';old=r/'evidence/arrays-owned-20261108-001';results={};artifacts={}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for name,count in [('datalib-core',5),('datalib-filedb',7)]:
 b=e/'build'/(name+'-final');p=e/'install'/name;s=(b/'test-suite.log').read_text();assert re.search(r'# TOTAL:\s*'+str(count)+r'\b',s) and re.search(r'# PASS:\s*'+str(count)+r'\b',s)
 cfg=p/'bin/datalib-config';flags={k:subprocess.check_output([str(cfg),'--'+k],text=True).strip() for k in ('cxxflags','ldflags','libs')}
 assert not any(re.search('xpath_reader|XPathReader|xmlWriter|libadat|lqdp|lqmp',v) for v in flags.values())
 deps=[x for x in b.rglob('*.Po') if x.read_text().strip() not in ('','# dummy')]
 for x in deps:assert not re.search(r'/(adat[^/\s]*|qdp[^/\s]*|qmp[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',x.read_text()),x
 results[name]={'checks':count,'dependency_records':len(deps),'flags':flags};f=p/'lib/libdatalib_serial.a';artifacts[name]={'path':str(f),'sha256':sha(f)}
# Compare moved tools against the last installed implementations on real saved graphs.
comparisons=[]
for case in ('pion','3pi','nuc_nuc_3pt','eta_eta_3pt'):
 inp=old/'paired/suite/no_1pt'/case/'corr_graph.bin';item={'case':case}
 for label,exe in [('before',old/'install/redstar/bin/view_corr_graph'),('after',e/'install/datalib-core/bin/view_corr_graph')]:
  text=subprocess.check_output([str(exe),str(inp)],text=True);text='\n'.join(x for x in text.splitlines() if not x.startswith('Time to read'))
  item[label+'_summary_sha256']=hashlib.sha256(text.encode()).hexdigest()
 assert item['before_summary_sha256']==item['after_summary_sha256'],case
 for label,exe in [('before',old/'install/redstar/bin/dump_corr_graph'),('after',e/'install/datalib-core/bin/dump_corr_graph')]:
  out=e/f'graph-{case}-{label}.xml';subprocess.run([str(exe),str(inp),str(out)],check=True,stdout=subprocess.DEVNULL);item[label+'_xml_sha256']=sha(out)
 assert item['before_xml_sha256']==item['after_xml_sha256'],case
 comparisons.append(item)
# FILEDB ownership and exported flags come from datalib, with no ADAT/QDP dependency.
for name in ('adat','qdpxx'):
 source=r/name;assert not subprocess.check_output(['git','-C',str(source),'ls-tree','HEAD','other_libs/filedb'],text=True)
assert subprocess.check_output(['git','-C',str(r/'datalib'),'rev-parse','HEAD:other_libs/filedb'],text=True).strip()=='db2a795aaf6cc57b6eefe2ef634fdc91ee7b3693'
qp=e/'install/qdpxx-mpi';qflags=subprocess.check_output([str(qp/'bin/qdp++-config'),'--libs'],text=True);assert '-lfiledb' not in qflags and '-lfilehash' not in qflags
assert not list((qp/'include').glob('qdp_db*.h'));assert not list((qp/'include').glob('*StoreDB*.h'));assert not list((qp/'lib').glob('*filedb*'));assert not list((qp/'lib').glob('*filehash*'))
# The Chroma legacy API is retained; no measurement sources were rewritten for this ownership move.
base=json.loads((e/'reorg-before.json').read_text())['chroma']['head']
assert not subprocess.check_output(['git','-C',str(r/'chroma'),'diff',base,'HEAD','--','lib/meas'],text=True)
cp=e/'install/chroma/bin/chroma';final_link=json.loads((e/'chroma-final-link.json').read_text());assert final_link['datalib_archive_sha256']==artifacts['datalib-filedb']['sha256'];assert final_link['executable_sha256']==sha(cp);syms=subprocess.check_output(['/usr/bin/nm','-C',str(cp)],text=True);assert not re.search(r'\b(ADAT|ADATIO|ADATXML|ADATUtil|ENSEM)::',syms)
deps=[p for p in (e/'build/chroma').rglob('*.Po') if p.read_text().strip() not in ('','# dummy')]
for p in deps:assert 'datalib/storage/dbtype.h' not in p.read_text()
for p in deps:assert not re.search(r'/(adat[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',p.read_text()),p
links=[]
for f in ('chroma-build.log','chroma-resumed-build.log','chroma-final-link.log'):
 if (e/f).exists():links += [x for x in (e/f).read_text().splitlines() if re.search(r' -o chroma(?:\s|$)',x)]
assert links and all('-ldatalib_serial' in x and '-ladat' not in x for x in links)
(e/'chroma-link.log').write_text(links[-1]+'\n');artifacts['chroma']={'path':str(cp),'sha256':sha(cp)}
for name in ('chroma','redstar'):
 f=e/'install'/name/'bin'/('chroma' if name=='chroma' else 'redstar_npt');s=subprocess.check_output(['/usr/bin/otool','-L',str(f)],text=True);assert 'libomp' in s and 'libgomp' not in s and 'libstdc++' not in s;(e/(name+'-dynamic-libraries.log')).write_text(s)
# No duplicate executables or old shared headers are installed by ADAT/Redstar.
assert not (e/'install/adat/bin/dbutil').exists();assert not (e/'install/adat/lib/libdbutil.a').exists()
for name in ('view_corr_graph','dump_corr_graph'):assert not (e/'install/redstar/bin'/name).exists()
assert not (e/'install/datalib-core/include/datalib/graph/process_corrs.h').exists()
sources=json.loads((e/'stack-sources.json').read_text())
for name,row in sources.items():
 p=Path(row['source']);assert subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip()==row['commit'];assert row['status_before_bootstrap']==''
 paths={'datalib':['src','include','main','tests'],'adat':['lib','main'],'chroma':['lib','mainprogs'],'colorvec':['src'],'redstar':['lib','src']}[name]
 assert not subprocess.check_output(['git','-C',str(p),'diff','HEAD','--',*paths],text=True),name
assert json.loads((e/'qdp-check-limitation.json').read_text())['baseline_same_simd_failure']
(e/'stack-verification.json').write_text(json.dumps({'status':'passed','artifacts':artifacts,'checks':results,'graph_tool_comparisons':comparisons,'filedb_owner':'datalib','filedb_commit':'db2a795aaf6cc57b6eefe2ef634fdc91ee7b3693','chroma_measurements_unchanged':True,'chroma_adat_symbols':False,'chroma_dependency_records':len(deps),'sources':sources,'qdp_full_check':'existing ARM/SIMD failure reproduced on baseline; see qdp-check-limitation.json'},indent=2)+'\n')
print('Standalone tool formats, FILEDB ownership and complete stack verified.',flush=True)
