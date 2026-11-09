from pathlib import Path
import ast
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';old=r/'evidence/arrays-owned-20261108-001'
wrapper_setup='''redstar=stack/'install/redstar/bin'
graph_tools=e/'graph-tools'
graph_tools.mkdir()
for tool,path in [('redstar_corr_graph',redstar/'redstar_corr_graph'),('view_corr_graph',e/'install/datalib-filedb/bin/view_corr_graph')]:
    wrapper=graph_tools/tool
    wrapper.write_text('#!/bin/sh\\nexec '+shlex.quote(str(path))+' "$@"\\n')
    wrapper.chmod(0o755)
'''
for name in ('check_api.py','check_aliases.py','openmp_check.py','run_full.py','audit.py'):
 s=(old/name).read_text().replace('evidence/arrays-owned-20261108-001','evidence/tools-filedb-20261109-001')
 if name=='run_full.py':
  s=s.replace("redstar=stack/'install/redstar/bin'",wrapper_setup.rstrip())
  s=s.replace("'--baseline-graph-bin',redstar,'--feature-graph-bin',redstar", "'--baseline-graph-bin',graph_tools,'--feature-graph-bin',graph_tools")
  s=s.replace("'--dbutil',adat/'bin/dbutil'", "'--dbutil',e/'install/datalib-filedb/bin/dbutil'")
  s=s.replace('Bundled-array datalib without external xpath_reader','Standalone datalib owns FILEDB, dbutil and graph viewers; FILEDB-free QDP++ with Chroma-owned communication adapter')
 if name=='audit.py':s=s.replace('before = reorg / "evidence/native-double-clean-full-20261107-002"','before = reorg / "evidence/arrays-owned-20261108-001"')
 (e/name).write_text(s)
s=(old/'export_correlators.cc').read_text().replace('"hadron/ensem_filenames.h"','<datalib/graph/filenames.h>').replace('"io/superb_kv.h"','<datalib/storage/corr_reader.h>').replace('ADATIO::StorageCorrFunc','Datalib::Storage::StorageCorrFunc');(e/'export_correlators.cc').write_text(s)
s=(old/'probe.cc').read_text().replace('datalib/graph/process_corrs.h','datalib/graph/corr_graph.h').replace('ADATIO::StorageCorrFunc','Datalib::Storage::StorageCorrFunc');(e/'probe.cc').write_text(s)
(e/'changes.json').write_bytes((old/'changes.json').read_bytes());(e/'environment.json').write_bytes((old/'environment.json').read_bytes())
for f in e.glob('*.py'):ast.parse(f.read_text())
print('Regression driver copies prepared; canonical Kanoni files unchanged.')
