from pathlib import Path
import re,json,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';a=r/'adat';e=r/'evidence/tools-filedb-20261109-001'
# Metadata facade stays header-only so tensor code uses each consumer's own SuperbBLAS mode.
p=d/'include/datalib/storage/dbtype.h';h=p.read_text();cc=(d/'src/dbtype.cc').read_text();cc=cc[cc.index('namespace Hadron'):]
cc=re.sub(r'^  (std::string|Layout::Layout_t|void|int) (\w+)\(',r'  inline \1 \2(',cc,flags=re.M)
h=h.replace('#endif','#include <datalib/superb/superb_aux.h>\n#include <datalib/storage/corr_reader.h>\n#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>\n#include <sstream>\n'+cc+'\n#endif');p.write_text(h);(d/'src/dbtype.cc').unlink()
p=d/'Makefile.am';s=p.read_text().replace('AM_CPPFLAGS = @FILEDB_CXXFLAGS@ @SUPERBBLAS_CXXFLAGS@','AM_CPPFLAGS = @FILEDB_CXXFLAGS@').replace('libdatalib_serial_a_SOURCES += src/dbtype.cc\n','');p.write_text(s)
p=d/'configure.ac';s=p.read_text();s=s.replace('AS_HELP_STRING([--with-filedb=DIR],[Enable standalone FILEDB checks and export FILEDB component flags])','AS_HELP_STRING([--with-filedb=bundled|DIR],[Build bundled FILEDB (default) or use an installed prefix; --without-filedb disables it])')
s=s.replace('SUPERBBLAS_CXXFLAGS="-DUSE_SUPERBBLAS -I$withval/include"; SUPERBBLAS_LDFLAGS="-L$withval/lib"; SUPERBBLAS_LIBS="-lsuperbblas"','AS_IF([test "x$with_filedb" = xno],[AC_MSG_ERROR([--with-superbblas requires FILEDB support])]); AS_IF([test ! -f "$withval/lib/pkgconfig/superbblas.pc"],[AC_MSG_ERROR([superbblas.pc not found])]); SUPERBBLAS_CXXFLAGS="-DUSE_SUPERBBLAS -I$withval/include `pkg-config --variable=superbblas_config "$withval/lib/pkgconfig/superbblas.pc"`"; SUPERBBLAS_LDFLAGS="-L$withval/lib"; SUPERBBLAS_LIBS="-lsuperbblas"')
p.write_text(s)
# All callers use the actual datalib reader; ADAT keeps only value conversion functions.
for repo in ('adat','redstar','colorvec'):
 for p in (r/repo).rglob('*'):
  if not p.is_file() or p.suffix not in ('.h','.cc') or 'other_libs' in p.parts:continue
  s=p.read_text();t=s.replace('ADATIO::StorageCorrFunc','Datalib::Storage::StorageCorrFunc').replace('ADATIO::OneCanUseTheNewFormat','Datalib::Storage::OneCanUseTheNewFormat').replace('ADATIO::DontUseTheNewFormat','Datalib::Storage::DontUseTheNewFormat')
  if t!=s:p.write_text(t)
p=a/'lib/io/superb_kv.h';s=p.read_text();s=s[:s.index('using Datalib::Storage::DontUseTheNewFormat;')]+'}\n';p.write_text(s)
# Keep CMake manifests honest about moved files; no new CMake validation claim.
p=a/'lib/CMakeLists.txt';s=p.read_text();m=json.loads((e/'extraction-map.json').read_text());
for old in m:
 if old.startswith('formfac/'):
  s=re.sub(r'^\s*'+re.escape(old[:-2]+'.cc')+r'\s*\n','',s,flags=re.M)
 else:
  s=re.sub(r'^\s*'+re.escape(old)+r'\s*\n','',s,flags=re.M);s=re.sub(r'^\s*'+re.escape(old[:-2]+'.cc')+r'\s*\n','',s,flags=re.M)
p.write_text(s)
p=a/'CMakeLists.txt';s=p.read_text().replace('add_subdirectory(other_libs/filedb)','''# FILEDB and shared data models are supplied by installed datalib.
find_program(DATALIB_CONFIG datalib-config HINTS "${DATALIB_ROOT}/bin" REQUIRED)
foreach(flag cxxflags ldflags libs filedb-cxxflags filedb-ldflags filedb-libs)
  execute_process(COMMAND ${DATALIB_CONFIG} --${flag} OUTPUT_VARIABLE result
                  OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
  separate_arguments(result NATIVE_COMMAND "${result}")
  if(flag MATCHES "cxxflags$")
    add_compile_options(${result})
  else()
    link_libraries(${result})
  endif()
endforeach()''');# Ensure global flags are configured before the targets.
s=s.replace('add_subdirectory(lib) \n','');s=s.replace('# The main codes','add_subdirectory(lib)\n\n# The main codes');p.write_text(s)
p=a/'main/dbutil/CMakeLists.txt';s=p.read_text();idx=s.index('set(EXES');s=s[idx:].replace('dbmerge dbutil dbmeta','dbmerge dbmeta').replace('dbscombine ','').replace('target_link_libraries(${EXE} dbutils)','target_link_libraries(${EXE} adat)');p.write_text(s)
# CMake lists sometimes co-locate the final entry with the closing bracket.
for p in (a/'lib/CMakeLists.txt',):
 s=p.read_text()
 for old in m:
  if not old.startswith('formfac/'):s=s.replace(old+' )',')')
 p.write_text(s)
# Portable frozen fixtures from the pre-move ADAT build.
fixtures=d/'tests/fixtures/dbtools';fixtures.mkdir(exist_ok=True)
for p in (e/'frozen').iterdir():
 if p.is_file():(fixtures/p.name).write_bytes(p.read_bytes())
probe='''#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/graph/filenames.h>
#include <datalib/codec/binary.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
std::string bytes(const std::string& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("missing fixture: "+p);return {std::istreambuf_iterator<char>(f),{}};}
template<class T>T roundtrip(const std::string& p){T value{};Datalib::Binary::BinaryBufferReader in(bytes(p));read(in,value);Datalib::Binary::BinaryBufferWriter out;write(out,value);if(out.str()!=bytes(p))throw std::runtime_error("legacy bytes changed: "+p);return value;}
int main(int argc,char**argv){if(argc!=2)return 2;std::string p=argv[1];auto a=roundtrip<FF::KeyHadron1PtCorr_t>(p+"/legacy-1pt.bin");auto b=roundtrip<FF::KeyHadron2PtCorr_t>(p+"/legacy-2pt.bin");auto c=roundtrip<FF::KeyHadron3PtCorr_t>(p+"/legacy-3pt.bin");roundtrip<Hadron::KeyHadronNPartNPtCorr_t>(p+"/legacy-su2-npt.bin");auto v=roundtrip<std::vector<std::complex<double>>>(p+"/legacy-vector.bin");if(v.size()!=3||v[0]!=std::complex<double>(1.25,-0.5)||v[2]!=std::complex<double>(-.75,3.5))throw std::runtime_error("legacy value decode differs");std::ostringstream out;out<<Hadron::ensemFileName(a)<<"\\n"<<Hadron::ensemFileName(b)<<"\\n"<<Hadron::ensemFileName(c)<<"\\n";if(out.str()!=bytes(p+"/legacy-filenames.txt"))throw std::runtime_error("filenames changed");}
'''
(d/'tests/dbtools_probe.cc').write_text(probe)
(d/'tests/check-dbtools.sh').write_text('#!/bin/sh\nset -eu\n./dbtools_probe "$srcdir/tests/fixtures/dbtools"\n')
(d/'tests/check-mod.sh').write_text('#!/bin/sh\nset -eu\nPYTHONDONTWRITEBYTECODE=1 python3 "$srcdir/tests/test_mod_reader.py" "$(pwd)/dbutil"\n')
for f in ('check-dbtools.sh','check-mod.sh'):(d/'tests'/f).chmod(0o755)
p=d/'Makefile.am';s=p.read_text()+'''\ncheck_PROGRAMS += dbtools_probe
dbtools_probe_SOURCES = tests/dbtools_probe.cc
dbtools_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@
TESTS += tests/check-dbtools.sh
if WITH_FILEDB
TESTS += tests/check-mod.sh
endif
EXTRA_DIST += tests/check-dbtools.sh tests/check-mod.sh tests/fixtures/dbtools/manifest.json tests/fixtures/dbtools/legacy-1pt.bin tests/fixtures/dbtools/legacy-2pt.bin tests/fixtures/dbtools/legacy-3pt.bin tests/fixtures/dbtools/legacy-su2-npt.bin tests/fixtures/dbtools/legacy-vector.bin tests/fixtures/dbtools/legacy-filenames.txt
''';p.write_text(s)
print('Storage boundary, metadata, manifests and frozen tests updated.')
