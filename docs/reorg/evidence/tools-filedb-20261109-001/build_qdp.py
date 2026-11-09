from pathlib import Path
import subprocess,json,shlex,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';q=r/'qdpxx';old=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002'
p=q/'FindQDPXX.cmake.in';s=p.read_text();start=s.index('# Check if using FILEDB');end=s.index('# Add other LFlags',start);p.write_text(s[:start]+s[end:])
# Commit canonical source changes; generated files in this upstream repo are left alone.
paths=['.gitmodules','CMakeLists.txt','configure.ac','examples/Makefile.am','examples/t_db.cc','include/CMakeLists.txt','include/Makefile.am','include/qdp_config_internal.h.cmake.in','include/qdp_db.h','include/qdp_db_imp.h','include/qdp_db_stub.h','lib/CMakeLists.txt','other_libs/Makefile.am','other_libs/filedb','qdp++-config.in','FindQDPXX.cmake.in']
# Deletions already in the index are preserved; git add -u covers those.
subprocess.run(['git','-C',str(q),'add','-u'],check=True)
subprocess.run(['git','-C',str(q),'diff','--cached','--check'],check=True)
subprocess.run(['git','-C',str(q),'commit','-m','Remove FILEDB ownership and database wrappers from QDP++'],check=True)
source=e/'sources/qdpxx';source.parent.mkdir(exist_ok=True);subprocess.run(['git','clone','--no-local','--single-branch','--branch','rge/devel-datalib-reorg',str(q),str(source)],check=True)
for line in subprocess.check_output(['git','-C',str(source),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
 key,path=line.split(None,1);subprocess.run(['git','-C',str(source),'config',key[:-4]+'url',str(q/path)],check=True)
subprocess.run(['git','-C',str(source),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],check=True)
env=json.loads((r/'evidence/arrays-owned-20261108-001/environments.json').read_text())['chroma'];env['CXX']='/opt/homebrew/bin/mpicxx'
# Use precisely the previous parallel-architecture and dependency choices.
args=shlex.split(subprocess.check_output(['/bin/sh',str(old/'builds/qdpxx-mpi/config.status'),'--config'],text=True));args=[arg for arg in args if not arg.startswith('--prefix=') and not arg.startswith(('CXX=','CXXFLAGS=','LDFLAGS=','LIBS=','CPPFLAGS='))]
b=e/'build/qdpxx-mpi';b.mkdir(parents=True);prefix=e/'install/qdpxx-mpi';records=[]
for label,cmd,cwd in [('bootstrap',['autoreconf','-fi'],source),('configure',[str(source/'configure'),'--prefix='+str(prefix),*args],b),('build',['make','-j6'],b),('check',['make','check','-j6'],b),('install',['make','install'],b)]:
 print('QDP '+label,flush=True);start=time.monotonic()
 with (e/('qdp-'+label+'.log')).open('w') as log:p=subprocess.run(cmd,cwd=cwd,env=env,stdout=log,stderr=subprocess.STDOUT)
 records.append(dict(step=label,command=cmd,cwd=str(cwd),returncode=p.returncode,seconds=time.monotonic()-start));(e/'qdp-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
libs=subprocess.check_output([str(prefix/'bin/qdp++-config'),'--libs'],text=True);assert '-lfiledb' not in libs and '-lfilehash' not in libs
assert not list((prefix/'include').glob('qdp_db*.h'));assert not list((prefix/'lib').glob('*filedb*'));assert not list((prefix/'lib').glob('*filehash*'))
(e/'qdp-build.exit').write_text('0\n');print('QDP++ clean MPI build/check/install passed without FILEDB.',flush=True)
