from pathlib import Path
import subprocess,json,hashlib,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001';e.mkdir(parents=True)
def git(p,*a):return subprocess.check_output(['git','--no-optional-locks','-C',str(p),*a],text=True)
protected={}
for n in ('adat.devel','redstar.devel','chroma.devel','colorvec.devel','qdpxx.devel','redstar_kanoni.devel'):
 p=r.parent/n;x={'devel':git(p,'rev-parse','refs/heads/devel')}
 if n=='redstar_kanoni.devel':x.update(status=git(p,'status','--short','--branch'),diff=hashlib.sha256(subprocess.check_output(['git','-C',str(p),'diff','--binary','HEAD'])).hexdigest())
 protected[n]=x
(e/'protected-before.json').write_text(json.dumps(protected,indent=2)+'\n')
state={}
for n in ('datalib','chroma','adat','redstar','colorvec','qdpxx','filedb','xpath_reader'):
 p=r/n;assert git(p,'branch','--show-current').strip()=='rge/devel-datalib-reorg';assert not git(p,'diff','--cached','--name-only')
 state[n]={'head':git(p,'rev-parse','HEAD').strip(),'status':git(p,'status','--short'),'local_devel':git(p,'for-each-ref','--format=%(objectname)','refs/heads/devel')}
(e/'reorg-before.json').write_text(json.dumps(state,indent=2)+'\n')
for rel in git(r/'chroma','ls-files').splitlines():
 p=r/'chroma'/rel
 if p.is_file() and p.suffix in ('.h','.cc'):
  text=p.read_text()
  if any(x in text for x in ('BinaryStoreDB','SerialDBKey','SerialDBData','FILEDB::')):
   dest=e/'before/chroma'/rel;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(p.read_bytes())
source=r/'adat/main/dbutil';assert not (source/'dbutil.cc').exists();assert ' dbutil ' not in (source/'Makefile.am').read_text().split('bin_PROGRAMS =',1)[1].split('\nEXTRA_PROGRAMS',1)[0]
old=r/'evidence/tools-filedb-20261109-001/install';assert (old/'datalib-filedb/bin/dbutil').is_file() and not (old/'adat/bin/dbutil').exists()
(e/'dbutil-ownership.json').write_text(json.dumps({'executable':str(old/'datalib-filedb/bin/dbutil'),'adat_executable_absent':True,'adat_source_absent':True,'remaining_directory':'ADAT ENSEM/physics utilities; rename to main/dbtools'},indent=2)+'\n')
s='''#include "util/ferm/key_prop_matelem.h"
#include "util/ferm/key_val_db.h"
#include <fstream>
#include <fcntl.h>
int main(int argc,char**argv){QDP_initialize(&argc,&argv);multi1d<int> dims(4);dims=2;Layout::setLattSize(dims);Layout::create();std::string dir=argv[1];Chroma::KeyPropElementalOperator_t key{};key.t_slice=3;key.t_source=1;key.spin_src=0;key.spin_snk=1;key.mass_label="U0.05";Chroma::ValPropElementalOperator_t value;value.mat.resize(2,2);for(int j=0;j<2;++j)for(int k=0;k<2;++k)value.mat(j,k)=cmplx(Real64(1.25+j-2*k),Real64(-.5+2*j+k));Chroma::SerialDBKey<Chroma::KeyPropElementalOperator_t> k(key);Chroma::SerialDBData<Chroma::ValPropElementalOperator_t> v(value);std::string kb,vb;k.writeObject(kb);v.writeObject(vb);std::ofstream(dir+"/prop-key.bin",std::ios::binary)<<kb;std::ofstream(dir+"/prop-value.bin",std::ios::binary)<<vb;for(int mode=0;mode<2;++mode){const std::string file=dir+(mode?"/local.sdb":"/collective.sdb");if(mode){Chroma::LocalBinaryStoreDB<decltype(k),decltype(v)> db;db.setMaxUserInfoLen(2048);db.open(file,O_RDWR|O_CREAT|O_EXCL,0600);db.insertUserdata("<DBMetaData><id>propElemOp</id><lattSize>2 2 2 4</lattSize><decay_dir>3</decay_dir></DBMetaData>");db.insert(k,v);db.close();}else{Chroma::BinaryStoreDB<decltype(k),decltype(v)> db;db.setMaxUserInfoLen(2048);db.open(file,O_RDWR|O_CREAT|O_EXCL,0600);db.insertUserdata("<DBMetaData><id>propElemOp</id><lattSize>2 2 2 4</lattSize><decay_dir>3</decay_dir></DBMetaData>");db.insert(k,v);db.close();}}QDP_finalize();}
'''
(e/'freeze.cc').write_text(s);(e/'frozen').mkdir();cfg=old/'chroma/bin/chroma-config'
def flags(n):return shlex.split(subprocess.check_output([str(cfg),'--'+n],text=True))
cmd=['/opt/homebrew/bin/mpicxx',*flags('cxxflags'),str(e/'freeze.cc'),'-o',str(e/'freeze'),*flags('ldflags'),*flags('libs')]
(e/'freeze-command.json').write_text(json.dumps(cmd,indent=2)+'\n')
with (e/'freeze-build.log').open('w') as out:subprocess.run(cmd,stdout=out,stderr=subprocess.STDOUT,check=True)
with (e/'freeze-run.log').open('w') as out:subprocess.run([str(e/'freeze'),str(e/'frozen')],stdout=out,stderr=subprocess.STDOUT,check=True)
(e/'frozen/manifest.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (e/'frozen').iterdir()},indent=2)+'\n')
print('Baseline states, dbutil ownership and real legacy writer fixtures frozen.',flush=True)
