import json
from pathlib import Path
import re
import shlex
import subprocess

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/chroma-storage-20261109-001'
removed=json.loads((e/'changes.json').read_text())['removed_headers']
prefix=e/'install/adat'
for name in removed:
    assert not (root/'adat'/name).exists(),name
    assert not (prefix/'include'/Path(name).relative_to('lib')).exists(),name
config=prefix/'bin/adat-config'
def flags(name):
    return shlex.split(subprocess.check_output([config,'--'+name],text=True))
source=e/'direct-api-probe.cc'
source.write_bytes((Path(__file__).parent/'probe.cc').read_bytes())
args=['/usr/bin/clang++',*flags('cxxflags'),source,'-o',e/'direct-api-probe',*flags('ldflags'),*flags('libs')]
with (e/'direct-api-compile.log').open('w') as stream:
    subprocess.run(list(map(str,args)),stdout=stream,stderr=subprocess.STDOUT,check=True)
result=subprocess.check_output([e/'direct-api-probe',root/'datalib/tests/fixtures/graph-v4.bin'],text=True)
(e/'direct-api-run.log').write_text(result)
print(result,end='')
source.write_text(source.read_text()+'\n'+''.join(
 '#if __has_include(<'+str(Path(name).relative_to('lib'))+'>)\n#error Compatibility header is still available\n#endif\n'
 for name in removed))
with (e/'compatibility-absence-compile.log').open('w') as stream:
    subprocess.run(list(map(str,args)),stdout=stream,stderr=subprocess.STDOUT,check=True)

old_paths={str(Path(name).relative_to('lib')) for name in removed}
old_names={Path(name).name for name in removed}
pattern=re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]',re.M)
errors=[]
for repo in ('adat','colorvec','redstar'):
    base=root/repo
    for name in subprocess.check_output(['git','-C',str(base),'ls-files','-z'],text=True).split('\0'):
        path=base/name
        if not name or not path.is_file() or path.suffix not in ('.h','.cc','.cpp','.hpp') or name.startswith('other_libs/'):
            continue
        text=path.read_text()
        for include in pattern.findall(text):
            if include in old_paths or (include in old_names and not (path.parent/include).exists() and '/' not in include):
                errors.append(repo+'/'+name+': '+include)
        if re.search(r'using\s+namespace\s+Datalib(?:::|\s*;)',text):
            errors.append(repo+'/'+name+': datalib namespace import')
assert not errors,errors
summary=dict(status='passed',removed_headers=len(removed),removed_from_source=True,
             removed_from_fresh_install=True,no_old_include_paths=True,
             no_datalib_namespace_imports=True,qualified_api_probe=True)
(e/'direct-api-check.json').write_text(json.dumps(summary,indent=2)+'\n')
print('Verified all 37 compatibility paths are absent from the fresh install.')
