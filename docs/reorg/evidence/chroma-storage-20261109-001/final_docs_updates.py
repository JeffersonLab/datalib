from pathlib import Path
r=Path('/Users/admin/Downloads/macos-datalib-reorg')
p=r/'chroma/lib/CMakeLists.txt';s=p.read_text().replace('filedb-cxxflags','storage-cxxflags').replace('filedb-ldflags','storage-ldflags').replace('filedb-libs','storage-libs');p.write_text(s)
p=r/'chroma/docs/datalib-reorg.md';p.write_text('''# Standalone datalib storage and CORR_SUPERB

This reorg branch requires `--with-datalib=PREFIX` for shared data and storage.
Add `--with-superbblas=PREFIX` to enable the SUPERB measurements, including
CORR_SUPERB. Use the same compiler/OpenMP runtime and matching QDP/QMP/SuperbBLAS
builds. No ADAT installation is required.

Legacy measurement writers now use `Datalib::TensorStorage::StorageMeson`,
`StorageBaryon`, `StorageProp4`, `StorageGenprop4`, `StorageDisco`, `StorageGlue`
or the common `StorageRecords` interface. Their record operations preserve
existing encoded keys, values and metadata. Persistence and backend selection
belong to datalib. The temporary `filedb_qdp.h` and `key_val_db.h` headers and
Chroma database/serializable wrapper classes are removed.

`util/ferm/storage_io.h` contains only a QDP local serialization helper.
Chroma owns rank selection, file-existence broadcasts and error/abort handling.
Collective producers perform the existing reductions on all ranks, then encode
and store records on the primary rank. Local SUPERB producer paths keep their
existing ownership. Datalib performs no QDP or MPI communication.

QDP XML and lattice I/O remain collective; datalib XML I/O is serial.
`SB::StorageTensor` continues to use the consumer's SuperbBLAS communicator.
Chroma obtains the storage component's compile/link flags through
`datalib-config --storage-cxxflags`, `--storage-ldflags` and `--storage-libs`.
Both Autotools and CMake use those backend-neutral query names.

See datalib's `docs/reorg/CHROMA-STORAGE-REGRESSIONS.md` for the tested
Autotools stack, old/new writer comparisons, full native-double matrix and
retained deferrals and limits. Full multi-rank CORR, GPU and on-demand GenProp
coverage are outside the cached-tensor suite.
''')
p=r/'chroma/README';p.write_text(p.read_text().replace('Reorg datalib/FILEDB and CORR_SUPERB','Reorg datalib storage and CORR_SUPERB'))
p=r/'adat/README';s=p.read_text();s+='''
The standalone dbutil executable, including MOD metadata and key inspection,
is installed by datalib. Use datalib/bin/dbutil file.mod meta or keys.
ADAT's remaining ENSEM/physics database utilities are built in main/dbtools.
''';p.write_text(s)
p=r/'adat/docs/corr-graph-v4.md';p.write_text(p.read_text().replace('datalib/graph/process_corrs.h','datalib/graph/corr_graph.h').replace('main/dbutil','main/dbtools'))
p=r/'adat/docs/datalib-api.md';s=p.read_text();s+='''
The remaining ADAT ENSEM/physics tools now live in `main/dbtools` (renamed
from `main/dbutil`). The `dbutil` executable itself is solely owned by datalib.
Its `meta`, `keys` and `keysxml` operations support MOD files without QDP or
ADAT, with unchanged read-only format validation.
''';p.write_text(s)
p=r/'qdpxx/docs/filedb-reorg.md';s=p.read_text().replace('and\nretains its old database adapter locally while legacy writers are modernized.','and\nuses datalib storage classes directly for the legacy writers. Chroma\'s temporary\ndatabase adapter and serializable wrappers have been removed.');p.write_text(s)
for n in ('view_corr_graph','dump_corr_graph'):
 p=r/('redstar/docs/'+n+'.md');p.write_text(p.read_text().replace('datalib/src/corr_graph.cc','datalib/lib/corr_graph.cc'))
