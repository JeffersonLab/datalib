#include <datalib/storage/record_store.h>
#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
using Datalib::TensorStorage::StorageRecords;
static void require(bool test, const char* reason) {
  if (!test) throw std::runtime_error(reason);
}
static std::string bytes(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  require(bool(file), "missing frozen codec fixture");
  return {std::istreambuf_iterator<char>(file), {}};
}
int main(int argc, char** argv) {
  try {
    require(argc==3, "expected fixture directory and fresh output path");
    const auto key = bytes(std::string(argv[1])+"/chroma-prop-key.bin");
    const auto value = bytes(std::string(argv[1])+"/chroma-prop-value.bin");
    const std::string metadata = "<DBMetaData><id>propElemOp</id></DBMetaData>";
    StorageRecords db;
    std::string got;
    require(db.put_record(key,value)!=0 && db.get_record(key,got)!=0 &&
            db.read_metadata(got)!=0 && db.write_metadata(metadata)!=0,
            "closed store accepted an operation");
    require(!StorageRecords::record_file_exists(argv[2]), "output already exists");
    db.record_options().metadata_capacity = 2048;
    db.record_options().configurations = 1;
    db.record_options().cache_bytes = 50*1024*1024;
    require(db.open_records(argv[2],O_CREAT|O_EXCL|O_RDWR,0600)==0, "create failed");
    require(db.write_metadata(metadata)==0 && db.put_record(key,value)==0, "write failed");
    require(db.close_records()==0 && db.close_records()==0, "close failed");
    require(db.open_records(argv[2],O_RDONLY,0)==0, "read-only reopen failed");
    require(db.read_metadata(got)==0 && got==metadata, "metadata changed");
    require(db.get_record(key,got)==0 && got==value, "frozen payload changed");
    require(db.record_keys()==std::vector<std::string>{key}, "key bytes changed");
    require(db.get_record("missing",got)!=0, "missing key accepted");
    db.close_records();
    require(db.open_records(argv[2],O_RDWR,0600)==0, "append reopen failed");
    const std::string key2("a\0b",3), value2("c\0d\0",4);
    require(db.put_record(key2,value2)==0, "append failed");
    db.close_records();
    require(db.open_records(argv[2],O_RDONLY,0)==0, "final reopen failed");
    auto keys=db.record_keys();std::sort(keys.begin(),keys.end());
    auto expected=std::vector<std::string>{key,key2};std::sort(expected.begin(),expected.end());
    require(keys==expected && db.get_record(key,got)==0 && got==value &&
            db.get_record(key2,got)==0 && got==value2 &&
            db.read_metadata(got)==0 && got==metadata, "append changed existing bytes");
    db.close_records();
    std::cout<<"PASS: frozen Chroma record bytes, metadata, reopen, append and missing-key checks\n";
  } catch(const std::exception& ex) {std::cerr<<ex.what()<<'\n';return 1;}
}
