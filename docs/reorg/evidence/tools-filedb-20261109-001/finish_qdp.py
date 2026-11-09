from pathlib import Path
import subprocess,json,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';ref=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002';env=json.loads((r/'evidence/arrays-owned-20261108-001/environments.json').read_text())['chroma'];env['CXX']='/opt/homebrew/bin/mpicxx'
# Reproduce an unchanged upstream SIMD-test failure against the old installation.
qcfg=ref/'install/qdpxx-mpi/bin/qdp++-config';flags=shlex.split(subprocess.check_output([str(qcfg),'--cxxflags'],text=True))
src=ref/'sources/qdpxx/tests/testVaxpyDouble.cc';cmd=['/opt/homebrew/bin/mpicxx',*flags,'-I'+str(src.parent),'-fsyntax-only',str(src)]
with (e/'qdp-baseline-simd-check.log').open('w') as log:p=subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT)
assert p.returncode!=0
s=(e/'qdp-baseline-simd-check.log').read_text();assert 'vaxpy4' in s or 'sse_mult_su3_nn.h' in s
(e/'qdp-check-limitation.json').write_text(json.dumps({'full_make_check':'failed','cause':'Unchanged upstream x86 SIMD tests are incompatible with this ARM/parscalar build','baseline_command':cmd,'baseline_returncode':p.returncode,'baseline_same_simd_failure':True,'action':'Retained failure; no tests disabled or acceptance gates changed'},indent=2)+'\n')
with (e/'qdp-install.log').open('w') as log:subprocess.run(['make','install'],cwd=e/'build/qdpxx-mpi',env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
prefix=e/'install/qdpxx-mpi';libs=subprocess.check_output([str(prefix/'bin/qdp++-config'),'--libs'],text=True)
assert '-lfiledb' not in libs and '-lfilehash' not in libs
assert not list((prefix/'include').glob('qdp_db*.h'));assert not list((prefix/'lib').glob('*filedb*'));assert not list((prefix/'lib').glob('*filehash*'))
(e/'qdp-install.exit').write_text('0\n');print('QDP install verified without FILEDB; full-check SIMD limitation reproduced on baseline.')
