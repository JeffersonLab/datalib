#ifdef REFERENCE
#include "io/adat_io.h"
#include "io/adat_xmlio.h"
namespace B=ADATIO; namespace X=ADATXML; namespace M=ADAT;
#else
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>
namespace B=Datalib::Binary; namespace X=Datalib::Serial; namespace M=Datalib::Model;
#endif
#include <omp.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <list>
#include <map>
void check(bool p,int line){if(!p)throw std::runtime_error("fixture mismatch at line "+std::to_string(line));}
#define need(x) check((x),__LINE__)
std::string load(const std::string&p){std::ifstream in(p,std::ios::binary);need(bool(in));return {std::istreambuf_iterator<char>(in),{}};}
int main(int argc,char**argv){need(argc==3);std::string mode=argv[1],dir=argv[2];
 B::BinaryBufferWriter b;
 int i=-1234567;unsigned u=4000000000U;short sh=-32760;float f=1.25f;double d=-1.23456789012345;bool flag=true;char c='Q';
 std::complex<double> z{2.5,-3.25};std::complex<float> zf{.125f,-.5f};
 std::vector<std::complex<double>> v{{1,2},{-3,.25}};std::vector<std::vector<int>> nested{{-1,2},{},{3}};std::list<int> list{7,-8,9};std::map<int,double> map{{-3,.25},{9,-1.5}};
 M::Array1dO<int> one(3);one[1]=-4;one[2]=0;one[3]=12;
 XMLArray::Array2d<int> two(2,3);int n=0;for(int j=0;j<2;++j)for(int k=0;k<3;++k)two[j][k]=++n;
 std::vector<int> empty;
 B::write(b,i);B::write(b,u);B::write(b,sh);B::write(b,f);B::write(b,d);B::write(b,flag);B::write(b,c);B::write(b,z);B::write(b,zf);B::write(b,v);B::write(b,nested);B::write(b,list);B::write(b,map);B::write(b,one);B::write(b,two);B::write(b,empty);B::writeDesc(b,"spaces & descriptions");
 if(mode=="emit"){std::ofstream out(dir+"/primitives.bin",std::ios::binary);out<<b.str();}
 else need(b.str()==load(dir+"/primitives.bin"));
 B::BinaryBufferReader reader(load(dir+"/primitives.bin"));
 int ii;unsigned uu;short ss;float ff;double dd;bool bb;char cc;std::complex<double> zz;std::complex<float> zzf;decltype(v) vv;decltype(nested) nn;decltype(list) ll;decltype(map) mm;decltype(one) oo;decltype(two) tt;decltype(empty) ee;std::string desc;
 B::read(reader,ii);B::read(reader,uu);B::read(reader,ss);B::read(reader,ff);B::read(reader,dd);B::read(reader,bb);B::read(reader,cc);B::read(reader,zz);B::read(reader,zzf);B::read(reader,vv);B::read(reader,nn);B::read(reader,ll);B::read(reader,mm);B::read(reader,oo);B::read(reader,tt);B::read(reader,ee);B::readDesc(reader,desc);
 need(ii==i&&uu==u&&ss==sh&&ff==f&&dd==d&&bb==flag&&cc==c&&zz==z&&zzf==zf&&vv==v&&nn==nested&&ll==list&&mm==map&&oo==one&&ee.empty()&&desc=="spaces & descriptions");
 for(int j=0;j<2;++j)for(int k=0;k<3;++k)need(tt[j][k]==two[j][k]);
 std::cerr<<"checksums reader="<<reader.getChecksum()<<" writer="<<b.getChecksum()<<"\n";need(reader.getChecksum()==b.getChecksum());
 bool rejected=false,failed=false;try{B::BinaryBufferReader bad(std::string(1,'x'));int tmp=0;B::read(bad,tmp);failed=bad.fail();}catch(const std::exception&){rejected=true;}
#ifdef REFERENCE
 need(rejected||failed); // Legacy EOF sets failbit but does not throw.
#else
 need(rejected); // Extracted codec must fail closed before consuming partial input.
#endif
 X::XMLBufferWriter xml;X::push(xml,"Fixture");X::write(xml,"integer",i);X::write(xml,"real",d);X::write(xml,"complex",z);X::write(xml,"values",std::vector<double>{1.25,-2.5});X::write(xml,"one_based",one);X::write(xml,"label",std::string("xml-safe label"));X::pop(xml);
 if(mode=="emit"){std::ofstream out(dir+"/metadata.xml");out<<xml.str();}else need(xml.str()==load(dir+"/metadata.xml"));
 std::istringstream input(load(dir+"/metadata.xml"));X::XMLReader xr(input);X::XMLReader scope(xr,"/Fixture");int xi;double xd;std::complex<double>xz;decltype(one) xo;std::vector<double>xv;std::string label;X::read(scope,"integer",xi);X::read(scope,"real",xd);X::read(scope,"complex",xz);X::read(scope,"values",xv);X::read(scope,"one_based",xo);X::read(scope,"label",label);need(xi==i&&xd==d&&xz==z&&xo==one&&xv==std::vector<double>({1.25,-2.5})&&label=="xml-safe label");
 int threads=0;
#pragma omp parallel
 {
#pragma omp single
 threads=omp_get_num_threads();
 }
 need(threads>=1);if(const char* expected=std::getenv("OMP_NUM_THREADS"))need(threads==std::stoi(expected));std::cout<<"PASS: frozen binary/XML bytes, semantic reads, checksum, indexing, truncated read; OpenMP threads="<<threads<<"\n";
}
