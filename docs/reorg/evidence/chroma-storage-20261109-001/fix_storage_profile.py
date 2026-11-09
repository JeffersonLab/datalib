from pathlib import Path
import subprocess,json,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';c=r/'chroma';source=e/'sources/chroma';assert (e/'build.exit').read_text().strip()=='0'
p=c/'lib/util/ferm/storage_io.h';s=p.read_text().replace('#ifdef BUILD_SB\n','#ifdef BUILD_SB\n// Match the CORR reader profile: tensor-storage class layouts depend on this.\n# ifndef USE_SUPERBBLAS\n#  define USE_SUPERBBLAS\n# endif\n');p.write_text(s)
p=c/'configure.ac';s=p.read_text().replace('# redstar-datalib \n','# redstar-datalib \n');mark='AS_IF([test "x$DATALIB_PREFIX" = xno]';i=s.index(mark);s=s[:i]+'''dnl Export the same storage layout to clients that include datalib directly.
DATALIB_PROFILE_CXXFLAGS=
AS_IF([test "x${with_superbblas}" != xno],
  [DATALIB_PROFILE_CXXFLAGS="-DUSE_SUPERBBLAS -DUSE_SUPERBBLAS_WITH_GPU_SUPPORT"])
AC_SUBST([DATALIB_PROFILE_CXXFLAGS])

'''+s[i:];p.write_text(s)
p=c/'chroma-config.in';s=p.read_text().replace('@REDSTAR_DATALIB_CXXFLAGS@','@REDSTAR_DATALIB_CXXFLAGS@ @SB_CXXFLAGS@ @DATALIB_PROFILE_CXXFLAGS@');p.write_text(s)
p=c/'lib/CMakeLists.txt';s=p.read_text().replace('  target_include_directories(chromalib PUBLIC ${SUPERBBLAS_DIR}/include)','  target_compile_definitions(chromalib PUBLIC USE_SUPERBBLAS USE_SUPERBBLAS_WITH_GPU_SUPPORT)\n  target_include_directories(chromalib PUBLIC ${SUPERBBLAS_DIR}/include)');p.write_text(s)
subprocess.run(['git','-C',str(c),'add','--','lib/util/ferm/storage_io.h','configure.ac','chroma-config.in','lib/CMakeLists.txt'],check=True);subprocess.run(['git','-C',str(c),'diff','--cached','--check'],check=True);subprocess.run(['git','-C',str(c),'commit','-m','Keep tensor storage layouts consistent across Chroma readers and writers'],check=True)
subprocess.run(['git','-C',str(source),'pull','--ff-only','origin','rge/devel-datalib-reorg'],check=True)
(e/'build.exit').rename(e/'build-initial.exit');(e/'chroma-build.exit').rename(e/'chroma-initial-build.exit')
rows=json.loads((e/'build.json').read_text());config=next(x['command'] for x in rows if x['step']=='chroma-configure');env=json.loads((e/'environments.json').read_text())['chroma'];records=[]
for label,cmd,cwd in [('profile-bootstrap',['autoreconf','-fi'],source),('profile-configure',config,e/'build/chroma'),('profile-build',['make','-j3'],e/'build/chroma'),('profile-install',['make','install'],e/'build/chroma')]:
 command=['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),*cmd];print(label,flush=True);start=time.monotonic()
 with (e/('chroma-'+label+'.log')).open('w') as f:p=subprocess.run(command,cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':command,'returncode':p.returncode,'seconds':time.monotonic()-start});(e/'chroma-profile-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
a=json.loads((e/'stack-sources.json').read_text());a['chroma']['commit']=subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip();a['chroma']['final_source_update']='Normal tracked source fast-forwards followed by dependency rebuilds for custom record binding and consistent tensor-storage ABI; class-size probe and source/compile dependencies recorded.';(e/'stack-sources.json').write_text(json.dumps(a,indent=2)+'\n')
state={name:{'head':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'rev-parse','HEAD'],text=True).strip(),'status':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'status','--short'],text=True),'local_devel':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'for-each-ref','--format=%(objectname)','refs/heads/devel'],text=True)} for name in ('datalib','chroma','adat','colorvec','redstar','xpath_reader','qdpxx')};(e/'reorg-validation-heads.json').write_text(json.dumps(state,indent=2)+'\n');(e/'chroma-build.exit').write_text('0\n');(e/'build.exit').write_text('0\n');print('Consistent storage profile rebuilt and installed')
