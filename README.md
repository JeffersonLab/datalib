# datalib

Datalib provides standalone serial XML and binary I/O, the version-4
correlation graph map and its codecs, the version-0 correlator container,
and an optional FILEDB adapter. The graph API is in
`<datalib/graph/process_corrs.h>`. Format 4 uses connected graphs directly
and omits the per-graph stitching records. Regenerate older graph files
with the matching Redstar graph generator.

The serial build needs the standalone xpath_reader array headers, libxml2,
a C++20 compiler, and OpenMP. Its XML parser and writer are compiled into
datalib under a separate namespace. The optional FILEDB adapter needs a FILEDB
installation. The FILEDB component also provides the typed tensor storage
keys and adapters used by Redstar and Chroma. The header-only evaluator
uses the consumer's SuperbBLAS build and its MPI configuration.
The core library builds independently of ADAT, QDP and MPI.

Run `autoreconf -fi` and configure from a separate build directory:

```sh
/path/to/datalib/configure --prefix=/path/to/install --without-qdp \
  --with-xpath=/path/to/xpath_reader --with-libxml2=/path/to/libxml2 \
  --enable-openmp
make
make check
make install
```

Set `OPENMP_CXXFLAGS` and `OPENMP_LIBS` for the selected toolchain.
Use `--with-filedb=PREFIX` to enable the adapter checks and export its
separate flags. Use `datalib-config` to obtain installed build flags.

Frozen fixtures check primitive codecs, format-4 binary and XML graph
serialization, correlator storage and FILEDB records. Format-3 fixtures
are used to test rejection. The provenance JSON files record the sources
and historical fixture generation.
