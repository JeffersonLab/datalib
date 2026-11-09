from pathlib import Path
import re,shutil,subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';a=r/'adat';e=r/'evidence/tools-filedb-20261109-001'
def put(p,s):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(s)
def edit(p,x,y):s=p.read_text();assert x in s,(p,x);p.write_text(s.replace(x,y))
def function_span(s,start):
 b=s.index('{',start);depth=0;state=None;i=b
 while i<len(s):
  c=s[i];n=s[i:i+2]
  if state=='line':
   if c=='\n':state=None
  elif state=='block':
   if n=='*/':state=None;i+=1
  elif state in ('"',"'"):
   if c=='\\':i+=1
   elif c==state:state=None
  elif n=='//':state='line';i+=1
  elif n=='/*':state='block';i+=1
  elif c in ('"',"'"):state=c
  elif c=='{':depth+=1
  elif c=='}':
   depth-=1
   if depth==0:return start,i+1
  i+=1
 raise ValueError('unclosed')
def extract_funcs(p,names):
 s=p.read_text();spans=[];functions=[]
 for name in names:
  found=list(re.finditer(r'^  (?:std::[^\n]*?|XMLArray::[^\n]*?|int|bool|RotateVec_t)\s+'+name+r'\s*\(',s,re.M))
  assert found,(p,name)
  for m in found:
   st,en=function_span(s,m.start());spans.append((st,en));functions.append(s[st:en])
 for st,en in sorted(spans,reverse=True):s=s[:st]+s[en:]
 p.write_text(s)
 return functions
# FILEDB becomes owned as a pinned submodule by datalib. Clone committed content only.
subprocess.run(['git','-C',str(d),'-c','protocol.file.allow=always','submodule','add',str(r/'filedb'),'other_libs/filedb'],check=True)
subprocess.run(['git','-C',str(d),'config','-f','.gitmodules','submodule.other_libs/filedb.url','https://github.com/usqcd-software/filedb.git'],check=True)
p=d/'other_libs/filedb';subprocess.run(['git','-C',str(p),'checkout','db2a795aaf6cc57b6eefe2ef634fdc91ee7b3693'],check=True)
subprocess.run(['git','-C',str(p),'checkout','-B','rge/devel-datalib-reorg'],check=True)
# Remove only the tracked gitlink; retain dirty old worktree on disk to preserve local bootstrap changes.
subprocess.run(['git','-C',str(a),'rm','--cached','other_libs/filedb'],check=True)
s=(a/'.gitmodules').read_text();s=re.sub(r'\[submodule "other_libs/filedb"\]\n(?:[^\[].*\n)*','',s);put(a/'.gitmodules',s)
edit(a/'other_libs/Makefile.am','xpath_reader minuit filedb','xpath_reader minuit')
edit(a/'configure.ac','AC_CONFIG_SUBDIRS(other_libs/filedb)','')
# Rename serialized graph API, and distinguish Redstar construction/assembly.
(d/'include/datalib/graph/process_corrs.h').rename(d/'include/datalib/graph/corr_graph.h')
for repo in ('datalib','adat','redstar','chroma','colorvec'):
 for name in subprocess.check_output(['git','-C',str(r/repo),'ls-files'],text=True).splitlines():
  p=r/repo/name
  if not p.is_file() or p.suffix not in ('.h','.cc','.am'):continue
  s=p.read_text();t=s.replace('datalib/graph/process_corrs.h','datalib/graph/corr_graph.h')
  if t!=s:put(p,t)
for ext in ('h','cc'):(r/f'redstar/lib/process_corrs.{ext}').rename(r/f'redstar/lib/corr_graph_build.{ext}')
for name in subprocess.check_output(['git','-C',str(r/'redstar'),'ls-files'],text=True).splitlines():
 p=r/'redstar'/name
 if p.is_file() and p.suffix in ('.cc','.h','.am'):
  s=p.read_text();t=s.replace('process_corrs.h','corr_graph_build.h').replace('process_corrs.cc','corr_graph_build.cc')
  if s!=t:put(p,t)
# Migrate neutral legacy keys and codecs, retain only real ENSEM adapters in ADAT.
mapping={f'formfac/hadron_{n}pt_corr.h':f'datalib/legacy/hadron_{n}pt_corr.h' for n in (1,2,3)}
mapping.update({'hadron/particle_op.h':'datalib/legacy/particle_op.h',**{f'hadron/su2_corr/{n}.h':f'datalib/legacy/{n}.h' for n in ('hadron_npart_irrep_op','hadron_npart_irrep','hadron_npart_npt_corr')}})
new_sources=[];new_headers=[]
for old,new in mapping.items():
 p=a/'lib'/old;s=p.read_text();stem=p.stem;source=p.with_suffix('.cc');cc=source.read_text()
 h=s.replace('#include <ensem/ensem.h>','').replace('#include "ensem/ensem.h"','').replace('  using namespace ENSEM;','  using Datalib::Binary::read;\n  using Datalib::Binary::write;\n  using Datalib::Serial::read;\n  using Datalib::Serial::write;')
 # Ensemble typedefs stay in ADAT; the canonical header owns only keys.
 typedefs=re.findall(r'^  typedef[^;]+;',h,re.M);h=re.sub(r'^  typedef[^;]+;','',h,flags=re.M)
 for include in ('#include "ConfDataStoreDB.h"','#include <datalib/storage/filedb.h>'):h=h.replace(include,'')
 for orig,canon in mapping.items():
  h=h.replace('"'+orig+'"','<'+canon+'>').replace('"'+Path(orig).name+'"','<'+canon+'>')
 cc=re.sub(r'#include "'+re.escape(old)+r'"|#include "'+re.escape(p.name)+r'"','#include <'+new+'>',cc)
 cc=cc.replace('#include "hadron/irrep_util.h"','#include <datalib/graph/naming.h>')
 # Unused diagnostic overload depends on ENSEM::ComplexD, remove exactly this overload.
 match=re.search(r'^    std::ostream& operator<<\(std::ostream& os, const XMLArray::Array<ComplexD>& d\)',cc,re.M)
 if match:
  st,en=function_span(cc,match.start());cc=cc[:st]+cc[en:]
 put(d/'include'/new,h);put(d/'src'/('legacy_'+stem+'.cc'),cc);new_headers.append('include/'+new);new_sources.append('src/legacy_'+stem+'.cc')
 source.unlink()
 if typedefs:
  # This header is an ENSEM adapter, not a copy of the key/model.
  adapter='#pragma once\n#include <'+new+'>\n#include <ensem/ensem.h>\n#include <datalib/storage/filedb.h>\n#include <ConfDataStoreDB.h>\nnamespace FF {\n'+ '\n'.join(t.replace('VectorComplex','ENSEM::VectorComplex').replace('EnsemENSEM::','ENSEM::Ensem') for t in typedefs)+'\n}\n'
  put(p,adapter)
 else:p.unlink()
# Move pure irrep/name tables, shared momentum selection and operator label helpers.
p=a/'lib/hadron/irrep_util.cc';s=p.read_text();st=s.index('  //! Irrep names');st=s.rfind('  //-----',0,st);en=s.index('  std::string generateLittleGroup(const std::vector<int>& mom)');en=s.rfind('  //-----',0,en)
names_part=s[st:en];put(p,s[:st]+s[en:])
func_names=['generateLittleGroupMom','allowedMomentaRotation','allowedMomentaCombin','refMomCombin','refCanonMomCombin','refArbitraryMomCombin','generateLGRotation','isign','rotateMom','findRotateVec']
funcs=extract_funcs(p,func_names)
header=(a/'lib/hadron/irrep_util.h').read_text();public=header[header.index('  //---'):header.index('  //! Return rotation angles')];public=public[:public.rfind('  //---')]
# Name declarations and only moved momentum declarations, plus existing datalib declarations.
name_protos=re.findall(r'^  (?:std::string|int) [\w]+\([^;]+;',public,re.M)
name_protos=[x for x in name_protos if re.search(r'\b(getIrrepLG|getIrrepDim|buildIrrepWithParity|buildIrrepWithPG|removeHelicity|removeIrrepLG|getSingleHadronOpIrrep|getSingleHadronOpSpinIrrep|conjOpName|getIrrepParity|removeIrrepParity|removeIrrepGParity|removeIrrepEmbed|getIrrepEmbed|getIrrepGParity|getCubicRepNoParity)\(',x)]
protos=[f[:f.index('{')].strip()+';' for f in funcs]
rot=header[header.index('  struct RotateVec_t'):header.index('  std::vector<RotateVec_t> generateLGRotation')]
put(d/'include/datalib/graph/naming.h','#pragma once\n#include <datalib/graph/particle_class_op.h>\n#include <list>\nnamespace Hadron {\n'+rot+'\n'+'\n'.join(name_protos+protos)+'\n}\n')
helpers='''namespace {template<class T> XMLArray::Array<T> convertArray(const std::vector<T>& a){XMLArray::Array<T> b(a.size());for(int i=0;i<b.size();++i)b[i]=a[i];return b;} template<class T> std::vector<T> convertArray(const XMLArray::Array<T>& a){return {a.begin(),a.end()};} std::ostream& operator<<(std::ostream& o,const XMLArray::Array<int>& a){for(int i=0;i<a.size();++i){if(i)o<<" ";o<<a[i];}return o;} }\n'''
# Array lacks begin/end on some providers: preserve explicit conversion loops.
helpers=helpers.replace('return {a.begin(),a.end()};','std::vector<T> b(a.size());for(int i=0;i<a.size();++i)b[i]=a[i];return b;')
put(d/'src/naming.cc','#include <datalib/graph/naming.h>\n#include <map>\n#include <sstream>\n#include <iostream>\n#include <cstdlib>\nnamespace Hadron {\n'+helpers+names_part+'\n'.join(funcs)+'\n}\n')
put(a/'lib/hadron/irrep_util.h',header.replace(rot,'').replace('#include <xml_array.h>','#include <datalib/graph/naming.h>'))
# Move operator-name utility whole: it contains no ADAT code after the include change.
for ext in ('h','cc'):
 old=a/f'lib/hadron/operator_name_util.{ext}';new=d/('include/datalib/graph/operator_name.h' if ext=='h' else 'src/operator_name.cc');s=old.read_text().replace('#include "irrep_util.h"','#include <datalib/graph/naming.h>').replace('#include "operator_name_util.h"','#include <datalib/graph/operator_name.h>');put(new,s);old.unlink()
mapping['hadron/operator_name_util.h']='datalib/graph/operator_name.h'
# Move filenames, leaving FSq-specific formatter with ADAT.
p=a/'lib/hadron/ensem_filenames.cc';s=p.read_text();match=re.search(r'^  std::string ensemFileName\(const Hadron::KeyFSqDiscoOperator_t& key\)',s,re.M);assert match
st,en=function_span(s,match.start());fsq=s[st:en];s=s[:st]+s[en:]
s=s.replace('#include <ensem/ensem.h>','').replace('  using namespace ENSEM;','').replace('#include "hadron/unsmeared_fsq_disco.h"','').replace('#include "hadron/irrep_util.h"','#include <datalib/graph/naming.h>').replace('#include "hadron/operator_name_util.h"','#include <datalib/graph/operator_name.h>').replace('#include "ensem_filenames.h"','#include <datalib/graph/filenames.h>').replace('#include "hadron/hadron_sun_class_npart_npt_corr.h"','#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>')
put(d/'src/filenames.cc',s)
put(p,'#include "hadron/unsmeared_fsq_disco.h"\n#include <datalib/graph/filenames.h>\n#include <sstream>\nnamespace Hadron {\nstd::string shortMom(const XMLArray::Array<int>&);\nstd::string dispName(const std::vector<int>&);\nstd::string smearName(const std::string&);\n'+fsq+'\n}\n')
p=a/'lib/hadron/ensem_filenames.h';s=p.read_text().replace('#include "xml_array.h"','#include <datalib/model/xml_array.h>').replace('#include "hadron_sun_class_npart_npt_corr.h"','#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>');put(d/'include/datalib/graph/filenames.h',s);p.unlink();mapping['hadron/ensem_filenames.h']='datalib/graph/filenames.h'
# Canonical include migration for all source consumers. Formfac ENSEM adapters remain in ADAT.
for repo in ('datalib','adat','redstar','chroma','colorvec'):
 for p in (r/repo).rglob('*'):
  if not p.is_file() or p.suffix not in ('.h','.cc') or 'other_libs' in p.parts or '.git' in p.parts:continue
  s=p.read_text();t=s
  for old,new in mapping.items():
   if repo!='datalib' and old.startswith('formfac/'):continue
   t=t.replace('"'+old+'"','<'+new+'>')
   if p.parent==a/'lib/hadron':t=t.replace('"'+Path(old).name+'"','<'+new+'>')
  if s!=t:put(p,t)
# Neutralize the storage reader. std::vector<complex<double>> has the identical legacy length + pair layout.
p=a/'lib/io/superb_kv.h';s=p.read_text();conv_start=s.index('  /// Return a copy of VectorComplex');conv_end=s.index('  /// Tag type');converters=s[conv_start:conv_end]
s=s[:conv_start]+'''  inline std::vector<std::vector<std::complex<double>>> unpackLegacy(const std::vector<SerialDBData<std::vector<std::complex<double>>>>& values) {std::vector<std::vector<std::complex<double>>> result;result.reserve(values.size());for(const auto& v:values)result.push_back(v.data());return result;}
  inline std::vector<SerialDBData<std::vector<std::complex<double>>>> packLegacy(const std::vector<std::vector<std::complex<double>>>& values) {std::vector<SerialDBData<std::vector<std::complex<double>>>> result;result.reserve(values.size());for(const auto& v:values)result.emplace_back(v);return result;}
'''+s[conv_end:]
s=s.replace('__adat_superb_kv_h__','DATALIB_CORR_READER_H').replace('#include "ensem/ensem.h"','').replace('#include "hadron/hadron_sun_class_npart_npt_corr.h"','#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>').replace('namespace ADATIO','namespace Datalib::Storage').replace('ENSEM::VectorComplex','std::vector<std::complex<double>>').replace('.data().numElem()','.data().size()').replace('from_VectorComplex_to_normal','unpackLegacy').replace('from_normal_to_VectorComplex','packLegacy')
put(d/'include/datalib/storage/corr_reader.h',s)
put(p,'#pragma once\n#include <ensem/ensem.h>\n#include <datalib/storage/corr_reader.h>\nnamespace ADATIO {\n'+converters+'using Datalib::Storage::DontUseTheNewFormat;\nusing Datalib::Storage::OneCanUseTheNewFormat;\ntemplate<class K,class F=std::conditional_t<SB::has_schema<K>::value,OneCanUseTheNewFormat,DontUseTheNewFormat>>\nusing StorageCorrFunc=Datalib::Storage::StorageCorrFunc<K,F>;\n}\n')
# Move metadata library, MOD inspector, and executable; preserve legacy metadata namespace API.
for ext in ('h','cc'):
 p=a/f'main/dbutil/dbtype.{ext}';s=p.read_text().replace('#include "dbtype.h"','#include <datalib/storage/dbtype.h>').replace('#include "io/superb_kv.h"','#include <datalib/storage/corr_reader.h>').replace('#include "hadron/hadron_sun_npart_npt_corr.h"','#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>').replace('ADATIO::','Datalib::Storage::').replace('KeyHadronSUNNPartNPtCorr_t','KeyHadronSUNClassNPartNPtCorr_t')
 put(d/('include/datalib/storage/dbtype.h' if ext=='h' else 'src/dbtype.cc'),s);p.unlink()
p=a/'main/dbutil/dbutil.cc';s=p.read_text().replace('#include <ensem/ensem.h>','').replace('using namespace ENSEM;','').replace('#include "dbtype.h"','#include <datalib/storage/dbtype.h>').replace('#include "io/superb_kv.h"','#include <datalib/storage/corr_reader.h>').replace('ADATIO::','Datalib::Storage::').replace('#include "hadron/hadron_sun_npart_npt_corr.h"','').replace('#include "hadron/hadron_sun_class_npart_npt_corr.h"','#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>')
for old,new in mapping.items():s=s.replace('"'+old+'"','<'+new+'>')
put(d/'main/dbutil.cc',s);p.unlink();shutil.move(a/'main/dbutil/mod_reader.h',d/'main/mod_reader.h')
for name in ('test_mod_reader.py',):shutil.move(a/'main/dbutil/tests'/name,d/'tests'/name)
shutil.move(a/'main/dbutil/mod_reader.txt',d/'main/mod_reader.txt')
# All former metadata callers use the canonical declaration.
for repo in ('adat','redstar'):
 for p in (r/repo).rglob('*'):
  if p.is_file() and p.suffix in ('.h','.cc') and 'other_libs' not in p.parts:
   s=p.read_text();t=s.replace('#include "dbtype.h"','#include <datalib/storage/dbtype.h>').replace('#include <dbtype.h>','#include <datalib/storage/dbtype.h>')
   if s!=t:put(p,t)
# Move graph tools to datalib and remove all ADAT/Redstar imports/timer.
for name in ('view_corr_graph','dump_corr_graph'):
 p=r/f'redstar/src/{name}.cc';s=p.read_text()
 for inc in ('#include <ensem/ensem.h>','#include "xml_array.h"','#include "adat/adat_stopwatch.h"','#include "redstar_diagnostics.h"','  using namespace ENSEM;'):s=s.replace(inc,'')
 s=s.replace('#include "corr_graph_build.h"','#include <datalib/graph/corr_graph.h>\n#include <chrono>\nstruct GraphTimer {std::chrono::steady_clock::time_point t;double elapsed=0;void reset(){elapsed=0;}void start(){t=std::chrono::steady_clock::now();}void stop(){elapsed+=std::chrono::duration<double>(std::chrono::steady_clock::now()-t).count();}double getTimeInSeconds()const{return elapsed;}};').replace('Util::StopWatch','GraphTimer')
 put(d/f'main/{name}.cc',s);p.unlink()
# Build list updates.
p=a/'lib/Makefile.am';s=p.read_text()
for old in mapping:
 if old.startswith('formfac/'):continue
 s=re.sub(r'^\s*'+re.escape(old)+r'\s*\\?\n','',s,flags=re.M)
for old in list(mapping)[:7]:s=re.sub(r'^\s*'+re.escape(old[:-2]+'.cc')+r'\s*\\?\n','',s,flags=re.M)
s=re.sub(r'^\s*hadron/operator_name_util.cc\s*\\?\n','',s,flags=re.M)
s=s.replace('              -I@top_srcdir@/other_libs/filedb/src \\\n','').replace('              -I@top_srcdir@/other_libs/filedb/filehash \\\n','').replace('AM_CPPFLAGS = @DATALIB_CXXFLAGS@','AM_CPPFLAGS = @DATALIB_CXXFLAGS@ @DATALIB_FILEDB_CXXFLAGS@')
put(p,s)
p=a/'main/dbutil/Makefile.am';s=p.read_text();s=s.replace('nobase_include_HEADERS = dbtype.h','').replace('lib_LIBRARIES = libdbutil.a','').replace('libdbutil_a_SOURCES = \\\n    dbtype.cc','').replace('-ldbutil ','')
s=re.sub(r'\bdbutil\b(?= dbmeta)','',s);s=re.sub(r'^dbutil_SOURCES =.*\n','',s,flags=re.M);s=s.replace('tests/test_mod_reader.py','').replace('    mod_reader.txt ',' ')
s=re.sub(r'^\s*-I@top_srcdir@/other_libs/filedb/[^\n]*\n','',s,flags=re.M);s=re.sub(r'^\s*-L@top_builddir@/other_libs/filedb/[^\n]*\n','',s,flags=re.M)
s='AM_CPPFLAGS = @DATALIB_FILEDB_CXXFLAGS@\n'+s;put(p,s)
p=a/'configure.ac';s=p.read_text().replace('AC_SUBST([DATALIB_CXXFLAGS])','AC_SUBST([DATALIB_CXXFLAGS])\nAC_SUBST([DATALIB_FILEDB_CXXFLAGS])\nDATALIB_FILEDB_LDFLAGS=`"$DATALIB_CONFIG" --filedb-ldflags`\nDATALIB_FILEDB_LIBS=`"$DATALIB_CONFIG" --filedb-libs`\nAC_SUBST([DATALIB_FILEDB_LDFLAGS])\nAC_SUBST([DATALIB_FILEDB_LIBS])').replace('LDFLAGS="$LDFLAGS $DATALIB_LDFLAGS"','LDFLAGS="$LDFLAGS $DATALIB_LDFLAGS $DATALIB_FILEDB_LDFLAGS"').replace('LIBS="$DATALIB_LIBS $LIBS"','LIBS="$DATALIB_LIBS $DATALIB_FILEDB_LIBS $LIBS"');put(p,s)
p=a/'adat-config.in';s=p.read_text().replace('@DATALIB_CXXFLAGS@','@DATALIB_CXXFLAGS@ @DATALIB_FILEDB_CXXFLAGS@').replace('@DATALIB_LDFLAGS@','@DATALIB_LDFLAGS@ @DATALIB_FILEDB_LDFLAGS@').replace('-lfiledb -lfilehash','@DATALIB_FILEDB_LIBS@');put(p,s)
edit(r/'redstar/src/Makefile.am','-lredstar -ldbutil','-lredstar');p=r/'redstar/src/Makefile.am';s=p.read_text().replace(' dump_corr_graph view_corr_graph','');s=re.sub(r'^(dump_corr_graph|view_corr_graph)_SOURCES =.*\n','',s,flags=re.M);put(p,s)
# Canonical independent APIs and graph tools are always built; FILEDB enables the dbutil tool.
p=d/'Makefile.am';s=p.read_text();s+='\n# Shared models and diagnostic tools.\nlibdatalib_serial_a_SOURCES += '+' '.join(new_sources)+' src/naming.cc src/operator_name.cc src/filenames.cc\nnobase_datalibheaders_HEADERS += '+' '.join(new_headers)+' include/datalib/graph/naming.h include/datalib/graph/operator_name.h include/datalib/graph/filenames.h\nbin_PROGRAMS = view_corr_graph dump_corr_graph\nview_corr_graph_SOURCES = main/view_corr_graph.cc\nview_corr_graph_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@\ndump_corr_graph_SOURCES = main/dump_corr_graph.cc\ndump_corr_graph_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@\nif WITH_FILEDB\nlibdatalib_serial_a_SOURCES += src/dbtype.cc\nnobase_datalibheaders_HEADERS += include/datalib/storage/dbtype.h include/datalib/storage/corr_reader.h\nbin_PROGRAMS += dbutil\ndbutil_SOURCES = main/dbutil.cc main/mod_reader.h\ndbutil_CPPFLAGS = $(AM_CPPFLAGS) @SUPERBBLAS_CXXFLAGS@\ndbutil_LDFLAGS = @FILEDB_LDFLAGS@ @SUPERBBLAS_LDFLAGS@\ndbutil_LDADD = libdatalib_serial.a @FILEDB_LIBS@ @SUPERBBLAS_LIBS@ @DATALIB_DEP_LIBS@\nendif\nEXTRA_DIST += tests/test_mod_reader.py main/mod_reader.txt\n';put(p,s)
# Bundled FILEDB default, optional external installation, and explicit serial SuperbBLAS flags for dbutil.
p=d/'configure.ac';s=p.read_text().replace('[],[with_filedb=no])','[],[with_filedb=bundled])').replace('  FILEDB_CXXFLAGS="-I$with_filedb/include"','  AS_IF([test "x$with_filedb" = xbundled],[\n    FILEDB_CXXFLAGS=\'-I$(top_srcdir)/other_libs/filedb/src -I$(top_srcdir)/other_libs/filedb/filehash\'\n    FILEDB_LDFLAGS=\'-L$(top_builddir)/other_libs/filedb/src -L$(top_builddir)/other_libs/filedb/filehash\'\n    FILEDB_LIBS="-lfiledb -lfilehash -lpthread"\n  ],[\n  FILEDB_CXXFLAGS="-I$with_filedb/include"').replace('  LIBS="$datalib_saved_LIBS"\n])','  LIBS="$datalib_saved_LIBS"\n  ])\n])')
s=s.replace('AM_CONDITIONAL([WITH_FILEDB]', 'AM_CONDITIONAL([BUNDLED_FILEDB],[test "x$with_filedb" = xbundled])\nAS_IF([test "x$with_filedb" = xbundled],[AC_CONFIG_SUBDIRS([other_libs/filedb])])\nAM_CONDITIONAL([WITH_FILEDB]')
s=s.replace('AC_CONFIG_FILES([Makefile datalib-config])','AC_ARG_WITH([superbblas],AS_HELP_STRING([--with-superbblas=DIR],[Serial SuperbBLAS installation for tensor dbutil operations]),[SUPERBBLAS_CXXFLAGS="-DUSE_SUPERBBLAS -I$withval/include"; SUPERBBLAS_LDFLAGS="-L$withval/lib"; SUPERBBLAS_LIBS="-lsuperbblas"],[SUPERBBLAS_CXXFLAGS=; SUPERBBLAS_LDFLAGS=; SUPERBBLAS_LIBS=])\nAC_SUBST([SUPERBBLAS_CXXFLAGS])\nAC_SUBST([SUPERBBLAS_LDFLAGS])\nAC_SUBST([SUPERBBLAS_LIBS])\nAS_IF([test "x$with_filedb" = xbundled],[FILEDB_EXPORT_CXXFLAGS="-I$prefix/include";FILEDB_EXPORT_LDFLAGS="-L$prefix/lib"],[FILEDB_EXPORT_CXXFLAGS="$FILEDB_CXXFLAGS";FILEDB_EXPORT_LDFLAGS="$FILEDB_LDFLAGS"])\nAC_SUBST([FILEDB_EXPORT_CXXFLAGS])\nAC_SUBST([FILEDB_EXPORT_LDFLAGS])\nAC_CONFIG_FILES([Makefile datalib-config])')
put(p,s)
p=d/'Makefile.am';s=p.read_text();s='if BUNDLED_FILEDB\nSUBDIRS = other_libs/filedb .\nelse\nSUBDIRS = .\nendif\n'+s;put(p,s)
p=d/'datalib-config.in';s=p.read_text().replace('@FILEDB_CXXFLAGS@','@FILEDB_EXPORT_CXXFLAGS@').replace('@FILEDB_LDFLAGS@','@FILEDB_EXPORT_LDFLAGS@');put(p,s)
# Record the source migration map for review and follow-up audits.
put(e/'extraction-map.json',json.dumps(mapping,indent=2)+'\n')
print('Source extraction complete; validate generated build rules before committing.')
