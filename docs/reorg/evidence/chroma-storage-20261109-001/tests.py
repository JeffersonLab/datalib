from pathlib import Path
import json,hashlib,shutil
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';e=r/'evidence/chroma-storage-20261109-001'
for f in ('prop-key.bin','prop-value.bin'):shutil.copy2(e/'frozen'/f,d/'tests/fixtures'/('chroma-'+f))
(d/'tests/records_probe.cc').write_text(r'''#include <datalib/storage/record_store.h>
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
''')
(d/'tests/check-records.sh').write_text('''#!/bin/sh
set -eu
tmp=$(mktemp -d "${TMPDIR:-/tmp}/datalib-records.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
./records_probe "$srcdir/tests/fixtures" "$tmp/records.sdb"
''');(d/'tests/check-records.sh').chmod(0o755)
p=d/'Makefile.am';s=p.read_text()+'''
if WITH_FILEDB
check_PROGRAMS += records_probe
records_probe_SOURCES = tests/records_probe.cc
records_probe_CPPFLAGS = $(AM_CPPFLAGS) @FILEDB_CXXFLAGS@
records_probe_LDFLAGS = @FILEDB_LDFLAGS@
records_probe_LDADD = @FILEDB_LIBS@
TESTS += tests/check-records.sh
endif
EXTRA_DIST += tests/check-records.sh tests/fixtures/chroma-records.json tests/fixtures/chroma-prop-key.bin tests/fixtures/chroma-prop-value.bin
''';p.write_text(s)
(d/'tests/fixtures/chroma-records.json').write_text(json.dumps({'provenance':'Chroma b067c360c3ff629b8927589b3cc3ca8588485d2d installed at tools-filedb-20261109-001, frozen before wrapper removal','source':'docs/reorg/evidence/chroma-storage-20261109-001/freeze.cc','sha256':{f:hashlib.sha256((d/'tests/fixtures'/f).read_bytes()).hexdigest() for f in ('chroma-prop-key.bin','chroma-prop-value.bin')}},indent=2)+'\n')
p=d/'main/mod_reader.h';p.write_text(p.read_text().replace('ADAT_DBUTIL_MOD_READER_H','DATALIB_DBUTIL_MOD_READER_H'))
p=d/'main/mod_reader.txt';s=p.read_text().replace('usual adat database-type','usual datalib database-type').replace('Build with the normal adat build.','Build with the normal datalib build.').replace('python3 main/dbutil/tests/test_mod_reader.py build/main/dbutil/dbutil','python3 tests/test_mod_reader.py /path/to/datalib/install/bin/dbutil');p.write_text(s)
