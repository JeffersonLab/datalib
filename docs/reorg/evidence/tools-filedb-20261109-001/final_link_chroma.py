from pathlib import Path
import subprocess,json,time,shutil,hashlib,re
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001'
if (e/'chroma-build.exit').exists():(e/'chroma-build.exit').rename(e/'chroma-initial-build.exit')
while True:
 try:
  rows=json.loads((e/'final-build.json').read_text())
  if any(x['step']=='final-datalib-filedb-install' and x['returncode']==0 for x in rows):break
 except (FileNotFoundError,json.JSONDecodeError):pass
 time.sleep(5)
# Confirm no Chroma compilation input changed with the metadata-only inline fix.
changed=subprocess.check_output(['git','-C',str(r/'datalib'),'diff','--name-only','10ce4b4','500a35f'],text=True).splitlines();assert changed==['include/datalib/storage/dbtype.h']
for p in (e/'build/chroma').rglob('*.Po'):assert 'datalib/storage/dbtype.h' not in p.read_text(),p
# Relink all installed Chroma main programs against the final archive, using the cleanly built, unchanged objects.
env=json.loads((e/'environments.json').read_text())['chroma'];records=[]
for label,cmd,cwd in [('unlink-programs',['make','clean-binPROGRAMS'],e/'build/chroma/mainprogs/main'),('link',['make','-j3'],e/'build/chroma/mainprogs/main')]:
 command=['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),*cmd]
 with (e/('chroma-final-'+label+'.log')).open('w') as out:p=subprocess.run(command,cwd=cwd,env=env,stdout=out,stderr=subprocess.STDOUT)
 records.append({'step':label,'command':command,'returncode':p.returncode});p.check_returncode()
shutil.move(e/'install/chroma',e/'install/chroma-initial')
with (e/'chroma-final-install.log').open('w') as out:p=subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),'make','install'],cwd=e/'build/chroma',env=env,stdout=out,stderr=subprocess.STDOUT)
p.check_returncode();records.append({'step':'install','returncode':0})
lib=e/'install/datalib-filedb/lib/libdatalib_serial.a';exe=e/'install/chroma/bin/chroma'
(e/'chroma-final-link.json').write_text(json.dumps({'status':'passed','records':records,'datalib_archive_sha256':hashlib.sha256(lib.read_bytes()).hexdigest(),'executable_sha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'chroma_source_code_unchanged':True,'metadata_only_header_not_in_chroma_dependencies':True},indent=2)+'\n')
(e/'chroma-build.exit').write_text('0\n');print('Final Chroma programs relinked against the corrected standalone archive.',flush=True)
