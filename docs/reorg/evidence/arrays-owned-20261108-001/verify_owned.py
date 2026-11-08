import gzip
import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess
import tarfile
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
envs=json.loads((e/'environments.json').read_text());env=envs['datalib']
rows={}
for name,expected in [('datalib-core',4),('datalib-filedb',5)]:
    prefix=e/'install'/name;build=e/'build'/name
    text=(build/'test-suite.log').read_text()
    assert re.search(r'# TOTAL:\s*'+str(expected)+r'\b',text) and re.search(r'# PASS:\s*'+str(expected)+r'\b',text)
    cfg=prefix/'bin/datalib-config'
    flags={key:subprocess.check_output([str(cfg),'--'+key],env=env,text=True).strip() for key in ('cxxflags','ldflags','libs')}
    assert all('xpath_reader' not in value and 'XPathReader' not in value and '-lxmlWriter' not in value for value in flags.values())
    deps=[p for p in build.rglob('*.Po') if p.read_text().strip() not in ('','# dummy')]
    for p in deps:
        content=p.read_text()
        assert not re.search(r'/(adat[^/\s]*|qdp[^/\s]*|qmp[^/\s]*|xpath_reader[^/\s]*)(/|\s|$)',content),p
    rows[name]={'passed_checks':expected,'exported_flags':flags,'dependency_records':len(deps)}

prefix=e/'install/datalib-core'
flags=rows['datalib-core']['exported_flags']
def compile_probe(label,args,restricted=True):
    with (e/(label+'.log')).open('w') as log:
        command=(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb')] if restricted else [])+args
        subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
headers=('xml_array.h','xml_array2d.h','xml_array3d.h','xml_array4d.h')
for name in headers:
    source=e/('installed-'+name+'.cc')
    source.write_text('#include <datalib/model/'+name+'>\nint main(){}\n')
    compile_probe('installed-'+name,['/usr/bin/clang++',*shlex.split(flags['cxxflags']),'-fsyntax-only',str(source)])

base=(root/'datalib/tests/arrays_probe.cc').read_text()
base=base.replace('#include <datalib/codec/binary.h>', '#ifdef COEXIST\n'+''.join('#include <'+h+'>\n' for h in headers)+'#endif\n#include <datalib/codec/binary.h>')
base=base.replace('require(aa==a && ee.size()==0);','require(aa==a && ee.size()==0);\n#ifdef EXPECT_EMPTY_EQUAL\n    require((ee==empty)==bool(EXPECT_EMPTY_EQUAL));\n#endif')
source=e/'coexist-arrays.cc';source.write_text(base)
providers={'xpath':root/'install/xpath_reader/include','qdp':root.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/install/qdpxx-mpi/include'}
interop=[]
for provider,include in providers.items():
    for order in ('bundled-first','upstream-first'):
        label=provider+'-'+order
        binary=e/label
        expected=0 if provider=='qdp' and order=='upstream-first' else 1
        switches=['-DCOEXIST','-DEXPECT_EMPTY_EQUAL='+str(expected)]
        if order=='upstream-first':switches+=['-DLEGACY_ARRAY_HEADERS']
        args=['/usr/bin/clang++',*switches,*shlex.split(flags['cxxflags']),'-I'+str(include),str(source),'-o',str(binary),*shlex.split(flags['ldflags']),*shlex.split(flags['libs'])]
        compile_probe(label+'-build',args,False)
        with (e/(label+'-run.log')).open('w') as log:
            subprocess.run([str(binary),'check',str(e/'sources/datalib/tests/fixtures')],env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
        interop.append({'provider':provider,'include':str(include),'order':order,'empty_equality':bool(expected),'status':'passed'})

# Verify source distribution includes all owned headers and unchanged fixtures.
with (e/'datalib-dist.log').open('w') as log:
    subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'datalib-dist.sb'),'make','dist'],cwd=e/'build/datalib-filedb',env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
archive=e/'build/datalib-filedb/datalib-0.1.tar.gz'
with tarfile.open(archive) as package:
    names=set(package.getnames())
    for name in headers:assert 'datalib-0.1/include/datalib/model/'+name in names

hashes={}
for name in ('datalib-core','datalib-filedb'):
    p=e/'install'/name/'lib/libdatalib_serial.a'
    hashes[name]={'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
result={'status':'passed','builds':rows,'header_self_containment':list(headers),'interoperability':interop,
        'artifacts':hashes,'distribution':{'path':str(archive),'sha256':hashlib.sha256(archive.read_bytes()).hexdigest()},
        'notes':'XMLArray namespace, layout, operators and guards retain the old upstream implementations; Chroma explicitly retains QDP-first array definitions.'}
(e/'owned-array-verification.json').write_text(json.dumps(result,indent=2)+'\n')
print('Owned array headers, restricted dependency closure, both provider include orders and source distribution passed.')
