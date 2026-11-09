from pathlib import Path
import subprocess,json,hashlib,shlex
root=Path('/Users/admin/Downloads/macos-datalib-reorg');e=root/'evidence/tools-filedb-20261109-001';e.mkdir(parents=True)
def git(p,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(p),*args],text=True)
protected={}
for name in ('adat.devel','redstar.devel','chroma.devel','colorvec.devel','redstar_kanoni.devel'):
 p=root.parent/name;v={'devel':git(p,'rev-parse','refs/heads/devel')}
 if name=='redstar_kanoni.devel':v.update(status=git(p,'status','--short','--branch'),diff=hashlib.sha256(subprocess.check_output(['git','-C',str(p),'diff','--binary','HEAD'])).hexdigest())
 protected[name]=v
(e/'protected-before.json').write_text(json.dumps(protected,indent=2)+'\n')
state={}
for name in ('datalib','adat','redstar','chroma','colorvec','xpath_reader','filedb'):
 p=root/name;assert git(p,'branch','--show-current').strip()=='rge/devel-datalib-reorg',name
 assert not git(p,'diff','--cached','--name-only'),name
 state[name]={'head':git(p,'rev-parse','HEAD').strip(),'status':git(p,'status','--short'),'local_devel':git(p,'for-each-ref','--format=%(objectname)','refs/heads/devel')}
(e/'reorg-before.json').write_text(json.dumps(state,indent=2)+'\n')
for name in ('adat','redstar'):
 for f in (root/name).rglob('*.cc'):
  if name=='adat' and ('main/dbutil' in str(f) or f.name=='ensem_filenames.cc'):
   dst=e/'before'/name/f.relative_to(root/name);dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(f.read_bytes())
# Compile a fixture against the unchanged installation, before any source moves.
old=root/'evidence/arrays-owned-20261108-001/install'; cfg=old/'adat/bin/adat-config'
s='''#include <ensem/ensem.h>
#include <formfac/hadron_1pt_corr.h>
#include <formfac/hadron_2pt_corr.h>
#include <formfac/hadron_3pt_corr.h>
#include <hadron/su2_corr/hadron_npart_npt_corr.h>
#include <hadron/ensem_filenames.h>
#include <datalib/codec/binary.h>
#include <fstream>
namespace {
template<class T> void save(const std::string& p,const T& key){Datalib::Binary::BinaryBufferWriter b;write(b,key);std::ofstream(p,std::ios::binary)<<b.str();}
}
int main(int argc,char**argv){std::string d=argv[1];
 ENSEM::VectorComplex v;v.elem().resize(3);for(int j=0;j<3;++j){auto& z=v.elem().elem(j).elem().elem();z.real()=1.25-j;z.imag()=-0.5+2*j;}save(d+"/legacy-vector.bin",v);
 FF::KeyHadron1PtCorr_t a{};a.num_vecs=10;a.creation_op=1;a.name="pion";a.smear="S";a.lorentz=XMLArray::Array<int>(0);a.mom=XMLArray::Array<int>(3);a.mom[0]=0;a.mom[1]=1;a.mom[2]=-2;a.spin=2;a.mass="U0.05";a.ensemble="fixture";save(d+"/legacy-1pt.bin",a);
 FF::KeyHadron2PtCorr_t b{};b.num_vecs=2;b.src_name="pion";b.snk_name="rho";b.src_smear="S";b.snk_smear="P";b.src_spin=0;b.snk_spin=1;b.mom=a.mom;b.mass=a.mass;b.ensemble=a.ensemble;save(d+"/legacy-2pt.bin",b);
 FF::KeyHadron3PtCorr_t c{};c.num_vecs=2;c.pi_pf.p_i=a.mom;c.pi_pf.p_f=a.mom;c.gamma=4;c.links=XMLArray::Array<int>(0);c.dt=7;c.quark=1;c.src_name="pion";c.snk_name="pion";c.src_spin=0;c.snk_spin=0;c.mass=a.mass;c.ensemble=a.ensemble;save(d+"/legacy-3pt.bin",c);
 Hadron::KeyHadronNPartNPtCorr_t n{};n.npoint.resize(1);n.npoint[1].t_slice=3;n.npoint[1].irrep={};save(d+"/legacy-su2-npt.bin",n);
 std::ofstream(d+"/legacy-filenames.txt")<<Hadron::ensemFileName(a)<<"\\n"<<Hadron::ensemFileName(b)<<"\\n"<<Hadron::ensemFileName(c)<<"\\n";
}
'''
(e/'freeze.cc').write_text(s);(e/'frozen').mkdir()
flags=lambda x:shlex.split(subprocess.check_output([str(cfg),x],text=True))
cmd=['/usr/bin/clang++',*flags('--cxxflags'),str(e/'freeze.cc'),'-o',str(e/'freeze'),*flags('--ldflags'),*flags('--libs')]
with (e/'freeze-build.log').open('w') as out:subprocess.run(cmd,stdout=out,stderr=subprocess.STDOUT,check=True)
subprocess.run([str(e/'freeze'),str(e/'frozen')],check=True)
(e/'frozen/manifest.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (e/'frozen').iterdir()},indent=2)+'\n')
print(e)
