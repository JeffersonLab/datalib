from pathlib import Path
import json,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';old=r/'evidence/dbtools-extract-20261109-001'
env=json.loads((e/'environments.json').read_text())['mpi'];env['OMP_NUM_THREADS']='2';env['PYTHONDONTWRITEBYTECODE']='1';(e/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
shutil.copy2(old/'export_correlators.cc',e/'export_correlators.cc')
s=(old/'run_full.py').read_text();s=s.replace("e=root/'evidence/dbtools-extract-20261109-001'", "e=root/'evidence/mpi-parscalar-20261110-001'")
s=s.replace('stack=e',"stack=root/'evidence/dbtools-extract-20261109-001'")
s=s.replace("e/'install/datalib-filedb", "e/'install/datalib")
s=s.replace("shlex.join(['/usr/bin/sandbox-exec','-f',str(profile),str(real_chroma)])", "shlex.join(['/opt/homebrew/bin/mpirun','-n','2','/usr/bin/sandbox-exec','-f',str(profile),str(real_chroma)])")
start=s.index("          '--qualification',");end=s.index(',*extra]',start)
s=s[:start]+"          '--qualification','Fresh MPI QMP, parscalar double QDP/OpenMP, serial and MPI SuperbBLAS, standalone datalib and ADAT-free Chroma from matching reorg branches. Chroma executes with two MPI ranks and two OpenMP threads per rank; NPT reference remains the previously validated single-process OpenMP implementation. Cached native-double tensors and explicit DISCO supplement; zero CG solves; original deferrals, 8192 MiB process-group RSS guard, 2 GiB planner and SB_CACHEGB_CPU=1 unchanged. Legacy QDP SIMD unit-program compile failure is preserved separately and does not establish an MPI runtime failure.'"+s[end:]
(e/'run_full_mpi.py').write_text(s)
# Retain the complete before/after publication record separately from MPI validation.
shutil.copy2(r/'evidence/dbtools-publish-20261110-001/publication.json',e/'dbtools-publication.json')
