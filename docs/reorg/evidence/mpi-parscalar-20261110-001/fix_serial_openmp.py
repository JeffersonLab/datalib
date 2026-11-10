from pathlib import Path
import json,subprocess,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';records=json.loads((e/'build.json').read_text());assert any(c['step']=='superbblas-serial-build-install' and c['returncode']==0 for c in records)
assert 'datalib-bootstrap' not in Path('/tmp/reorg-mpi/build-driver.log').read_text(), 'Dependent datalib build already started'
oldbuild=e/'build/superbblas-serial';oldprefix=e/'install/superbblas-serial';oldbuild.rename(e/'build/superbblas-serial-initial-no-openmp');oldprefix.rename(e/'install/superbblas-serial-initial-no-openmp')
for label in ('superbblas-serial-generate','superbblas-serial-build-install'):
 (e/(label+'.log')).rename(e/(label+'-initial-no-openmp.log'))
build=e/'build/superbblas-serial';build.mkdir();env=json.loads((e/'environments.json').read_text())['serial'];flags=env['CXXFLAGS']+' '+env['OPENMP_CXXFLAGS']+' -DSUPERBBLAS_USE_CBLAS';args=['TARGET=cpu','BUILDDIR='+str(build),'PREFIX='+str(e/'install/superbblas-serial'),'LIBRARY='+str(build/'libsuperbblas.a'),'CXX='+env['CXX'],'CXXFLAGS='+flags,'SUPERBBLAS_WITH_MPI=no','INSTALL_LINK_SOURCE=no','PYTHON=/opt/homebrew/bin/python3'];source=e/'sources/superbblas/src';rows=[]
def run(label,cmd):
 start=time.monotonic()
 with (e/(label+'.log')).open('w') as log:p=subprocess.run(cmd,cwd=source,env=env,stdout=log,stderr=subprocess.STDOUT)
 rows.append({'step':label,'command':cmd,'returncode':p.returncode,'seconds':time.monotonic()-start});(e/'serial-openmp-build.json').write_text(json.dumps(rows,indent=2)+'\n');p.check_returncode()
run('superbblas-serial-generate',['make',str(build/'superbblas_lib'),*args]);lines=(build/'superbblas_lib').read_text().splitlines(True);chunks=[]
for i in range(10):
 part=''.join(lines[len(lines)*i//10:len(lines)*(i+1)//10]);chunks.append(part);(build/f'superbblas_lib_{i}.cpp').write_text('#include "superbblas.h"\nusing namespace superbblas; using namespace superbblas::detail;\n'+part)
assert ''.join(chunks)==(build/'superbblas_lib').read_text();(build/'superbblas_lib.split').touch()
run('superbblas-serial-build-install',['make','install','-j2',*args])
(e/'serial-openmp-correction.json').write_text(json.dumps({'status':'passed','initial_build_preserved':True,'fresh_build_and_install_with_openmp':True,'reason':'Build-driver serial SuperbBLAS flags initially omitted OPENMP_CXXFLAGS. Corrected before datalib configuration, without overlays.'},indent=2)+'\n');print('Fresh serial SuperbBLAS rebuilt with OpenMP flags before dependent configuration.')
