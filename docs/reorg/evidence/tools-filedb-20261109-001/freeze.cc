#include <ensem/ensem.h>
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
 std::ofstream(d+"/legacy-filenames.txt")<<Hadron::ensemFileName(a)<<"\n"<<Hadron::ensemFileName(b)<<"\n"<<Hadron::ensemFileName(c)<<"\n";
}
