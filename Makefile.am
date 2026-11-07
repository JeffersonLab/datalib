AM_CXXFLAGS = -I$(srcdir)/include @XPATH_CXXFLAGS@ @DATALIB_XML_CFLAGS@
lib_LIBRARIES = libdatalib_serial.a
libdatalib_serial_a_SOURCES = src/binary.cc src/byteorder.cc src/serial_xml.cc src/crc32.cc
nobase_datalibheaders_HEADERS = include/datalib/io/serial_xml.h include/datalib/codec/binary.h include/datalib/codec/byteorder.h include/datalib/model/arrays.h include/datalib/model/arraynd.h
datalibheadersdir = $(prefix)
bin_SCRIPTS = datalib-config
EXTRA_DIST = datalib-config.in provenance.json COPYING

check_PROGRAMS = codec_probe
codec_probe_SOURCES = tests/codec_probe.cc
codec_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
TESTS = tests/check-codec.sh
TESTS_ENVIRONMENT = srcdir='$(srcdir)'
EXTRA_DIST += tests/check-codec.sh tests/fixtures/manifest.json tests/fixtures/primitives.bin tests/fixtures/metadata.xml
EXTRA_DIST += tests/fixtures/graph-v3.bin tests/fixtures/graph-v4.bin

libdatalib_serial_a_SOURCES += \
    src/cgc_irrep_mom.cc \
    src/cgc_su3.cc \
    src/corr_graph.cc \
    src/graph_momentum.cc \
    src/hadron_adj_map.cc \
    src/hadron_class_npart_npt_conn_graph.cc \
    src/hadron_diagram_time.cc \
    src/hadron_disp_vertex.cc \
    src/hadron_sun_class_npart_irrep.cc \
    src/hadron_sun_class_npart_irrep_op.cc \
    src/hadron_sun_class_npart_npt_corr.cc \
    src/hadron_timeslice.cc \
    src/particle_class_op.cc \
    src/single_hadron_coeffs.cc
nobase_datalibheaders_HEADERS += \
    include/datalib/graph/process_corrs.h \
    include/datalib/graph/hadron_class_npart_npt_conn_graph.h \
    include/datalib/graph/hadron_sun_class_npart_npt_corr.h \
    include/datalib/graph/hadron_timeslice.h \
    include/datalib/graph/single_hadron_coeffs.h \
    include/datalib/graph/hadron_disp_vertex.h \
    include/datalib/graph/hadron_adj_map.h \
    include/datalib/graph/hadron_sun_class_npart_irrep_op.h \
    include/datalib/graph/hadron_sun_class_npart_irrep.h \
    include/datalib/graph/hadron_diagram_time.h \
    include/datalib/graph/particle_class_op.h \
    include/datalib/graph/hadron_sun_class_npart_version.h \
    include/datalib/graph/cgc_su3.h \
    include/datalib/graph/cgc_irrep_mom.h \
    include/datalib/model/map_obj.h \
    include/datalib/codec/superb_options.h
EXTRA_DIST += graph-provenance.json

check_PROGRAMS += graph_probe
graph_probe_SOURCES = tests/graph_probe.cc
graph_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
TESTS += tests/check-graph.sh
EXTRA_DIST += tests/check-graph.sh tests/fixtures/graph-reference.json
EXTRA_DIST += tests/fixtures/legacy-v3.xml tests/fixtures/legacy-v4.bin tests/fixtures/legacy-v4.xml tests/fixtures/legacy-v4.xml.bin

nobase_datalibheaders_HEADERS += include/datalib/storage/corr_storage.h
check_PROGRAMS += storage_probe
storage_probe_SOURCES = tests/storage_probe.cc
TESTS += tests/check-storage.sh
EXTRA_DIST += storage-provenance.json tests/check-storage.sh \
    tests/fixtures/storage-reference.json tests/fixtures/correlator-v0-little.bin \
    tests/fixtures/correlator-v0-big.bin

nobase_datalibheaders_HEADERS += include/datalib/storage/filedb.h
if WITH_FILEDB
check_PROGRAMS += filedb_probe
filedb_probe_SOURCES = tests/filedb_probe.cc
filedb_probe_CPPFLAGS = $(AM_CPPFLAGS) @FILEDB_CXXFLAGS@
filedb_probe_LDFLAGS = @FILEDB_LDFLAGS@
filedb_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@ @FILEDB_LIBS@
TESTS += tests/check-filedb.sh
endif
EXTRA_DIST += filedb-provenance.json tests/check-filedb.sh tests/filedb_probe.cc \
    tests/fixtures/filedb-reference.json tests/fixtures/filedb-key.bin \
    tests/fixtures/filedb-legacy-key.bin tests/fixtures/filedb-value.bin

libdatalib_serial_a_SOURCES += src/corr_output.cc
nobase_datalibheaders_HEADERS += include/datalib/storage/corr_keys.h include/datalib/storage/corr_output.h

if WITH_FILEDB
AM_CPPFLAGS = @FILEDB_CXXFLAGS@
libdatalib_serial_a_SOURCES += src/baryon_elem_type.cc src/meson_elem_type.cc src/prop_elem_type.cc src/unsmeared_genprop4_elem.cc src/unsmeared_meson_elem.cc src/unsmeared_twoquark_disco.cc src/twoquark_gamma_util.cc src/twoquark_operators_util.cc src/glueball_elem_type.cc src/tetraquark_elem_type.cc
nobase_datalibheaders_HEADERS += include/datalib/superb/baryon_elem_type.h include/datalib/superb/baryon_superb.h include/datalib/superb/colorvec_matelem_type.h include/datalib/superb/disco_superb.h include/datalib/superb/genprop4.h include/datalib/superb/glueball_elem_type.h include/datalib/superb/glueball_superb.h include/datalib/superb/tetraquark_elem_type.h include/datalib/superb/tetraquark_superb.h include/datalib/superb/meson_elem_type.h include/datalib/superb/meson_superb.h include/datalib/superb/prop_elem_type.h include/datalib/superb/prop_superb.h include/datalib/superb/superb_aux.h include/datalib/superb/superb_contractions.h include/datalib/superb/superb_tensor.h include/datalib/superb/twoquark_gamma_util.h include/datalib/superb/twoquark_operators_util.h include/datalib/superb/types.h include/datalib/superb/unsmeared_genprop4_elem.h include/datalib/superb/unsmeared_meson_elem.h include/datalib/superb/unsmeared_twoquark_disco.h
endif

libdatalib_serial_a_SOURCES += src/xml/basic_xpath_reader.cc src/xml/xml_simplewriter.cc src/xml/attribute.cc
nobase_datalibheaders_HEADERS += include/datalib/io/detail/basic_xpath_reader.h include/datalib/io/detail/xml_document.h include/datalib/io/detail/xml_simplewriter.h include/datalib/io/detail/xml_attribute.h
EXTRA_DIST += COPYING.xpath_reader src/xml/README.md

EXTRA_DIST += runtime-provenance.json

EXTRA_DIST += \
    docs/reorg/NATIVE-DOUBLE-REGRESSIONS.md \
    docs/reorg/README.md \
    docs/reorg/VALIDATION.md \
    docs/reorg/evidence/native-double-full-20261106-001/audit.log \
    docs/reorg/evidence/native-double-full-20261106-001/audit.py \
    docs/reorg/evidence/native-double-full-20261106-001/independent-audit.json \
    docs/reorg/evidence/native-double-full-20261106-001/npt-only/command.json \
    docs/reorg/evidence/native-double-full-20261106-001/npt-only/suite/results.json \
    docs/reorg/evidence/native-double-full-20261106-001/paired/command.json \
    docs/reorg/evidence/native-double-full-20261106-001/paired/suite/results.json \
    docs/reorg/evidence/native-double-full-20261106-001/policy-runner-library.sha256 \
    docs/reorg/evidence/native-double-full-20261106-001/regression-policy.json \
    docs/reorg/evidence/native-double-full-20261106-001/reorg-sources.log \
    docs/reorg/evidence/no1pt-reorg-runtime-001/build-chroma.json \
    docs/reorg/evidence/no1pt-reorg-runtime-001/build-cpu.json \
    docs/reorg/evidence/no1pt-reorg-runtime-001/datalib-core-only-test-suite.log \
    docs/reorg/evidence/no1pt-reorg-runtime-001/native-double-smoke/suite/results.json \
    docs/reorg/evidence/no1pt-reorg-runtime-001/verification.json \
    docs/reorg/evidence/no1pt-reorg-runtime-001/verify-gamma.log \
    docs/reorg/evidence/no1pt-reorg-runtime-001/verify-omp.log
