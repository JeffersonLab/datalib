#include <datalib/storage/record_store.h>
#include <datalib/superb/meson_elem_type.h>
#include <datalib/superb/baryon_elem_type.h>
#include <fcntl.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class T> T decode(const std::string& bytes){Datalib::Binary::BinaryBufferReader bin(bytes);T value;read(bin,value);return value;}
static std::vector<std::complex<double>> values(const std::string& kind,const std::string& bytes){std::vector<std::complex<double>> out;
 if(kind=="meson"){auto v=decode<Hadron::ValMesonElementalOperator_t>(bytes);require(v.op.size1()==2&&v.op.size2()==2,"meson shape differs");for(int j=0;j<2;++j)for(int k=0;k<2;++k)out.push_back(v.op(j,k));}
 else{auto v=decode<Hadron::ValBaryonElementalOperator_t>(bytes);require(v.op.size1()==2&&v.op.size2()==2&&v.op.size3()==2,"baryon shape differs");for(int i=0;i<2;++i)for(int j=0;j<2;++j)for(int k=0;k<2;++k)out.push_back(v.op(i,j,k));}
 return out;
}
int main(int argc,char**argv){try{require(argc==4,"expected kind reference candidate");std::string kind=argv[1];Datalib::TensorStorage::StorageRecords a,b;require(a.open_records(argv[2],O_RDONLY,0)==0&&b.open_records(argv[3],O_RDONLY,0)==0,"open failed");auto keys=a.record_keys(),other=b.record_keys();std::sort(keys.begin(),keys.end());std::sort(other.begin(),other.end());require(!keys.empty()&&keys==other,"key coverage differs");std::string ma,mb;require(a.read_metadata(ma)==0&&b.read_metadata(mb)==0,"metadata read failed");std::ofstream(std::string(argv[2])+".metadata.xml")<<ma;std::ofstream(std::string(argv[3])+".metadata.xml")<<mb;double global=0,maxerr=0;int count=0,nonzero=0,identical=0;std::vector<std::pair<std::complex<double>,std::complex<double>>> pairs;
 for(const auto&key:keys){std::string av,bv;require(a.get_record(key,av)==0&&b.get_record(key,bv)==0,"read failed");identical+=(av==bv);auto va=values(kind,av),vb=values(kind,bv);require(va.size()==vb.size(),"shape differs");for(std::size_t i=0;i<va.size();++i){auto z=va[i],w=vb[i];require(std::isfinite(z.real())&&std::isfinite(z.imag())&&std::isfinite(w.real())&&std::isfinite(w.imag()),"nonfinite value");global=std::max(global,std::abs(z));maxerr=std::max(maxerr,std::abs(z-w));nonzero+=(std::abs(z)>0);pairs.emplace_back(z,w);++count;}}
 require(nonzero>0,"all values zero");for(auto pair:pairs)require(std::abs(pair.first-pair.second)<=1e-9*std::max(std::abs(pair.first),std::abs(pair.second))+1e-12*std::max(global,1e-30),"numerical comparison failed");std::cout<<"PASS kind="<<kind<<" keys="<<keys.size()<<" values="<<count<<" byte_identical_values="<<identical<<" max_abs_error="<<maxerr<<" metadata=exported-for-independent-check\n";
}catch(const std::exception&ex){std::cerr<<ex.what()<<'\n';return 1;}}
