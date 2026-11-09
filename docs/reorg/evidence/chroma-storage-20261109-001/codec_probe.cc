#include "util/ferm/key_prop_matelem.h"
#include "util/ferm/storage_io.h"
#include <datalib/superb/prop_superb.h>
#include <fstream>
#include <iterator>
#include <fcntl.h>
#include <mpi.h>
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static std::string bytes(const std::string& path){std::ifstream f(path,std::ios::binary);require(bool(f),"missing fixture");return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char**argv){QDP_initialize(&argc,&argv);multi1d<int> dims(4);dims=2;Layout::setLattSize(dims);Layout::create();int rank=0;MPI_Comm_rank(MPI_COMM_WORLD,&rank);try{
 require(argc==3,"expected frozen directory and fresh output directory");const std::string frozen=argv[1],output=argv[2];
 Chroma::KeyPropElementalOperator_t key{};key.t_slice=3;key.t_source=1;key.spin_src=0;key.spin_snk=1;key.mass_label="U0.05";
 Chroma::ValPropElementalOperator_t value;value.mat.resize(2,2);for(int j=0;j<2;++j)for(int k=0;k<2;++k)value.mat(j,k)=cmplx(Real64(1.25+j-2*k),Real64(-.5+2*j+k));
 const auto kb=Chroma::encodeStorageRecord(key),vb=Chroma::encodeStorageRecord(value);
 require(kb==bytes(frozen+"/prop-key.bin")&&vb==bytes(frozen+"/prop-value.bin"),"QDP local encoding differs from frozen old wrappers");
 const std::string metadata="<DBMetaData><id>propElemOp</id><lattSize>2 2 2 4</lattSize><decay_dir>3</decay_dir></DBMetaData>";
 for(int mode=0;mode<2;++mode){
  const bool owner=mode || Layout::primaryNode();const std::string file=output+(mode?"/local-"+std::to_string(rank)+".sdb":"/collective.sdb");
  Datalib::TensorStorage::StorageProp4 db(false);db.record_options().metadata_capacity=2048;
  if(!mode){db.record_options().cache_bytes=50*1024*1024;db.record_options().configurations=1;}
  if(owner){require(db.open_records(file,O_RDWR|O_CREAT|O_EXCL,0600)==0,"open failed");require(db.write_metadata(metadata)==0&&db.put_record(kb,vb)==0,"write failed");db.close_records();}
  MPI_Barrier(MPI_COMM_WORLD);
  require(db.open_records(file,O_RDONLY,0)==0,"read reopen failed");std::string got;
  require(db.get_record(kb,got)==0&&got==vb&&db.record_keys()==std::vector<std::string>{kb},"written record differs");
  require(db.read_metadata(got)==0&&got==metadata,"metadata differs");db.close_records();
  require(db.open_records(frozen+(mode?"/local.sdb":"/collective.sdb"),O_RDONLY,0)==0,"old writer read failed");require(db.get_record(kb,got)==0&&got==vb,"old database payload differs");db.close_records();
 }
 std::cout<<"PASS rank="<<rank<<" old/new raw codecs, metadata and records; primary-only and per-rank ownership\n";
}catch(const std::exception&ex){std::cerr<<ex.what()<<'\n';MPI_Abort(MPI_COMM_WORLD,1);}QDP_finalize();}
