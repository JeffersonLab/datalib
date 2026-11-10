from pathlib import Path
import subprocess,json,shlex,re,math,hashlib,xml.etree.ElementTree as ET
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/mpi-parscalar-20261110-001';source=e/'producer_readback.cc';source.write_bytes((Path(__file__).parent/'producer_readback.cc').read_bytes());cfg=e/'install/datalib/bin/datalib-config'
def flags(n):return shlex.split(subprocess.check_output([str(cfg),'--'+n],text=True))
cmd=['/usr/bin/clang++',*flags('cxxflags'),*flags('storage-cxxflags'),str(source),'-o',str(e/'producer-readback'),*flags('ldflags'),*flags('storage-ldflags'),*flags('libs'),*flags('storage-libs')]
with (e/'producer-readback-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,check=True)
def metadata(path):
 file=Path(str(path)+'.metadata.xml');raw=file.read_bytes();root=ET.fromstring(raw);info=root.find('ProgramInfo');assert info is not None;date=info.find('run_date');assert date is not None and date.text.strip();geometry=info.find('Setgeom');assert geometry is not None
 get=lambda n:list(map(int,geometry.find(n).text.split()))
 size=get('latt_size');logical=get('logical_size');subgrid=get('subgrid_size');total=int(geometry.find('total_volume').text);volume=int(geometry.find('subgrid_volume').text);ranks=int(re.search(r'-(\d+)-ranks',str(path)).group(1));assert size==[4,4,4,16] and len(logical)==len(subgrid)==4;assert all(x>0 for x in logical+subgrid);assert math.prod(logical)==ranks and math.prod(size)==total==1024 and math.prod(subgrid)==volume==total//ranks;assert [a*b for a,b in zip(logical,subgrid)]==size
 provenance={'run_date':date.text,'logical_size':logical,'subgrid_size':subgrid,'subgrid_volume':volume,'ranks':ranks,'raw_sha256':hashlib.sha256(raw).hexdigest()};date.text='RUN_TIMESTAMP'
 if root.find('id').text=='mesonElemOp':
  batch=root.find('./Params/max_tslices_in_contraction');assert int(batch.text)==logical[3]
  assert ET.parse(path.parent/'input.xml').find('./Param/InlineMeasurements/elem/Param/max_tslices_in_contraction') is None
  provenance['automatic_timeslice_batch']=int(batch.text);batch.text='VALIDATED_DEFAULT_LOGICAL_TIME_BATCH'
 for tag in ('logical_size','subgrid_size','subgrid_volume'):geometry.find(tag).text='VALIDATED_RANK_GEOMETRY'
 return ET.tostring(root),provenance
results=[]
for kind in ('meson','baryon'):
 for label,a,b in [('old-new-1-rank','old/'+kind+'-1-ranks','new/'+kind+'-1-ranks'),('old-new-2-ranks','old/'+kind+'-2-ranks','new/'+kind+'-2-ranks'),('new-1-vs-2-ranks','new/'+kind+'-1-ranks','new/'+kind+'-2-ranks')]:
  pa=e/'producer-smoke'/a/(kind+'.sdb');pb=e/'producer-smoke'/b/(kind+'.sdb');cmd=[str(e/'producer-readback'),kind,str(pa),str(pb)];p=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(e/(kind+'-'+label+'-readback.log')).write_text(p.stdout);p.check_returncode();ca,ma=metadata(pa);cb,mb=metadata(pb);assert ca==cb,(kind,label,'physics metadata differs')
  if ma['ranks']==mb['ranks']:assert {k:ma[k] for k in ('logical_size','subgrid_size','subgrid_volume')}=={k:mb[k] for k in ('logical_size','subgrid_size','subgrid_volume')}
  if ma['ranks']==mb['ranks']:assert ma.get('automatic_timeslice_batch')==mb.get('automatic_timeslice_batch')
  results.append({'case':kind,'comparison':label,'command':cmd,'returncode':p.returncode,'output':p.stdout,'metadata':{'status':'passed','exact_except_validated_execution_provenance':True,'reference':ma,'candidate':mb,'normalized_sha256':hashlib.sha256(ca).hexdigest()}});(e/'producer-comparison.json').write_text(json.dumps(results,indent=2)+'\n')
print('old/new real producers and 1/2-rank numerical readback passed; physics metadata exact, run date and rank geometry separately validated')
