from pathlib import Path
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';c=r/'chroma'
p=d/'configure.ac';s=p.read_text();s='\n'.join(x for x in s.splitlines() if not (x.startswith('AC_ARG_WITH([qdp') or x.startswith('AS_IF([test "x$with_qdp"')))+'\n';p.write_text(s)
p=d/'README.md';s=p.read_text().replace(' --without-qdp','');s=s.replace('`datalib-config --filedb-cxxflags`,\n`--filedb-ldflags` and `--filedb-libs`','`datalib-config --storage-cxxflags`,\n`--storage-ldflags` and `--storage-libs`');p.write_text(s)
p=d/'datalib-config.in';s=p.read_text().replace('FILEDB flags require configuring datalib with --with-filedb=DIR','Storage flags require configuring datalib with FILEDB support').replace('--prefix|--backend|--filedb-cxxflags|--filedb-ldflags|--filedb-libs','--prefix|--backend|--storage-cxxflags|--storage-ldflags|--storage-libs');p.write_text(s)
p=d/'include/datalib/storage/record_store.h';s=p.read_text().replace('return records_ ? records_->insert(key, value) : -1;','return records_ ? records_->insert(Bytes{key}, Bytes{value}) : -1;');p.write_text(s)
p=c/'lib/CMakeLists.txt';p.write_text(p.read_text().replace('util/ferm/key_val_db.h','util/ferm/storage_io.h'))
p=c/'lib/meas/inline/hadron/inline_stoch_hadron_w.cc';s=p.read_text();s='\n'.join(x for x in s.splitlines() if '//BinaryFxStoreDB<' not in x)+'\n';p.write_text(s)
