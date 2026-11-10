from pathlib import Path
import subprocess,json,time,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';ref=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002';envs=json.loads((e/'environments.json').read_text());records=[]
shutil.copy2(r/'evidence/tools-filedb-20261109-001/qdp-check.log',e/'qdpxx-previous-generic-check.log')
(e/'qdpxx-check-limitation.json').write_text(json.dumps({'status':'failed','scope':'Upstream architecture-specific SSE unit/benchmark programs on the generic Apple-arm64 parscalar build','reproduced_in_prior_reorg':True,'missing_helpers':['vaxpy4','vaxpyz4','vaxmy4'],'library_build_passed':True,'test_sources_unchanged':True,'not_an_MPI_runtime_result':True},indent=2)+'\n')
def run(label,cmd,cwd,kind,sandbox=None):
 cmd=list(map(str,cmd));cmd=(['/usr/bin/sandbox-exec','-f',str(e/(sandbox+'.sb'))]+cmd) if sandbox else cmd;start=time.monotonic();print(label,flush=True)
 with (e/(label+'.log')).open('w') as out:p=subprocess.run(cmd,cwd=cwd,env=envs[kind],stdout=out,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':cmd,'cwd':str(cwd),'environment':kind,'returncode':p.returncode,'seconds':time.monotonic()-start});(e/'continued-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
assert json.loads((e/'serial-openmp-correction.json').read_text())['status']=='passed'
run('qdpxx-install',['make','install'],e/'build/qdpxx','mpi')
assert subprocess.check_output([e/'install/qmp/bin/qmp-config','--comms'],text=True).strip()=='MPI'
for name,opts,kind,sandbox,check in [
 ('datalib',['--enable-openmp','--with-filedb=bundled','--with-superbblas='+str(e/'install/superbblas-serial'),'--with-libxml2=/opt/homebrew/opt/libxml2'],'serial','datalib',True),
 ('chroma',['--with-qdp='+str(e/'install/qdpxx'),'--with-superbblas='+str(e/'install/superbblas-mpi'),'--with-datalib='+str(e/'install/datalib'),'--with-primme='+str(ref/'sources/primme'),'--enable-mgproton','--disable-cpp-wilson-dslash','--enable-testcase-runner=trivial','--with-libxml2=/opt/homebrew/opt/libxml2','--with-gmp=/opt/homebrew/opt/gmp'],'mpi','chroma',False)]:
 source=e/'sources'/name;build=e/'build'/name;build.mkdir();prefix=e/'install'/name;assert not prefix.exists()
 run(name+'-bootstrap',['autoreconf','-fi'],source,kind,sandbox)
 run(name+'-configure',[source/'configure','--prefix='+str(prefix),*opts],build,kind,sandbox)
 run(name+'-build',['make','-j3'],build,kind,sandbox)
 if check:run(name+'-check',['make','check','-j3'],build,kind,sandbox)
 run(name+'-install',['make','install'],build,kind,sandbox)
(e/'build.exit').write_text('0\n');print('Fresh MPI stack built; QDP legacy architecture-specific test failure retained separately.',flush=True)
