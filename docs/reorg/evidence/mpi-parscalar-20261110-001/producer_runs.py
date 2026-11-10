from pathlib import Path
import subprocess,json,xml.etree.ElementTree as ET,hashlib,sys
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';k=r.parent/'redstar_kanoni.devel';old=r/'evidence/dbtools-extract-20261109-001';lane=sys.argv[1];assert lane in ('old','new');stack=old if lane=='old' else e;env=json.loads((e/'environments.json').read_text())['chroma'];env['OMP_NUM_THREADS']='2';records=[]
for name in ('meson','baryon'):
 for ranks in (1,2):
  folder=e/'producer-smoke'/lane/(name+'-'+str(ranks)+'-ranks');folder.mkdir(parents=True);tree=ET.parse(k/('chroma_inputs/superb-double/'+name+'.deriv.ini.xml'));param=tree.find('./Param/InlineMeasurements/elem/Param');param.find('num_vecs').text='2';param.find('mom2_max').text='0';param.find('mom_list').clear();disp=param.find('displacement_list');first=list(disp)[:2];disp.clear();disp.extend(first);param.find('use_superb_format').text='false';named=tree.find('./Param/InlineMeasurements/elem/NamedObject');named.find('./colorvec_files/elem').text=str(k/'runs/native-superb-rsd14-20261101-001/colorvec.mod');named.find(name+'_op_file').text=name+'.sdb';inp=folder/'input.xml';tree.write(inp,encoding='utf-8',xml_declaration=True)
  cmd=['/opt/homebrew/bin/mpirun','-n',str(ranks),'/usr/bin/sandbox-exec','-f',str(e/'chroma.sb'),str(stack/'install/chroma/bin/chroma'),'-i',str(inp),'-o',str(folder/'output.xml')]
  (folder/'command.json').write_text(json.dumps(cmd,indent=2)+'\n');print(lane,name,ranks,flush=True)
  with (folder/'run.log').open('w') as f:p=subprocess.run(cmd,cwd=folder,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=240)
  records.append({'case':name,'ranks':ranks,'returncode':p.returncode,'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'database':str(folder/(name+'.sdb'))});(e/('producer-'+lane+'.json')).write_text(json.dumps(records,indent=2)+'\n');p.check_returncode()
print(lane+' producer smoke completed',flush=True)
