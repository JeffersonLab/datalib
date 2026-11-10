from pathlib import Path
import hashlib,json,importlib.util,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/dbtools-extract-20261109-001';kanoni=r.parent/'redstar_kanoni.devel'
spec=importlib.util.spec_from_file_location('audit_helpers',e/'audit_helpers.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
policy=json.loads((kanoni/'docs/regression-policy.json').read_text());inventory={p.parent.name for p in (kanoni/'regressions/redstar').glob('*/case.json')};deferred={n for n,v in policy['deferred_by_user'].items() if v['corr_superb']};nptcases={n for n in deferred if not policy['deferred_by_user'][n]['npt']}
paired=mod.audit_lane('paired','native-full',True,inventory,deferred)
npt=mod.audit_lane('npt-only','native-npt-only',False,nptcases,set())
assert (paired['passed'],paired['keys'],paired['values'])==(30,925,4197)
assert (npt['passed'],npt['keys'],npt['values'])==(3,6,96)
for result in (paired,npt):
 for row in result['cases']:
  assert row['npt_unevaluable_graphs']==row['prior_npt_unevaluable_graphs'],row['case']

previous=r/'evidence/chroma-storage-20261109-001';same={'paired_npt':0,'paired_corr':0,'npt_only':0}; database_hashes={}
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for lane,rows in [('paired',paired['cases']),('npt-only',npt['cases'])]:
 for row in rows:
  folder=Path(lane)/'suite/no_1pt'/row['case']
  for filename in (('output.sdb1','superb.sdb1') if lane=='paired' else ('output.sdb1',)):
   database_hashes[str(folder/filename)]={'current':digest(e/folder/filename),'previous':digest(previous/folder/filename)}
  label='paired_npt' if lane=='paired' else 'npt_only'
  same[label]+=digest(e/folder/'output.sdb1')==digest(previous/folder/'output.sdb1')
  if lane=='paired':same['paired_corr']+=digest(e/folder/'superb.sdb1')==digest(previous/folder/'superb.sdb1')
assert same=={'paired_npt':30,'paired_corr':30,'npt_only':3},same
(e/'devel-after.log').write_text(mod.git_snapshot())
assert (e/'devel-before.log').read_bytes()==(e/'devel-after.log').read_bytes()
for wrapper in json.loads((e/'graph-executors.json').read_text()):
 assert digest(Path(wrapper['wrapper']))==wrapper['wrapper_sha256'];assert digest(Path(wrapper['executable']))==wrapper['executable_sha256']
result={'status':'passed','paired':paired,'npt_only':npt,'retained_deferrals':policy['deferred_by_user'],'policy_sha256':digest(kanoni/'docs/regression-policy.json'),'versus_previous_reorg_byte_identical':same,'resource_configuration':{'SB_CACHEGB_CPU':'1','max_rss_mb':8192,'tensor_gb':2},'devel_snapshot_unchanged':True,'database_sha256':database_hashes}
(e/'independent-audit.json').write_text(json.dumps(result,indent=2)+'\n')
print('Independent full readback passed; all 63 output DBs byte-identical to previous reorg checkpoint.')
