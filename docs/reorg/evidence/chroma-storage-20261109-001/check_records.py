from pathlib import Path
import subprocess,json,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';cfg=r/'evidence/tools-filedb-20261109-001/install/datalib-filedb/bin/datalib-config'
def flags(n):return shlex.split(subprocess.check_output([str(cfg),'--'+n],text=True))
cmd=['/usr/bin/clang++','-I'+str(r/'datalib/include'),*flags('cxxflags'),*flags('filedb-cxxflags'),r/'datalib/tests/records_probe.cc','-o',e/'records-probe',*flags('ldflags'),*flags('filedb-ldflags'),*flags('filedb-libs')];cmd=list(map(str,cmd))
with (e/'records-probe-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True)
cmd=[str(e/'records-probe'),str(r/'datalib/tests/fixtures'),str(e/'records-probe.sdb')]
with (e/'records-probe-run.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,check=True)
print('records checks passed')
