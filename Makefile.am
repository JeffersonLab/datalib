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

# Clean standalone build verification records; no binaries or numerical databases.
EXTRA_DIST += \
    docs/reorg/STANDALONE-CLEAN-BUILD.md \
    docs/reorg/evidence/standalone-clean-20261107-001/audit-driver.log \
    docs/reorg/evidence/standalone-clean-20261107-001/audit.py \
    docs/reorg/evidence/standalone-clean-20261107-001/build.json \
    docs/reorg/evidence/standalone-clean-20261107-001/build.py \
    docs/reorg/evidence/standalone-clean-20261107-001/chroma-clean-checkout.json \
    docs/reorg/evidence/standalone-clean-20261107-001/chroma-dynamic-libraries.log \
    docs/reorg/evidence/standalone-clean-20261107-001/chroma-link.log \
    docs/reorg/evidence/standalone-clean-20261107-001/chroma-submodule-commits.log \
    docs/reorg/evidence/standalone-clean-20261107-001/chroma.sb \
    docs/reorg/evidence/standalone-clean-20261107-001/data-provenance.json \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib-clean-checkout.json \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib-core-dynamic-libraries.log \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib-core-test-suite.log \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib-filedb-dynamic-libraries.log \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib-filedb-test-suite.log \
    docs/reorg/evidence/standalone-clean-20261107-001/datalib.sb \
    docs/reorg/evidence/standalone-clean-20261107-001/environments.json \
    docs/reorg/evidence/standalone-clean-20261107-001/finish.py \
    docs/reorg/evidence/standalone-clean-20261107-001/openmp-build.log \
    docs/reorg/evidence/standalone-clean-20261107-001/openmp.cc \
    docs/reorg/evidence/standalone-clean-20261107-001/openmp.json \
    docs/reorg/evidence/standalone-clean-20261107-001/openmp.py \
    docs/reorg/evidence/standalone-clean-20261107-001/prepare.py \
    docs/reorg/evidence/standalone-clean-20261107-001/protected-before.json \
    docs/reorg/evidence/standalone-clean-20261107-001/readback-build.log \
    docs/reorg/evidence/standalone-clean-20261107-001/readback-command.json \
    docs/reorg/evidence/standalone-clean-20261107-001/readback-dynamic-libraries.log \
    docs/reorg/evidence/standalone-clean-20261107-001/readback.cc \
    docs/reorg/evidence/standalone-clean-20261107-001/report.py \
    docs/reorg/evidence/standalone-clean-20261107-001/sandbox-canaries.json \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke/3pi/readback.json \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke/nuc_nuc_3pt/readback.json \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke/pion/readback.json \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke-driver.log \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke.json \
    docs/reorg/evidence/standalone-clean-20261107-001/smoke.py \
    docs/reorg/evidence/standalone-clean-20261107-001/toolchain.log \
    docs/reorg/evidence/standalone-clean-20261107-001/toolchain.py \
    docs/reorg/evidence/standalone-clean-20261107-001/verification.json

# Full clean-stack regression records, including the resource-limited attempt.
EXTRA_DIST += \
    docs/reorg/NATIVE-DOUBLE-CLEAN-REGRESSIONS.md \
    docs/reorg/evidence/native-double-clean-full-20261107-001/adat-clean-checkout.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/adat-submodule-commits.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/build.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-001/build.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/build_stack.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/check_aliases.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/check_api.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/chroma-dynamic-libraries.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/chroma-executor.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/chroma-without-adat \
    docs/reorg/evidence/native-double-clean-full-20261107-001/colorvec-clean-checkout.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/colorvec-submodule-commits.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/datalib-documentation-push.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/devel-before.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/direct-api-check.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/direct-api-probe.cc \
    docs/reorg/evidence/native-double-clean-full-20261107-001/environment.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/export_correlators.cc \
    docs/reorg/evidence/native-double-clean-full-20261107-001/exporter-command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/finish.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/memory-limit-failure.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/npt-only.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-001/npt-only/command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/npt-only/suite.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/npt-only/suite/results.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/openmp-check.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/openmp.cc \
    docs/reorg/evidence/native-double-clean-full-20261107-001/openmp_check.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired/command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired/suite.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired/suite/no_1pt/nucnuc_small/chroma.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired/suite/no_1pt/nucnuc_small/chroma.out.xml.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/paired/suite/results.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/phases.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/policy-runner-library.sha256 \
    docs/reorg/evidence/native-double-clean-full-20261107-001/prepare_checks.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/probe.cc.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/protected-before.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/redstar-clean-checkout.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/redstar-dynamic-libraries.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/redstar-submodule-commits.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/reorg-before.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/reorg-sources.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/environment.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/exit \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/suite/no_1pt/nucnuc_small/chroma.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/suite/no_1pt/nucnuc_small/strict-comparison.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/retry-cache1/suite/results.json \
    docs/reorg/evidence/native-double-clean-full-20261107-001/run_full-driver.log \
    docs/reorg/evidence/native-double-clean-full-20261107-001/run_full.py \
    docs/reorg/evidence/native-double-clean-full-20261107-001/stack-sources.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/audit-driver.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/audit.py \
    docs/reorg/evidence/native-double-clean-full-20261107-002/build.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/chroma-dynamic-libraries.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/chroma-executor.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/chroma-without-adat \
    docs/reorg/evidence/native-double-clean-full-20261107-002/datalib-documentation-push.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/devel-after.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/devel-before.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/direct-api-check.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/environment.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/export_correlators.cc \
    docs/reorg/evidence/native-double-clean-full-20261107-002/exporter-command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/finish.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-002/independent-audit.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/npt-only.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-002/npt-only/command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/npt-only/suite.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-002/npt-only/suite/results.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/openmp-check.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired/command.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired/suite.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired/suite/no_1pt/nucnuc_small/chroma.log.gz \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired/suite/no_1pt/nucnuc_small/strict-comparison.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/paired/suite/results.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/phases.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/policy-runner-library.sha256 \
    docs/reorg/evidence/native-double-clean-full-20261107-002/protected-before.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/redstar-dynamic-libraries.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/regression.exit \
    docs/reorg/evidence/native-double-clean-full-20261107-002/reorg-before.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/reorg-sources.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/report-driver.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/report.py \
    docs/reorg/evidence/native-double-clean-full-20261107-002/resource-retry.json \
    docs/reorg/evidence/native-double-clean-full-20261107-002/restart_full.py \
    docs/reorg/evidence/native-double-clean-full-20261107-002/run_full-driver.log \
    docs/reorg/evidence/native-double-clean-full-20261107-002/run_full.py \
    docs/reorg/evidence/native-double-clean-full-20261107-002/stack-sources.json \
    docs/reorg/evidence/native-double-clean-full-publication.json
