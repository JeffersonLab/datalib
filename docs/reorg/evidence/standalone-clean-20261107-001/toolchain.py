import json
from pathlib import Path
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
env=json.loads((e/'environments.json').read_text())['chroma']
commands=[['/usr/bin/clang++','--version'],['autoreconf','--version'],['automake','--version'],
          ['/opt/homebrew/opt/libxml2/bin/xml2-config','--version'],['/opt/homebrew/bin/mpicxx','--showme:command']]
with (e/'toolchain.log').open('w') as log:
    for command in commands:
        log.write('Command: '+repr(command)+'\n');log.flush()
        subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
print('Recorded compiler, Autotools, libxml2 and MPI wrapper versions.')
