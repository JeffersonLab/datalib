import hashlib
import json
from pathlib import Path
import re
import subprocess

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
environments=json.loads((e/'environments.json').read_text())
adat=re.compile(r'/(?:adat[^/\s]*|libadat[^/\s]*)(?:/|\s|$)')
qdp=re.compile(r'/(?:qdp[^/\s]*|qmp[^/\s]*|libqdp[^/\s]*|libqmp[^/\s]*)(?:/|\s|$)')
symbols=re.compile(r'\b(?:ADAT|ADATIO|ADATXML|ADATUtil|ENSEM)::')
def sha(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b''): h.update(chunk)
    return h.hexdigest()
def git(repo,*args):
    return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)
results={'status':'passed','builds':{},'artifacts':{}}
records=json.loads((e/'build.json').read_text())
assert len(records)==13,len(records)
assert all(row['returncode']==0 for row in records)
for label,expected in [('datalib-core',3),('datalib-filedb',4),('chroma',None)]:
    build=e/'build'/label
    files=[p for p in sorted(build.rglob('*.Po')) if p.read_text().strip() not in ('','# dummy')]
    assert files,label
    forbidden=[]
    for dep in files:
        text=dep.read_text()
        if adat.search(text) or (label!='chroma' and qdp.search(text)):
            forbidden.append(str(dep.relative_to(e)))
    assert not forbidden,(label,forbidden)
    exported={}
    config=e/'install'/label/'bin'/('chroma-config' if label=='chroma' else 'datalib-config')
    options=['cxxflags','ldflags','libs']
    if label=='datalib-filedb':options+=['filedb-cxxflags','filedb-ldflags','filedb-libs']
    for option in options:
        text=subprocess.check_output([str(config),'--'+option],env=environments['chroma' if label=='chroma' else 'datalib'],text=True)
        assert not adat.search(text),text
        assert not re.search(r'-l(?:adat|Minuit2Base)\b',text),text
        if label!='chroma':
            assert not qdp.search(text),text
            assert not re.search(r'-l(?:qdp|qmp|mpi|XPathReader|xmlWriter)\b',text),text
        exported[option]=text.strip()
    item={'compiler_dependency_files':len(files),'forbidden_dependency_paths':forbidden,'exported_flags':exported}
    if expected:
        suite=(build/'test-suite.log').read_text()
        assert re.search(r'# TOTAL:\s*'+str(expected)+r'\b',suite),label
        assert re.search(r'# PASS:\s*'+str(expected)+r'\b',suite),label
        assert re.search(r'# FAIL:\s*0\b',suite),label
        item['passed_checks']=expected
    results['builds'][label]=item

for label,artifact in [('datalib-core',e/'install/datalib-core/lib/libdatalib_serial.a'),
                       ('datalib-filedb',e/'install/datalib-filedb/lib/libdatalib_serial.a'),
                       ('chroma',e/'install/chroma/bin/chroma'),('readback',e/'readback')]:
    output=subprocess.check_output(['/usr/bin/nm','-C',str(artifact)],stderr=subprocess.STDOUT,text=True)
    forbidden=[line for line in output.splitlines() if symbols.search(line)]
    assert not forbidden,(label,forbidden[:10])
    dynamic=subprocess.check_output(['/usr/bin/otool','-L',str(artifact)],stderr=subprocess.STDOUT,text=True)
    (e/(label+'-dynamic-libraries.log')).write_text(dynamic)
    assert 'libgomp' not in dynamic and 'libstdc++' not in dynamic and not adat.search(dynamic),label
    if label=='chroma': assert 'libomp' in dynamic
    if label=='readback':
        assert not qdp.search(dynamic) and 'libmpi' not in dynamic
    results['artifacts'][label]={'path':str(artifact),'sha256':sha(artifact),'forbidden_symbols':forbidden}

build_log=(e/'chroma-build.log').read_text()
links=[line for line in build_log.splitlines() if re.search(r' -o chroma(?:\s|$)',line)]
assert len(links)==1,len(links)
link=links[0]
assert '-ldatalib_serial' in link
assert str(e/'install/datalib-filedb/lib') in link
assert '-ladat' not in link and '-lMinuit2Base' not in link
assert not adat.search(link)
(e/'chroma-link.log').write_text(link+'\n')
results['chroma_links_fresh_datalib']=True
config_header=e/'build/chroma/lib/chroma_config_internal.h'
configuration=config_header.read_text()
assert re.search(r'^#define BUILD_REDSTAR_DATALIB\b',configuration,re.M)
results['corr_superb_enabled']=True
for name in ('datalib','chroma'):
    repo=e/'sources'/name
    snapshot=json.loads((e/(name+'-clean-checkout.json')).read_text())
    assert not snapshot['status_before_bootstrap']
    assert git(repo,'rev-parse','HEAD').strip()==snapshot['head']
    assert git(repo,'branch','--show-current').strip()=='rge/devel-datalib-reorg'
    paths=['src','include','tests'] if name=='datalib' else ['lib','tests','mainprogs']
    assert not git(repo,'diff','HEAD','--',*paths),name
    snapshot['after_bootstrap_status']=git(repo,'status','--short')
    snapshot['no_source_overlays']=True
    results[name+'_source']=snapshot
    if name=='chroma':
        submodules=[]
        for line in git(repo,'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$').splitlines():
            _,path=line.split(None,1)
            expected=git(repo,'ls-tree','HEAD',path).split()[2]
            subrepo=repo/path
            assert git(subrepo,'rev-parse','HEAD').strip()==expected,path
            changes=git(subrepo,'diff','HEAD','--','*.c','*.cc','*.cpp','*.h','*.f')
            assert not changes,path
            submodules.append({'path':path,'commit':expected,'no_source_overlays':True})
        results['chroma_submodules']=submodules

protected=json.loads((e/'protected-before.json').read_text())
for name,expected in protected.items():
    if name.startswith('reorg/'):
        repo=root/name.split('/')[1]
        assert git(repo,'rev-parse','HEAD')==expected['head'],name
        assert git(repo,'status','--short')==expected['status'],name
        assert git(repo,'for-each-ref','--format=%(objectname)','refs/heads/devel')==expected['local_devel'],name
    else:
        repo=root.parent/name
        assert git(repo,'rev-parse','refs/heads/devel')==expected['devel'],name
        if 'status' in expected:
            assert git(repo,'status','--porcelain=v1','--branch')==expected['status'],name
            assert hashlib.sha256(git(repo,'diff','--binary','HEAD').encode()).hexdigest()==expected['diff_sha256'],name
results['original_reorg_devel_and_kanoni_unchanged']=True
smoke=json.loads((e/'smoke.json').read_text())
assert smoke['status']=='passed'
assert len(smoke['cases'])==3
assert all(row['byte_identical'] and row['failures']==0 and row['CG_solves']==0 for row in smoke['cases'])
assert sum(row['keys'] for row in smoke['cases'])==7
assert sum(row['values'] for row in smoke['cases'])==103
results['smoke']=smoke
canaries=json.loads((e/'sandbox-canaries.json').read_text())
assert len(canaries)==4
assert all((row['returncode']==0)==row['allowed'] for row in canaries)
results['sandbox_canaries']=canaries
results['openmp']=json.loads((e/'openmp.json').read_text())
assert results['openmp']==[{'requested':1,'observed':1},{'requested':4,'observed':4}]
(e/'verification.json').write_text(json.dumps(results,indent=2)+'\n')
print('Clean build dependency, symbol, link, runtime and protected-state audits passed.')
for label,row in results['builds'].items():print(label,row['compiler_dependency_files'],'compiler dependency records')
