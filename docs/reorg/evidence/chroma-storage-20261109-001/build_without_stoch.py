from pathlib import Path
import subprocess,json,time
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';source=e/'sources/chroma';started=time.monotonic()
while not (e/'build.exit').exists():
 records=json.loads((e/'chroma-profile-build.json').read_text());assert all(x['returncode']==0 for x in records),records
 if time.monotonic()-started>1200:raise RuntimeError('Timed out waiting for the current shared-profile build')
 time.sleep(3)
assert (e/'build.exit').read_text().strip()=='0';(e/'build.exit').rename(e/'build-before-removal.exit');(e/'chroma-build.exit').rename(e/'chroma-before-removal.exit')
print('Applying removal to isolated build checkout',flush=True);subprocess.run(['git','-C',str(source),'pull','--ff-only','origin','rge/devel-datalib-reorg'],check=True)
old=e/'install/chroma';old.rename(e/'install/chroma-before-stoch-removal')
obj=e/'build/chroma/lib/meas/inline/hadron/inline_stoch_hadron_w.o'
if obj.exists():obj.rename(e/'removed-stoch-hadron-object.o')
config=next(x['command'] for x in json.loads((e/'build.json').read_text()) if x['step']=='chroma-configure');env=json.loads((e/'environments.json').read_text())['chroma'];records=[]
for label,cmd,cwd in [('removal-bootstrap',['autoreconf','-fi'],source),('removal-configure',config,e/'build/chroma'),('removal-build',['make','-j3'],e/'build/chroma'),('removal-install',['make','install'],e/'build/chroma')]:
 command=['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),*cmd];print(label,flush=True);start=time.monotonic()
 with (e/('chroma-'+label+'.log')).open('w') as f:p=subprocess.run(command,cwd=cwd,env=env,stdout=f,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':command,'returncode':p.returncode,'seconds':time.monotonic()-start});(e/'chroma-removal-build.json').write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
archive=e/'install/chroma/lib/libchroma.a';members=subprocess.check_output(['/usr/bin/ar','-t',str(archive)],text=True);assert 'inline_stoch_hadron_w.o' not in members
symbols=subprocess.check_output(['/usr/bin/nm','-C',str(e/'install/chroma/bin/chroma')],text=True);assert 'InlineStochHadronEnv::' not in symbols
assert not list((e/'install/chroma/include').rglob('inline_stoch_hadron_w.h'))
sourcefiles=subprocess.check_output(['git','-C',str(source),'ls-files','lib'],text=True).splitlines()
for rel in sourcefiles:
 p=source/rel
 if p.is_file() and p.suffix in ('.cc','.h','.am','.txt'):assert not any(x in p.read_text() for x in ('inline_stoch_hadron_w','InlineStochHadronEnv','STOCH_HADRON')),rel
removal=json.loads((e/'stoch-hadron-removal.json').read_text());removal.update(status='removed and verified',archive_member_absent=True,executable_symbols_absent=True,installed_header_absent=True);(e/'stoch-hadron-removal.json').write_text(json.dumps(removal,indent=2)+'\n')
a=json.loads((e/'stack-sources.json').read_text());a['chroma']['commit']=subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip();a['chroma']['final_source_update']='Tracked source fast-forwards, normal dependency rebuilds and relinking for tensor-storage ABI; STOCH_HADRON then removed from source, factory, build/archive and new empty installation. No copied overlays.';(e/'stack-sources.json').write_text(json.dumps(a,indent=2)+'\n')
state={name:{'head':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'rev-parse','HEAD'],text=True).strip(),'status':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'status','--short'],text=True),'local_devel':subprocess.check_output(['git','--no-optional-locks','-C',str(r/name),'for-each-ref','--format=%(objectname)','refs/heads/devel'],text=True)} for name in ('datalib','chroma','adat','colorvec','redstar','xpath_reader','qdpxx')};(e/'reorg-validation-heads.json').write_text(json.dumps(state,indent=2)+'\n');(e/'chroma-build.exit').write_text('0\n');(e/'build.exit').write_text('0\n');print('Chroma installed without STOCH_HADRON',flush=True)
