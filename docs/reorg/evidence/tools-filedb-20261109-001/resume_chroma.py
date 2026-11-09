from pathlib import Path
import subprocess,json,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';env=json.loads((e/'environments.json').read_text())['chroma'];records=[]
for step,args in [('build',['make','-j3']),('install',['make','install'])]:
 print('chroma-resumed-'+step,flush=True);cmd=['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),*args];start=time.monotonic()
 with (e/('chroma-resumed-'+step+'.log')).open('w') as log:p=subprocess.run(cmd,cwd=e/'build/chroma',env=env,stdout=log,stderr=subprocess.STDOUT)
 records.append(dict(step=step,executed_command=cmd,returncode=p.returncode,seconds=time.monotonic()-start));(e/'chroma-resumed-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
(e/'chroma-build.exit').write_text('0\n');print('Chroma resumed build/install passed.',flush=True)
