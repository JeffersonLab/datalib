# datalib

Datalib provides standalone serial XML and binary I/O, the version-4
correlation graph map and its codecs, the version-0 correlator container,
and FILEDB storage adapters. The graph API is in
`<datalib/graph/corr_graph.h>`. Format 4 uses connected graphs directly
and omits the per-graph stitching records. Regenerate older graph files
with the matching Redstar graph generator.

The serial build needs libxml2, a C++20 compiler, and OpenMP. The array
headers are installed under `<datalib/model/xml_array*.h>` and retain the
existing `XMLArray` public types; no xpath_reader installation is needed.
Its XML parser and writer are compiled into datalib under a separate namespace. FILEDB is a pinned submodule in `other_libs/filedb` and is built by default.
Use `git submodule update --init --recursive` before bootstrapping.
An external FILEDB installation remains supported. The FILEDB component also provides the typed tensor storage
keys and adapters used by Redstar and Chroma. The header-only evaluator
uses the consumer's SuperbBLAS build and its MPI configuration.
The core library builds independently of ADAT, QDP and MPI.

Implementation sources live in `lib/`, public headers in `include/datalib/`,
and command-line tools in `main/`.

Run `autoreconf -fi` and configure from a separate build directory:

```sh
/path/to/datalib/configure --prefix=/path/to/install \
  --with-libxml2=/path/to/libxml2 \
  --enable-openmp
make
make check
make install
```

Set `OPENMP_CXXFLAGS` and `OPENMP_LIBS` for the selected toolchain.
Use `--with-filedb=PREFIX` for an external installation, or `--without-filedb`
for the independent core and graph tools. `datalib-config --storage-cxxflags`,
`--storage-ldflags` and `--storage-libs` export the selected provider.
`view_corr_graph` and `dump_corr_graph` are installed by datalib. FILEDB builds
also install `dbutil`; `--with-superbblas=PREFIX` enables its serial tensor
backend. Link that tool with the selected SuperbBLAS BLAS/runtime dependencies
through `LDFLAGS` and `LIBS`, using the same compiler and OpenMP runtime.
The core library does not link a SuperbBLAS runtime or a QDP adapter.

Frozen fixtures check primitive codecs, format-4 binary and XML graph
serialization, correlator storage and FILEDB records. Format-3 fixtures
are used to test rejection. The provenance JSON files record the sources
and historical fixture generation.

## Reorganization validation

[Full regression and build reports](docs/reorg/README.md) record the tested
format-4 stack, retained deferrals, numerical results and source provenance.

The [clean standalone build verification](docs/reorg/STANDALONE-CLEAN-BUILD.md)
records datalib builds with ADAT/QDP access blocked and a fresh ADAT-free
Chroma build with targeted runtime checks.

The [full clean-stack regression](docs/reorg/NATIVE-DOUBLE-CLEAN-REGRESSIONS.md)
records the complete Redstar/Chroma matrix, retained memory-limit failure,
and successful cache-capped rerun under the unchanged process-memory guard.

The [bundled-array regression report](docs/reorg/ARRAYS-OWNED-REGRESSIONS.md)
records the xpath_reader dependency removal, format fixtures, provider
coexistence checks and complete native-double numerical sweep.

Shared metadata and correlator readers are available through
`<datalib/storage/dbtype.h>` and `<datalib/storage/corr_reader.h>`.
Legacy key models/codecs live under `<datalib/legacy/>`; ENSEM adapters stay
in ADAT. Operator filename formatting is in `<datalib/graph/filenames.h>`.

## CORR_SUPERB in Chroma

Configure Chroma with `--with-datalib=PREFIX` and
`--with-superbblas=PREFIX` to enable the CORR_SUPERB measurement. The normal
FILEDB-enabled datalib installation already provides its shared graph, key,
codec, storage and evaluator headers. Chroma uses its own matching QDP/QMP and
SuperbBLAS communication configuration; ADAT is not a dependency. Datalib's
`--with-superbblas` option enables the separate standalone dbutil tensor backend.

[FILEDB ownership and full CORR_SUPERB regression](docs/reorg/FILEDB-TOOLS-REGRESSIONS.md)
records the earlier ownership checkpoint and its retained platform-test limits.

[Direct Chroma storage and full regression](docs/reorg/CHROMA-STORAGE-REGRESSIONS.md)
records the subsequent writer migration, removal of temporary wrappers and
STOCH_HADRON, consistent SuperbBLAS storage profiles, and the lib/ source layout.
Datalib has no QDP or ADAT configure dependency. Chroma retains its own QDP/MPI
coordination and matching SuperbBLAS configuration.
