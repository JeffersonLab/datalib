import json
from pathlib import Path
import shutil
import subprocess
import sys
import time
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
while not (e/'build.exit').exists() or not (e/'owned-array-verification.json').exists():
    try:
        rows=json.loads((e/'build.json').read_text())
        assert all(row['returncode']==0 for row in rows),'A build failed; preserve evidence before continuing.'
    except json.JSONDecodeError:pass
    time.sleep(5)
for name in ('prepare.py','build.py','arrays_probe.cc','verify_owned.py','setup_full.py','verify_stack.py','finish.py'):
    shutil.copy2(Path(__file__).parent/name,e/name)
phases=[]
for name in ('verify_stack.py','check_api.py','check_aliases.py','openmp_check.py','run_full.py','audit.py','report.py'):
    print('Running '+name,flush=True)
    with (e/(name[:-3]+'-driver.log')).open('w') as log:
        status=subprocess.run([sys.executable,str(e/name)],stdout=log,stderr=subprocess.STDOUT)
    phases.append({'step':name,'returncode':status.returncode})
    (e/'phases.json').write_text(json.dumps(phases,indent=2)+'\n')
    status.check_returncode()
(e/'finish.exit').write_text('0\n')
print('Owned-array builds, complete native-double regressions, independent audit and report finished.',flush=True)
