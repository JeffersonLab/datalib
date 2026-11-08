#ifdef LEGACY_ARRAY_HEADERS
#include <xml_array.h>
#include <xml_array2d.h>
#include <xml_array3d.h>
#include <xml_array4d.h>
#else
#include <datalib/model/xml_array.h>
#include <datalib/model/xml_array2d.h>
#include <datalib/model/xml_array3d.h>
#include <datalib/model/xml_array4d.h>
#endif
#ifdef COEXIST
#include <xml_array.h>
#include <xml_array2d.h>
#include <xml_array3d.h>
#include <xml_array4d.h>
#endif
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

std::string load(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("cannot read fixture: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
void require(bool valid) {
  if (!valid) throw std::runtime_error("array fixture mismatch");
}
int main(int argc, char** argv) {
  try {
    require(argc == 3);
    const std::string mode=argv[1], folder=argv[2];
    XMLArray::Array<int> a(std::vector<int>{7,-8,9}), empty;
    XMLArray::Array2d<int> b(2,3);
    XMLArray::Array3d<int> c(2,3,4);
    XMLArray::Array4d<std::complex<double>> d(2,3,2,4);
    int n=0;
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)b[i][j]=++n-4;
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)for(int k=0;k<4;++k)c(i,j,k)=++n-17;
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)for(int k=0;k<2;++k)for(int l=0;l<4;++l) {
      ++n; d(i,j,k,l)={n/16.0,-n/32.0};
    }
    Datalib::Binary::BinaryBufferWriter out;
    Datalib::Binary::write(out,a);Datalib::Binary::write(out,b);
    Datalib::Binary::write(out,c);Datalib::Binary::write(out,d);
    Datalib::Binary::write(out,empty);
    Datalib::Serial::XMLBufferWriter xml;
    Datalib::Serial::push(xml,"Arrays");
    Datalib::Serial::write(xml,"values",a);
    Datalib::Serial::pop(xml);
    if(mode=="emit") {
      std::ofstream bin(folder+"/arrays-ranks.bin",std::ios::binary);bin<<out.str();
      std::ofstream text(folder+"/arrays-ranks.xml");text<<xml.str();
    } else {
      require(out.str()==load(folder+"/arrays-ranks.bin"));
      require(xml.str()==load(folder+"/arrays-ranks.xml"));
    }
    XMLArray::Array<int> aa, ee;
    XMLArray::Array2d<int> bb;
    XMLArray::Array3d<int> cc;
    XMLArray::Array4d<std::complex<double>> dd;
    Datalib::Binary::BinaryBufferReader in(load(folder+"/arrays-ranks.bin"));
    Datalib::Binary::read(in,aa);Datalib::Binary::read(in,bb);
    Datalib::Binary::read(in,cc);Datalib::Binary::read(in,dd);Datalib::Binary::read(in,ee);
    require(aa==a && ee.size()==0);
#ifdef EXPECT_EMPTY_EQUAL
    require((ee==empty)==bool(EXPECT_EMPTY_EQUAL));
#endif
    require(bb.size1()==3 && bb.size2()==2 && cc.size1()==4 && cc.size2()==3 && cc.size3()==2);
    require(dd.size1()==4 && dd.size2()==2 && dd.size3()==3 && dd.size4()==2);
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)require(bb[i][j]==b[i][j]);
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)for(int k=0;k<4;++k)require(cc(i,j,k)==c(i,j,k));
    for(int i=0;i<2;++i)for(int j=0;j<3;++j)for(int k=0;k<2;++k)for(int l=0;l<4;++l)require(dd(i,j,k,l)==d(i,j,k,l));
    require(in.getChecksum()==out.getChecksum());
    std::istringstream text(load(folder+"/arrays-ranks.xml"));
    Datalib::Serial::XMLReader reader(text);
    XMLArray::Array<int> restored;
    Datalib::Serial::read(reader,"/Arrays/values",restored);require(restored==a);
    std::cout<<"PASS: frozen rank-1/2/3/4 array bytes, dimensions, readback and XML\n";
  } catch(const std::string& error) {std::cerr<<error<<'\n';return 1;}
    catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
