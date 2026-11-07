import hashlib
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import xml.etree.ElementTree as ET

root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/standalone-clean-20261107-001'
environments=json.loads((e/'environments.json').read_text())
config=e/'install/datalib-core/bin/datalib-config'
def flags(option):
    return shlex.split(subprocess.check_output([str(config),'--'+option],env=environments['datalib'],text=True))
source=e/'readback.cc'
if (Path(__file__).parent/'readback.cc').resolve()!=source.resolve():
    shutil.copy2(Path(__file__).parent/'readback.cc',source)
probe=e/'readback'
compile_args=['/usr/bin/clang++',*flags('cxxflags'),str(source),'-o',str(probe),*flags('ldflags'),*flags('libs')]
with (e/'readback-build.log').open('w') as log:
    subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*compile_args],env=environments['datalib'],stdout=log,stderr=subprocess.STDOUT,check=True)
(e/'readback-command.json').write_text(json.dumps(compile_args,indent=2)+'\n')
def sha(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
    return h.hexdigest()
previous=root/'evidence/native-double-full-20261106-001/paired/suite/no_1pt'
prior=json.loads((previous.parent/'results.json').read_text())
provenance={**prior['data_provenance'],**prior['disco_provenance']}
assert all(sha(Path(path))==expected for path,expected in provenance.items())
(e/'data-provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
smoke=e/'smoke'
smoke.mkdir()
results=[]
for case,keys,times in [('pion',3,16),('3pi',3,16),('nuc_nuc_3pt',1,7)]:
    folder=smoke/case
    folder.mkdir()
    old=previous/case
    # Preserve all validated parameters and real absolute tensor paths.
    shutil.copy2(old/'chroma.xml',folder/'chroma.xml')
    shutil.copy2(old/'corr_graph.bin',folder/'corr_graph.bin')
    tree=ET.parse(folder/'chroma.xml')
    assert tree.findtext('Param/InlineMeasurements/elem/Param/average_1pt_diagrams')=='false'
    env={**environments['chroma'],'OMP_NUM_THREADS':'1'}
    args=[str(e/'install/chroma/bin/chroma'),'-i','chroma.xml','-o','chroma.out.xml']
    with (folder/'chroma.log').open('w') as log:
        run=subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),*args],cwd=folder,env=env,stdout=log,stderr=subprocess.STDOUT)
    assert run.returncode==0,(case,run.returncode)
    log=(folder/'chroma.log').read_text()
    assert 'CORR_SUPERB: ran successfully' in log
    assert 'CG_SOLVER_TIME:' not in log
    candidate=folder/'superb.sdb1'
    reference=old/'superb.sdb1'
    assert candidate.is_file()
    output=subprocess.check_output(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),str(probe),str(reference),str(candidate),str(keys),str(times)],env=environments['datalib'],text=True)
    readback=json.loads(output)
    assert readback['failures']==0 and readback['zero_channels']==0
    row={'case':case,'command':args,'returncode':run.returncode,'CG_solves':0,
         'graph_sha256':sha(folder/'corr_graph.bin'),'input_sha256':sha(folder/'chroma.xml'),
         'reference_sha256':sha(reference),'output_sha256':sha(candidate),
         'byte_identical':sha(reference)==sha(candidate),**readback}
    results.append(row)
    (folder/'readback.json').write_text(json.dumps(row,indent=2)+'\n')
    print(case,'passed',flush=True)
assert all(sha(Path(path))==expected for path,expected in provenance.items())
(e/'smoke.json').write_text(json.dumps({'status':'passed','cases':results},indent=2)+'\n')
print('ADAT-free Chroma runtime and datalib-only independent readback passed.',flush=True)
