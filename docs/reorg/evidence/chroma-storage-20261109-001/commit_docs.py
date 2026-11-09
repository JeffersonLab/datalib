from pathlib import Path
import subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';assert (e/'finish.exit').read_text().strip()=='0'
paths={'datalib':['README.md','Makefile.am','docs/reorg/README.md','docs/reorg/FILEDB-TOOLS-REGRESSIONS.md','docs/reorg/CHROMA-STORAGE-REGRESSIONS.md','docs/reorg/evidence/chroma-storage-20261109-001','tests/fixtures/chroma-records.json'],'chroma':['README','docs/datalib-reorg.md'],'adat':['README','docs/corr-graph-v4.md','docs/datalib-api.md'],'qdpxx':['docs/filedb-reorg.md'],'redstar':['docs/view_corr_graph.md','docs/dump_corr_graph.md']};records={}
for name,select in paths.items():
 p=r/name;assert subprocess.check_output(['git','-C',str(p),'branch','--show-current'],text=True).strip()=='rge/devel-datalib-reorg';assert not subprocess.check_output(['git','-C',str(p),'diff','--cached','--name-only'],text=True)
 subprocess.run(['git','-C',str(p),'add','--',*select],check=True);subprocess.run(['git','-C',str(p),'diff','--cached','--check'],check=True)
 if subprocess.check_output(['git','-C',str(p),'diff','--cached','--name-only'],text=True).strip():subprocess.run(['git','-C',str(p),'commit','-m','Document direct datalib storage, obsolete measurement removal and validated regressions' if name=='datalib' else 'Update reorg storage and tool ownership documentation'],check=True)
 records[name]={'commit':subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip(),'status':subprocess.check_output(['git','-C',str(p),'status','--short'],text=True)}
(e/'documentation-commits.json').write_text(json.dumps(records,indent=2)+'\n');print('Reorg documentation committed; no devel or Kanoni changes staged')
