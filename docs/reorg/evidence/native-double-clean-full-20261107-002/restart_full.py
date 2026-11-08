import json
from pathlib import Path
import shutil
import subprocess
import sys
import time
root=Path.home()/'Downloads/macos-datalib-reorg'
old=root/'evidence/native-double-clean-full-20261107-001'
while not (old/'retry-cache1/exit').exists():time.sleep(5)
assert (old/'retry-cache1/exit').read_text().strip()=='0','Cache-bounded targeted retry did not pass'
target=json.loads((old/'retry-cache1/suite/results.json').read_text())
assert target['status']=='passed'
e=root/'evidence/native-double-clean-full-20261107-002'
e.mkdir()
for name in ['environment.json','protected-before.json','reorg-before.json','stack-sources.json','reorg-sources.log',
             'build.json','direct-api-check.json','openmp-check.json','datalib-documentation-push.json']:
    shutil.copy2(old/name,e/name)
env=json.loads((e/'environment.json').read_text());env['SB_CACHEGB_CPU']='1'
(e/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
resource={'failed_full_attempt':str(old/'paired/suite/results.json'),
          'failed_case':'nucnuc_small','error':'memory limit exceeded (8192 MiB; observed 8403888 KiB RSS)',
          'targeted_retry':str(old/'retry-cache1/suite/results.json'),
          'SB_CACHEGB_CPU':'1','max_rss_mb':8192,'tensor_gb':2,
          'change_scope':'Reusable CPU allocation cache only; unchanged physics, numerical gate, deferrals and executables.'}
(e/'resource-retry.json').write_text(json.dumps(resource,indent=2)+'\n')
for name in ('redstar-dynamic-libraries.log','chroma-dynamic-libraries.log'):
    shutil.copy2(old/name,e/name)
shutil.copy2(old/'export_correlators.cc',e/'export_correlators.cc')
for name in ('run_full.py','audit.py','report.py'):
    source=(old/name).read_text()
    # Update only the current evidence root; existing build-source paths stay in
    # their explicit records. The same freshly built executables are reused.
    if name=='run_full.py':
        source=source.replace("e=root/'evidence/native-double-clean-full-20261107-001'", "e=root/'evidence/native-double-clean-full-20261107-002'\nstack=root/'evidence/native-double-clean-full-20261107-001'")
        source=source.replace("adat=e/'install/adat'", "adat=stack/'install/adat'").replace("redstar=e/'install/redstar/bin'", "redstar=stack/'install/redstar/bin'")
        source=source.replace("e/'install/adat/lib/libadat.a',e/'install/colorvec/lib/libcolorvec.a',e/'install/redstar/lib/libredstar.a'", "stack/'install/adat/lib/libadat.a',stack/'install/colorvec/lib/libcolorvec.a',stack/'install/redstar/lib/libredstar.a'")
        source=source.replace("'--qualification','Clean-source", "'--qualification','SB_CACHEGB_CPU=1, unchanged 8192 MiB RSS cap and 2 GiB planner after retaining a resource-limited first attempt. Clean-source")
    elif name=='audit.py':
        source=source.replace('evidence = reorg / "evidence/native-double-clean-full-20261107-001"','evidence = reorg / "evidence/native-double-clean-full-20261107-002"')
        source=source.replace('    policy = load(kanoni / "docs/regression-policy.json")','    resource = load(evidence / "resource-retry.json")\n    assert resource["SB_CACHEGB_CPU"] == "1" and resource["max_rss_mb"] == 8192 and resource["tensor_gb"] == 2\n    assert load(Path(resource["targeted_retry"]))["status"] == "passed"\n    assert load(Path(resource["failed_full_attempt"]))["status"] == "attention_required"\n    policy = load(kanoni / "docs/regression-policy.json")')
        source=source.replace('result = {"direct_api": direct_api,', 'result = {"resource_retry": resource, "direct_api": direct_api,')
    else:
        source=source.replace('relative = Path("evidence/native-double-clean-full-20261107-001")','relative = Path("evidence/native-double-clean-full-20261107-002")')
        insert='''## Resource configuration and retained first attempt

The first full attempt passed 29 paired cases and all three NPT-only cases,
but `nucnuc_small` was terminated by the unchanged 8,192 MiB RSS guard at
8,403,888 KiB (8,206.9 MiB). Its results and incomplete Chroma output remain
in `evidence/native-double-clean-full-20261107-001/`; that attempt is not
labelled successful. The supported SuperbBLAS reusable CPU allocation cache
normally defaults to 10% of system RAM, separately from the 2 GiB contraction
planner budget.

A fresh targeted retry passed with `SB_CACHEGB_CPU=1`. The entire paired
matrix and NPT-only lane were then rerun in fresh directories with that same
1 GiB reusable-cache cap. The 8 GiB RSS guard, 2 GiB tensor planner, physics,
thread/rank counts, executables, numerical gate and deferrals were unchanged.
No source fix or larger process-memory allowance was used. The successful
full run is under `evidence/native-double-clean-full-20261107-002/`.
See [resource retry provenance](evidence/native-double-clean-full-20261107-002/resource-retry.json).

'''
        source=source.replace('## Paired results',insert+'## Paired results')
    (e/name).write_text(source)
shutil.copy2(Path(__file__),e/'restart_full.py')
records=[]
for name in ('run_full.py','audit.py','report.py'):
    print('Running bounded-cache '+name,flush=True)
    with (e/(name[:-3]+'-driver.log')).open('w') as log:
        status=subprocess.run([sys.executable,str(e/name)],stdout=log,stderr=subprocess.STDOUT)
    records.append({'step':name,'returncode':status.returncode})
    (e/'phases.json').write_text(json.dumps(records,indent=2)+'\n')
    status.check_returncode()
(e/'finish.exit').write_text('0\n')
print('Full bounded-cache sweep, independent audit and report completed.',flush=True)
