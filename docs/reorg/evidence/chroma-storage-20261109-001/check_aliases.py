import json
from pathlib import Path
import shlex
import subprocess

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/chroma-storage-20261109-001'
cfg=e/'install/adat/bin/adat-config'
flags=shlex.split(subprocess.check_output([cfg,'--cxxflags'],text=True))
checks={'ADAT::Array1dO<int>': 'array', 'ADAT::MapObject<int,int>': 'map',
        'ADATIO::BinaryReader': 'binary', 'ADATXML::XMLReader': 'xml',
        'ADATIO::SerialDBKey<int>': 'filedb'}
results={}
for alias,label in checks.items():
    source=e/('removed-alias-'+label+'.cc')
    source.write_text('#include <adat/handle.h>\n#include <ensem/ensem.h>\n#include <io/adat_xml_group_reader.h>\n#include <io/superb_kv.h>\n'+alias+'* legacy;\n')
    with (e/('removed-alias-'+label+'.log')).open('w') as out:
        status=subprocess.run(['/usr/bin/clang++',*flags,'-fsyntax-only',str(source)],stdout=out,stderr=subprocess.STDOUT)
    assert status.returncode!=0,alias
    results[alias]='rejected'
summary=json.loads((e/'direct-api-check.json').read_text())
summary['removed_legacy_aliases']=results
(e/'direct-api-check.json').write_text(json.dumps(summary,indent=2)+'\n')
print('Verified removal of legacy ADAT container and I/O aliases.')
