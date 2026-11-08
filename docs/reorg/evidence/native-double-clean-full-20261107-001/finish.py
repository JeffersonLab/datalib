import json
from pathlib import Path
import shutil
import subprocess
import sys
import time
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/native-double-clean-full-20261107-001'
while not (e/'build.exit').exists():
    try:
        records=json.loads((e/'build.json').read_text())
        if records and records[-1]['returncode']!=0:raise RuntimeError('Downstream build failed; preserving evidence.')
    except json.JSONDecodeError:pass
    time.sleep(5)
assert (e/'build.exit').read_text().strip()=='0'
for name in ('build_stack.py','prepare_checks.py','run_full.py','openmp_check.py','audit.py','report.py','finish.py'):
    shutil.copy2(Path(__file__).parent/name,e/name)
records=[]
for name in ('prepare_checks.py','check_api.py','check_aliases.py','openmp_check.py','run_full.py','audit.py','report.py'):
    print('Running '+name,flush=True)
    with (e/(name[:-3]+'-driver.log')).open('w') as log:
        status=subprocess.run([sys.executable,str(e/name)],stdout=log,stderr=subprocess.STDOUT)
    records.append({'step':name,'returncode':status.returncode})
    (e/'phases.json').write_text(json.dumps(records,indent=2)+'\n')
    status.check_returncode()
(e/'finish.exit').write_text('0\n')
print('Full clean-stack regression, audit and report completed.',flush=True)
