if BUNDLED_FILEDB
SUBDIRS = other_libs/filedb .
else
SUBDIRS = .
endif
AM_CXXFLAGS = -I$(srcdir)/include @DATALIB_XML_CFLAGS@
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
    include/datalib/graph/corr_graph.h \
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

# Array definitions are owned by datalib; no external xpath_reader headers.
nobase_datalibheaders_HEADERS += include/datalib/model/xml_array.h include/datalib/model/xml_array2d.h include/datalib/model/xml_array3d.h include/datalib/model/xml_array4d.h
EXTRA_DIST += array-provenance.json

check_PROGRAMS += arrays_probe
arrays_probe_SOURCES = tests/arrays_probe.cc
arrays_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
TESTS += tests/check-arrays.sh
EXTRA_DIST += tests/check-arrays.sh tests/fixtures/arrays-ranks.bin tests/fixtures/arrays-ranks.xml tests/fixtures/arrays-ranks-reference.json

# Owned-array extraction verification and full regression records.
EXTRA_DIST += \
    docs/reorg/ARRAYS-OWNED-REGRESSIONS.md \
    docs/reorg/evidence/arrays-owned-20261108-001/array-provenance.json \
    docs/reorg/evidence/arrays-owned-20261108-001/arrays_probe.cc \
    docs/reorg/evidence/arrays-owned-20261108-001/audit-driver.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/audit.py \
    docs/reorg/evidence/arrays-owned-20261108-001/build.json \
    docs/reorg/evidence/arrays-owned-20261108-001/build.py \
    docs/reorg/evidence/arrays-owned-20261108-001/check_aliases.py \
    docs/reorg/evidence/arrays-owned-20261108-001/check_api.py \
    docs/reorg/evidence/arrays-owned-20261108-001/chroma-dynamic-libraries.log \
    docs/reorg/evidence/arrays-owned-20261108-001/chroma-executor.json \
    docs/reorg/evidence/arrays-owned-20261108-001/chroma-link.log \
    docs/reorg/evidence/arrays-owned-20261108-001/chroma-without-adat \
    docs/reorg/evidence/arrays-owned-20261108-001/chroma.sb \
    docs/reorg/evidence/arrays-owned-20261108-001/coexist-arrays.cc \
    docs/reorg/evidence/arrays-owned-20261108-001/datalib-dist.sb \
    docs/reorg/evidence/arrays-owned-20261108-001/datalib.sb \
    docs/reorg/evidence/arrays-owned-20261108-001/devel-after.log \
    docs/reorg/evidence/arrays-owned-20261108-001/devel-before.log \
    docs/reorg/evidence/arrays-owned-20261108-001/direct-api-check.json \
    docs/reorg/evidence/arrays-owned-20261108-001/environment.json \
    docs/reorg/evidence/arrays-owned-20261108-001/environments.json \
    docs/reorg/evidence/arrays-owned-20261108-001/export_correlators.cc \
    docs/reorg/evidence/arrays-owned-20261108-001/finish.exit \
    docs/reorg/evidence/arrays-owned-20261108-001/finish.py \
    docs/reorg/evidence/arrays-owned-20261108-001/freeze-command.json \
    docs/reorg/evidence/arrays-owned-20261108-001/freeze-run.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/frozen-arrays/manifest.json \
    docs/reorg/evidence/arrays-owned-20261108-001/independent-audit.json \
    docs/reorg/evidence/arrays-owned-20261108-001/npt-only.exit \
    docs/reorg/evidence/arrays-owned-20261108-001/npt-only/command.json \
    docs/reorg/evidence/arrays-owned-20261108-001/npt-only/suite.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/npt-only/suite/results.json \
    docs/reorg/evidence/arrays-owned-20261108-001/openmp-check.json \
    docs/reorg/evidence/arrays-owned-20261108-001/openmp_check.py \
    docs/reorg/evidence/arrays-owned-20261108-001/owned-array-verification.json \
    docs/reorg/evidence/arrays-owned-20261108-001/paired.exit \
    docs/reorg/evidence/arrays-owned-20261108-001/paired/command.json \
    docs/reorg/evidence/arrays-owned-20261108-001/paired/suite.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/paired/suite/results.json \
    docs/reorg/evidence/arrays-owned-20261108-001/phases.json \
    docs/reorg/evidence/arrays-owned-20261108-001/policy-runner-library.sha256 \
    docs/reorg/evidence/arrays-owned-20261108-001/prepare.py \
    docs/reorg/evidence/arrays-owned-20261108-001/protected-before.json \
    docs/reorg/evidence/arrays-owned-20261108-001/publication.json \
    docs/reorg/evidence/arrays-owned-20261108-001/qdp-bundled-first-run.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/qdp-upstream-first-run.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/redstar-dynamic-libraries.log \
    docs/reorg/evidence/arrays-owned-20261108-001/reorg-before.json \
    docs/reorg/evidence/arrays-owned-20261108-001/reorg-sources.log \
    docs/reorg/evidence/arrays-owned-20261108-001/reorg-validation-heads.json \
    docs/reorg/evidence/arrays-owned-20261108-001/report.py \
    docs/reorg/evidence/arrays-owned-20261108-001/run_full-driver.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/run_full.py \
    docs/reorg/evidence/arrays-owned-20261108-001/sandbox-canaries.json \
    docs/reorg/evidence/arrays-owned-20261108-001/setup_full.py \
    docs/reorg/evidence/arrays-owned-20261108-001/stack-sources.json \
    docs/reorg/evidence/arrays-owned-20261108-001/stack-verification.json \
    docs/reorg/evidence/arrays-owned-20261108-001/verify_owned.py \
    docs/reorg/evidence/arrays-owned-20261108-001/verify_stack.py \
    docs/reorg/evidence/arrays-owned-20261108-001/xpath-bundled-first-run.log.gz \
    docs/reorg/evidence/arrays-owned-20261108-001/xpath-upstream-first-run.log.gz

# Shared models and diagnostic tools.
libdatalib_serial_a_SOURCES += src/legacy_hadron_1pt_corr.cc src/legacy_hadron_2pt_corr.cc src/legacy_hadron_3pt_corr.cc src/legacy_particle_op.cc src/legacy_hadron_npart_irrep_op.cc src/legacy_hadron_npart_irrep.cc src/legacy_hadron_npart_npt_corr.cc src/naming.cc src/operator_name.cc src/filenames.cc
nobase_datalibheaders_HEADERS += include/datalib/legacy/hadron_1pt_corr.h include/datalib/legacy/hadron_2pt_corr.h include/datalib/legacy/hadron_3pt_corr.h include/datalib/legacy/particle_op.h include/datalib/legacy/hadron_npart_irrep_op.h include/datalib/legacy/hadron_npart_irrep.h include/datalib/legacy/hadron_npart_npt_corr.h include/datalib/graph/naming.h include/datalib/graph/operator_name.h include/datalib/graph/filenames.h
bin_PROGRAMS = view_corr_graph dump_corr_graph
view_corr_graph_SOURCES = main/view_corr_graph.cc
view_corr_graph_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
dump_corr_graph_SOURCES = main/dump_corr_graph.cc
dump_corr_graph_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
if WITH_FILEDB
nobase_datalibheaders_HEADERS += include/datalib/storage/dbtype.h include/datalib/storage/corr_reader.h
bin_PROGRAMS += dbutil
dbutil_SOURCES = main/dbutil.cc main/mod_reader.h
dbutil_CPPFLAGS = $(AM_CPPFLAGS) @SUPERBBLAS_CXXFLAGS@
dbutil_LDFLAGS = @FILEDB_LDFLAGS@ @SUPERBBLAS_LDFLAGS@
dbutil_LDADD = libdatalib_serial.a @FILEDB_LIBS@ @SUPERBBLAS_LIBS@ @DATALIB_DEP_LIBS@
endif
EXTRA_DIST += tests/test_mod_reader.py main/mod_reader.txt

check_PROGRAMS += dbtools_probe
dbtools_probe_SOURCES = tests/dbtools_probe.cc
dbtools_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
TESTS += tests/check-dbtools.sh
if WITH_FILEDB
TESTS += tests/check-mod.sh
endif
EXTRA_DIST += tests/check-dbtools.sh tests/check-mod.sh tests/fixtures/dbtools/manifest.json tests/fixtures/dbtools/legacy-1pt.bin tests/fixtures/dbtools/legacy-2pt.bin tests/fixtures/dbtools/legacy-3pt.bin tests/fixtures/dbtools/legacy-su2-npt.bin tests/fixtures/dbtools/legacy-vector.bin tests/fixtures/dbtools/legacy-filenames.txt

EXTRA_DIST += tools-provenance.json

# FILEDB/tool extraction and full native-double regression evidence.
EXTRA_DIST += \
    docs/reorg/FILEDB-TOOLS-REGRESSIONS.md \
    docs/reorg/evidence/tools-filedb-20261109-001/adapter-1-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adapter-2-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adapter-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adapter_probe.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/adapter_probe.py \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-002-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-002-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-002-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-003-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-003-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-003-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-004-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-004-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-004-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-005-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-005-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-005-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-005-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-005-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-clone.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-inactive-utility-limitations.json \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-submodule-commits.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-submodules.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/adat-utility-syntax.json \
    docs/reorg/evidence/tools-filedb-20261109-001/audit-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/audit.py \
    docs/reorg/evidence/tools-filedb-20261109-001/baseline-syntax-dbconvert.cc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/baseline-syntax-dbconvert_read.cc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/baseline-syntax-dbconvert_write.cc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/baseline-syntax-dbcorr_read.cc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/baseline-syntax-extract_corr.cc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/build_downstream_final.py \
    docs/reorg/evidence/tools-filedb-20261109-001/build_qdp.py \
    docs/reorg/evidence/tools-filedb-20261109-001/build_stack.py \
    docs/reorg/evidence/tools-filedb-20261109-001/bundled-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/bundled-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/bundled-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/bundled-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/changes.json \
    docs/reorg/evidence/tools-filedb-20261109-001/check_aliases-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/check_aliases.py \
    docs/reorg/evidence/tools-filedb-20261109-001/check_api-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/check_api.py \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-clone.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-dynamic-libraries.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-executor.json \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-final-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-final-link.json \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-final-link.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-final-unlink-programs.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-link.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-resumed-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-resumed-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-resumed-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-submodule-commits.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma-submodules.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/chroma.sb \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-005-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-005-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-005-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-005-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-005-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-clone.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-submodule-commits.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/colorvec-submodules.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/commit_sources.py \
    docs/reorg/evidence/tools-filedb-20261109-001/compatibility-absence-compile.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/core-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/core-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/core-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/core-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/corr-superb-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-clone.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-core-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-core-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-core-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-core-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-dist-final.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-dist.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-dist.sb \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-filedb-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-filedb-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-filedb-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-filedb-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-submodule-commits.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib-submodules.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/datalib.sb \
    docs/reorg/evidence/tools-filedb-20261109-001/devel-after.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/devel-before.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/direct-api-check.json \
    docs/reorg/evidence/tools-filedb-20261109-001/direct-api-compile.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/direct-api-probe.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/direct-api-run.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/downstream-final-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/downstream-retry-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/downstream-retry3-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/downstream-retry4-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/eloy-filedb-check.json \
    docs/reorg/evidence/tools-filedb-20261109-001/eloy-filedb-final-check.json \
    docs/reorg/evidence/tools-filedb-20261109-001/environment.json \
    docs/reorg/evidence/tools-filedb-20261109-001/environments.json \
    docs/reorg/evidence/tools-filedb-20261109-001/export_correlators.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/exporter-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/exporter-command.json \
    docs/reorg/evidence/tools-filedb-20261109-001/external-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/external-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/external-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/external-final-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/external-final-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/external-final-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/extract.py \
    docs/reorg/evidence/tools-filedb-20261109-001/extraction-map.json \
    docs/reorg/evidence/tools-filedb-20261109-001/final-adat-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-adat-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-adat-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-adat-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-adat-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/final-colorvec-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-colorvec-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-colorvec-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-colorvec-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-colorvec-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-core-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-core-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-core-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-core-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-filedb-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-filedb-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-filedb-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-datalib-filedb-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-redstar-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-redstar-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-redstar-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-redstar-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final-redstar-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/final_link_chroma.py \
    docs/reorg/evidence/tools-filedb-20261109-001/finish_ensem.py \
    docs/reorg/evidence/tools-filedb-20261109-001/finish_qdp.py \
    docs/reorg/evidence/tools-filedb-20261109-001/finish_redstar.py \
    docs/reorg/evidence/tools-filedb-20261109-001/finish_validation.py \
    docs/reorg/evidence/tools-filedb-20261109-001/fix_ensem.py \
    docs/reorg/evidence/tools-filedb-20261109-001/fix_includes.py \
    docs/reorg/evidence/tools-filedb-20261109-001/freeze-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/freeze.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/graph-executors.json \
    docs/reorg/evidence/tools-filedb-20261109-001/include-path-completion.json \
    docs/reorg/evidence/tools-filedb-20261109-001/independent-audit.json \
    docs/reorg/evidence/tools-filedb-20261109-001/initial-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/initial-source-commits.json \
    docs/reorg/evidence/tools-filedb-20261109-001/npt-only/command.json \
    docs/reorg/evidence/tools-filedb-20261109-001/npt-only/suite/results.json \
    docs/reorg/evidence/tools-filedb-20261109-001/openmp-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/openmp-check.json \
    docs/reorg/evidence/tools-filedb-20261109-001/openmp.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/openmp_check-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/openmp_check.py \
    docs/reorg/evidence/tools-filedb-20261109-001/package-check-final.json \
    docs/reorg/evidence/tools-filedb-20261109-001/package-check.json \
    docs/reorg/evidence/tools-filedb-20261109-001/package_checks.py \
    docs/reorg/evidence/tools-filedb-20261109-001/package_checks_final.py \
    docs/reorg/evidence/tools-filedb-20261109-001/paired/command.json \
    docs/reorg/evidence/tools-filedb-20261109-001/paired/suite/results.json \
    docs/reorg/evidence/tools-filedb-20261109-001/phases.json \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight-build-second.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight-build-third.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight.py \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight2-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/preflight2.py \
    docs/reorg/evidence/tools-filedb-20261109-001/prepare.py \
    docs/reorg/evidence/tools-filedb-20261109-001/preserved-adat-filedb-worktree.json \
    docs/reorg/evidence/tools-filedb-20261109-001/probe.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/protected-before.json \
    docs/reorg/evidence/tools-filedb-20261109-001/publish_report.py \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-baseline-simd-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-check-limitation.json \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdp-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/qdpxx-base.json \
    docs/reorg/evidence/tools-filedb-20261109-001/qdpxx.py \
    docs/reorg/evidence/tools-filedb-20261109-001/rebuild_final.py \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-005-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-005-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-005-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-006-bootstrap.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-006-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-006-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-clone.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-dynamic-libraries.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-final-build.json \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-submodule-commits.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/redstar-submodules.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/refine.py \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-array.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-array.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-binary.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-binary.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-filedb.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-filedb.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-map.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-map.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-xml.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/removed-alias-xml.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/reorg-before.json \
    docs/reorg/evidence/tools-filedb-20261109-001/reorg-sources.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/reorg-validation-heads.json \
    docs/reorg/evidence/tools-filedb-20261109-001/report.py \
    docs/reorg/evidence/tools-filedb-20261109-001/resume.py \
    docs/reorg/evidence/tools-filedb-20261109-001/resume_chroma.py \
    docs/reorg/evidence/tools-filedb-20261109-001/resume_downstream.py \
    docs/reorg/evidence/tools-filedb-20261109-001/resume_downstream3.py \
    docs/reorg/evidence/tools-filedb-20261109-001/resume_downstream4.py \
    docs/reorg/evidence/tools-filedb-20261109-001/run_full-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/run_full.py \
    docs/reorg/evidence/tools-filedb-20261109-001/sandbox-canaries.json \
    docs/reorg/evidence/tools-filedb-20261109-001/setup_regressions.py \
    docs/reorg/evidence/tools-filedb-20261109-001/stack-sources.json \
    docs/reorg/evidence/tools-filedb-20261109-001/stack-verification.json \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-1-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-2-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-final-1-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-final-2-ranks.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage-probe-final-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/storage_probe.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/storage_probe.py \
    docs/reorg/evidence/tools-filedb-20261109-001/storage_probe_final.cc \
    docs/reorg/evidence/tools-filedb-20261109-001/storage_probe_final.py \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-check_corr_graph.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-db_rephase_corrs.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-db_transform_corrs.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbavg.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbavg_disco.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbavgcors.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbavgsrc.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbbin.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbcanon_mom.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbcombine.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbconvert.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbconvert_corr_util.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbconvert_ops_util.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbconvert_read.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbconvert_write.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbcorr_read.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbgaus_corr.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbiso_wick_rescale.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbmerge.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbmeson_t0fix.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbmeta.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbprop_diag_t0.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbprop_t0fix.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbreplicate.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-dbwick_rescale.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/syntax-extract_corr.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/tensor-build.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/tensor-check.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/tensor-configure.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/tensor-install.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/verify_tools-driver.log.gz \
    docs/reorg/evidence/tools-filedb-20261109-001/verify_tools.py \
    docs/reorg/evidence/tools-filedb-20261109-001/publication.json

EXTRA_DIST += docs/reorg/evidence/tools-filedb-20261109-001/filedb-worktree-cleanup.json

if WITH_FILEDB
nobase_datalibheaders_HEADERS += include/datalib/storage/record_store.h
endif

if WITH_FILEDB
check_PROGRAMS += records_probe
records_probe_SOURCES = tests/records_probe.cc
records_probe_CPPFLAGS = $(AM_CPPFLAGS) @FILEDB_CXXFLAGS@
records_probe_LDFLAGS = @FILEDB_LDFLAGS@
records_probe_LDADD = @FILEDB_LIBS@
TESTS += tests/check-records.sh
endif
EXTRA_DIST += tests/check-records.sh tests/fixtures/chroma-records.json tests/fixtures/chroma-prop-key.bin tests/fixtures/chroma-prop-value.bin
