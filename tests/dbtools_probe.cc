#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/graph/filenames.h>
#include <datalib/codec/binary.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
std::string bytes(const std::string& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("missing fixture: "+p);return {std::istreambuf_iterator<char>(f),{}};}
template<class T>T roundtrip(const std::string& p){T value{};Datalib::Binary::BinaryBufferReader in(bytes(p));read(in,value);Datalib::Binary::BinaryBufferWriter out;write(out,value);if(out.str()!=bytes(p))throw std::runtime_error("legacy bytes changed: "+p);return value;}
int main(int argc,char**argv){if(argc!=2)return 2;std::string p=argv[1];auto a=roundtrip<FF::KeyHadron1PtCorr_t>(p+"/legacy-1pt.bin");auto b=roundtrip<FF::KeyHadron2PtCorr_t>(p+"/legacy-2pt.bin");auto c=roundtrip<FF::KeyHadron3PtCorr_t>(p+"/legacy-3pt.bin");roundtrip<Hadron::KeyHadronNPartNPtCorr_t>(p+"/legacy-su2-npt.bin");auto v=roundtrip<std::vector<std::complex<double>>>(p+"/legacy-vector.bin");if(v.size()!=3||v[0]!=std::complex<double>(1.25,-0.5)||v[2]!=std::complex<double>(-.75,3.5))throw std::runtime_error("legacy value decode differs");std::ostringstream out;out<<Hadron::ensemFileName(a)<<"\n"<<Hadron::ensemFileName(b)<<"\n"<<Hadron::ensemFileName(c)<<"\n";if(out.str()!=bytes(p+"/legacy-filenames.txt"))throw std::runtime_error("filenames changed");}
