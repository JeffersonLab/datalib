from pathlib import Path
import json,subprocess,hashlib,xml.etree.ElementTree as ET
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';old=r/'evidence/tools-filedb-20261109-001';source=r.parent/'redstar_kanoni.devel/runs/native-superb-rsd14-20261101-001/colorvec.mod';digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();before=digest(source);results={}
for label,exe in [('old',old/'install/datalib-filedb/bin/dbutil'),('new',e/'install/datalib-filedb/bin/dbutil')]:
 outputs={}
 for mode in ('meta','keys','keysxml'):
  cmd=[str(exe),str(source),mode];out=e/('mod-'+label+'-'+mode+'.xml')
  if mode=='keysxml':cmd.append(str(out))
  if label=='new':cmd=['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*cmd]
  p=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,check=True);(e/('mod-'+label+'-'+mode+'.log')).write_bytes(p.stdout);outputs[mode]={'command':cmd,'stdout_sha256':hashlib.sha256(p.stdout).hexdigest()}
  if mode=='keysxml':outputs[mode].update(xml_sha256=digest(out),key_count=len(ET.parse(out).findall('elem')))
 results[label]=outputs
assert all(results['old'][mode]['stdout_sha256']==results['new'][mode]['stdout_sha256'] for mode in ('meta','keys'))
assert results['old']['keysxml']['xml_sha256']==results['new']['keysxml']['xml_sha256'];assert results['new']['keysxml']['key_count']>0;assert digest(source)==before
(e/'mod-inspection.json').write_text(json.dumps({'status':'passed','source':str(source),'source_sha256':before,'unchanged_source':True,'results':results},indent=2)+'\n');print('Real MOD metadata, text keys and XML keys match with ADAT/QDP access blocked')
