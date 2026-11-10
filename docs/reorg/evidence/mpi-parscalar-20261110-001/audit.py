from pathlib import Path
import hashlib,json,importlib.util,math,subprocess,xml.etree.ElementTree as ET
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';k=r.parent/'redstar_kanoni.devel'
spec=importlib.util.spec_from_file_location('audit_helpers',e/'audit_helpers.py');h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
policy=json.loads((k/'docs/regression-policy.json').read_text());inventory={p.parent.name for p in (k/'regressions/redstar').glob('*/case.json')};deferred={n for n,v in policy['deferred_by_user'].items() if v['corr_superb']};npt_cases={n for n in deferred if not policy['deferred_by_user'][n]['npt']}
paired=h.audit_lane('paired','native-full',True,inventory,deferred);npt=h.audit_lane('npt-only','native-npt-only',False,npt_cases,set())
assert (paired['passed'],paired['keys'],paired['values'])==(30,925,4197)
assert (npt['passed'],npt['keys'],npt['values'])==(3,6,96)
resource=[];geometry={}
for row in paired['cases']:
 folder=e/'paired/suite/no_1pt'/row['case'];root=ET.parse(folder/'chroma.out.xml')
 geoms=[]
 for node in root.findall('.//Setgeom'):
  get=lambda tag:list(map(int,node.findtext(tag).split()))
  dims=get('latt_size');logical=get('logical_size');subgrid=get('subgrid_size')
  assert dims==[4,4,4,16] and math.prod(logical)==2 and math.prod(subgrid)==512
  assert [a*b for a,b in zip(logical,subgrid)]==dims
  assert int(node.findtext('total_volume'))==1024 and int(node.findtext('subgrid_volume'))==512
  geoms.append({'logical_size':logical,'subgrid_size':subgrid})
 assert geoms,row['case'];geometry[row['case']]=geoms
 for rank in (0,1):
  record=json.loads((folder/f'resource-rank-{rank}.json').read_text())
  assert record['rank']==rank and record['returncode']==0 and record['error'] is None
  assert record['rss_limit_mb']==8192 and record['rss_samples']>0
  assert 0<record['peak_rss_kib']<=8192*1024 and record['omp_num_threads']==2
  assert str(e/'install/chroma/bin/chroma') in record['command']
  resource.append({'case':row['case'],**record})
for result in (paired,npt):
 for row in result['cases']:assert row['npt_unevaluable_graphs']==row['prior_npt_unevaluable_graphs']
(e/'devel-after.log').write_text(h.git_snapshot());assert (e/'devel-after.log').read_bytes()==(e/'devel-before.log').read_bytes()
boundaries=json.loads((e/'boundary-probe.json').read_text());assert [x['result']['mpi_ranks'] for x in boundaries]==[1,2]
producer=json.loads((e/'producer-comparison.json').read_text());assert len(producer)==6 and all(x['returncode']==0 for x in producer)
assert subprocess.check_output([e/'install/qmp/bin/qmp-config','--comms'],text=True).strip()=='MPI'
assert subprocess.check_output([e/'install/qdpxx/bin/qdp++-config','--parallel-arch'],text=True).strip()=='parscalar'
for name in ('chroma','qdpxx'):
 libs=subprocess.check_output(['/usr/bin/otool','-L',str(e/'install/chroma/bin/chroma' if name=='chroma' else e/'boundary-probe')],text=True)
 assert 'libmpi' in libs and 'libomp' in libs and 'libgomp' not in libs and 'libstdc++' not in libs
 (e/(name+'-dynamic-libraries.log')).write_text(libs)
result={'status':'passed_with_existing_qdp_test_limitation','qmp_comms':'MPI','qdp_parallel_arch':'parscalar','paired':paired,'npt_only':npt,'mpi_ranks':2,'chroma_openmp_threads_per_rank':2,'npt_openmp_threads':1,'rank_resources':resource,'rank_geometry':geometry,'boundary_probes':boundaries,'producers':producer,'max_paired_absolute_error':max(c['max_abs_error'] for c in paired['cases']),'max_rank_rss_kib':max(c['peak_rss_kib'] for c in resource),'existing_qdp_make_check_failure':json.loads((e/'qdpxx-check-limitation.json').read_text()),'devel_and_kanoni_unchanged':True}
(e/'independent-audit.json').write_text(json.dumps(result,indent=2)+'\n')
print('Full two-rank matrix and independent audit passed; legacy QDP SIMD unit-program compile failure retained.')
