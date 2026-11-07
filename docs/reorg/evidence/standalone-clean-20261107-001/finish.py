import json
from pathlib import Path
import shutil
import subprocess
import sys
import time
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
while not (e/'build.exit').exists():
    try:
        records=json.loads((e/'build.json').read_text())
        if records and records[-1]['returncode']!=0:raise RuntimeError('Build failed; preserving evidence and stopping follow-up checks.')
    except json.JSONDecodeError: pass
    time.sleep(5)
assert (e/'build.exit').read_text().strip()=='0'
for name in ('prepare.py','build.py','smoke.py','readback.cc','audit.py','report.py','openmp.py','toolchain.py','finish.py'):
    shutil.copy2(Path(__file__).parent/name,e/name)
for name in ('smoke.py','audit.py','report.py'):
    with (e/(name[:-3]+'-driver.log')).open('w') as log:
        print('Running '+name,flush=True)
        subprocess.run([sys.executable,str(e/name)],stdout=log,stderr=subprocess.STDOUT,check=True)
(e/'finish.exit').write_text('0\n')
print('Clean builds, ADAT-free runtime checks and documentation completed.',flush=True)
