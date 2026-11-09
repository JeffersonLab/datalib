from pathlib import Path
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';old=r/'evidence/tools-filedb-20261109-001'
for name in ('check_api.py','check_aliases.py','openmp_check.py','run_full.py','audit.py'):
 s=(old/name).read_text().replace('evidence/tools-filedb-20261109-001','evidence/chroma-storage-20261109-001')
 if name=='run_full.py':
  s=s.replace('Standalone datalib owns FILEDB, dbutil and graph viewers; FILEDB-free QDP++ with Chroma-owned communication adapter','Standalone datalib lib layout, no QDP options; legacy Chroma producers use datalib tensor storage directly; no Chroma FILEDB classes or temporary wrappers')
  s=s.replace("real_chroma=clean/'install/chroma/bin/chroma'",'''(e/'graph-executors.json').write_text(json.dumps([{'wrapper':str(graph_tools/tool),'wrapper_sha256':hashlib.sha256((graph_tools/tool).read_bytes()).hexdigest(),'executable':str(path),'executable_sha256':hashlib.sha256(path.read_bytes()).hexdigest()} for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib-filedb/bin/view_corr_graph')]],indent=2)+'\\n')
real_chroma=clean/'install/chroma/bin/chroma' ''')
 if name=='audit.py':s=s.replace('before = reorg / "evidence/arrays-owned-20261108-001"','before = reorg / "evidence/tools-filedb-20261109-001"')
 (e/name).write_text(s)
for name in ('export_correlators.cc','probe.cc','changes.json','environment.json'):(e/name).write_bytes((old/name).read_bytes())
for name in ('build_stack.py','prepare.py','extract.py','migrate_chroma.py','refine.py','tests.py','commit_sources.py','rename_lib.py','syntax.py','check_records.py','codec_probe.py','wire_layout.py'):(e/name).write_bytes((Path(__file__).parent/name).read_bytes())
print('regression and audit drivers prepared')
