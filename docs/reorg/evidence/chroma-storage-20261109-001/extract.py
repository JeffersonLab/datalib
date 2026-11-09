from pathlib import Path
import subprocess,re,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';c=r/'chroma';a=r/'adat';e=r/'evidence/chroma-storage-20261109-001'
def put(p,s):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(s)
# Serialized record transport shared by Eloy's typed storage classes. No QDP/ADAT code.
put(d/'include/datalib/storage/record_store.h',r'''#pragma once
#include <ConfDataStoreDBSimple.h>
#include <memory>
#include <string>
#include <vector>
namespace Datalib::TensorStorage {
struct RecordOptions {
  unsigned int cache_bytes = 0;
  unsigned int metadata_capacity = 0;
  unsigned int configurations = 0;
  unsigned int buckets = 32;
};
// Rank-local storage. The consumer owns serialization and communication.
// Encoded records are stored verbatim, preserving all historical key/value formats.
class StorageRecords {
  struct Bytes {
    std::string value;
    void writeObject(std::string& out) const { out = value; }
    void readObject(const std::string& in) { value = in; }
  };
  RecordOptions options_;
  std::shared_ptr<FILEDB::ConfDataStoreDBSimple> records_;
public:
  RecordOptions& record_options() { return options_; }
  static bool record_file_exists(const std::string& file) {
    return FILEDB::ConfDataStoreDBSimple::fileExists(file);
  }
  int open_records(const std::string& file, int flags, int mode) {
    close_records();
    auto next = std::make_shared<FILEDB::ConfDataStoreDBSimple>();
    next->setCacheSize(options_.cache_bytes);
    next->setMaxNumberConfigs(options_.configurations);
    next->setNumberBuckets(options_.buckets);
    next->setMaxUserInfoLen(options_.metadata_capacity);
    int status = next->open(file, flags, mode);
    if (status == 0) records_ = std::move(next);
    return status;
  }
  int write_metadata(const std::string& metadata) {
    return records_ ? records_->insertUserdata(metadata) : -1;
  }
  int read_metadata(std::string& metadata) const {
    return records_ ? records_->getUserdata(metadata) : -1;
  }
  int put_record(const std::string& key, const std::string& value) {
    return records_ ? records_->insert(key, value) : -1;
  }
  int get_record(const std::string& key, std::string& value) const {
    if (!records_) return -1;
    Bytes k{key}, v;
    int status = records_->get(k, v);
    if (status == 0) value = std::move(v.value);
    return status;
  }
  std::vector<std::string> record_keys() const {
    std::vector<std::string> result;
    if (records_) {
      std::vector<Bytes> keys;
      records_->keys<Bytes,Bytes>(keys);
      for (auto& key : keys) result.push_back(std::move(key.value));
    }
    return result;
  }
  int close_records() {
    if (!records_) return 0;
    int status = records_->close();
    records_.reset();
    return status;
  }
};
}
''')
for filename,cls in [('meson_superb.h','StorageMeson'),('baryon_superb.h','StorageBaryon'),('prop_superb.h','StorageProp4'),('genprop4.h','StorageGenprop4'),('disco_superb.h','StorageDisco'),('glueball_superb.h','StorageGlue'),('tetraquark_superb.h','StorageTetraQuark')]:
 p=d/'include/datalib/superb'/filename;s=p.read_text();s='#include "datalib/storage/record_store.h"\n'+s;s=s.replace('class '+cls+'\n','class '+cls+' : public StorageRecords\n').replace('class '+cls+' {','class '+cls+' : public StorageRecords {');assert ': public StorageRecords' in s;p.write_text(s)
p=d/'Makefile.am';s=p.read_text();s+='\nif WITH_FILEDB\nnobase_datalibheaders_HEADERS += include/datalib/storage/record_store.h\nendif\n';p.write_text(s)
p=d/'datalib-config.in';s=p.read_text().replace('--filedb-cxxflags|--filedb-ldflags|--filedb-libs)', '--filedb-cxxflags|--filedb-ldflags|--filedb-libs|--storage-cxxflags|--storage-ldflags|--storage-libs)').replace('--filedb-cxxflags) echo','--filedb-cxxflags|--storage-cxxflags) echo').replace('--filedb-ldflags) echo','--filedb-ldflags|--storage-ldflags) echo').replace('--filedb-libs) echo','--filedb-libs|--storage-libs) echo');p.write_text(s)
# Free codec function at the QDP boundary, without a database/key/value wrapper.
put(c/'lib/util/ferm/storage_io.h',r'''#pragma once
#include "chromabase.h"
#include <datalib/storage/record_store.h>
#ifdef BUILD_SB
# ifndef USE_SUPERBBLAS_WITH_GPU_SUPPORT
#  define USE_SUPERBBLAS_WITH_GPU_SUPPORT
# endif
#endif
namespace Chroma {
template<class T> std::string encodeStorageRecord(const T& value) {
  QDP::LocalBinaryBufferWriter out;
  write(out, value);
  return out.strPrimaryNode();
}
}
''')
# Scan balanced punctuation (comments/string literals excluded).
def pairs(s,op='{',cl='}'):
 stack=[];result={};state=None;i=0
 while i<len(s):
  ch=s[i];n=s[i:i+2]
  if state=='line':
   if ch=='\n':state=None
  elif state=='block':
   if n=='*/':state=None;i+=1
  elif state in ('"',"'"):
   if ch=='\\':i+=1
   elif ch==state:state=None
  elif n=='//':state='line';i+=1
  elif n=='/*':state='block';i+=1
  elif ch in ('"',"'"):state=ch
  elif ch==op:stack.append(i)
  elif ch==cl and stack:result[stack.pop()]=i
  i+=1
 return result
def code_mask(s):
 out=list(s);state=None;i=0
 while i<len(s):
  ch=s[i];n=s[i:i+2]
  if state=='line':
   if ch=='\n':state=None
   else:out[i]=' '
  elif state=='block':
   out[i]=' '
   if n=='*/':out[i+1]=' ';state=None;i+=1
  elif state in ('"',"'"):
   out[i]=' '
   if ch=='\\':
    if i+1<len(s):out[i+1]=' '
    i+=1
   elif ch==state:state=None
  elif n=='//':out[i]=out[i+1]=' ';state='line';i+=1
  elif n=='/*':out[i]=out[i+1]=' ';state='block';i+=1
  elif ch in ('"',"'"):out[i]=' ';state=ch
  i+=1
 return ''.join(out)
def live_matches(pattern,s):
 mask=code_mask(s)
 return [m for m in re.finditer(pattern,s) if not mask[m.start()].isspace()]
def angle_end(s,start):
 dep=0
 for i in range(start,len(s)):
  if s[i]=='<':dep+=1
  elif s[i]=='>':
   dep-=1
   if dep==0:return i
 raise ValueError('unclosed template')
def storage_class(key,file):
 for word,cls in [('GenProp','StorageGenprop4'),('Meson','StorageMeson'),('Baryon','StorageBaryon'),('PropElemental','StorageProp4'),('Glue','StorageGlue')]:
  if word in key:return cls
 if 'disco' in file:return 'StorageDisco'
 return 'StorageRecords'
headers={'StorageMeson':'meson_superb.h','StorageBaryon':'baryon_superb.h','StorageProp4':'prop_superb.h','StorageGenprop4':'genprop4.h','StorageDisco':'disco_superb.h','StorageGlue':'glueball_superb.h'}
inventory={}
files=[c/p for p in subprocess.check_output(['git','-C',str(c),'ls-files'],text=True).splitlines() if Path(p).suffix in ('.h','.cc') and not p.startswith('other_libs/')]
for p in files:
 if not p.exists() or p.name in ('filedb_qdp.h','key_val_db.h'):continue
 s=p.read_text();original=s;collective=False;used=set();decls=[]
 # Replace complete database types before removing serializer wrappers.
 for m in live_matches(r'\b(LocalBinaryStoreDB|BinaryStoreDB)\s*<',s)[::-1]:
  st=m.start();end=angle_end(s,s.index('<',st));text=s[st:end+1];cls=storage_class(text,p.name);used.add(cls)
  if m.group(1)=='BinaryStoreDB':collective=True
  s=s[:st]+'Datalib::TensorStorage::'+cls+s[end+1:]
 # Unwrap the value holders; only named locals get accessor changes in their lexical scope.
 scopes=pairs(s);spans=[]
 for m in live_matches(r'(?<![:\w])(?:Local)?SerialDB(Key|Data)\s*<',s):
  end=angle_end(s,s.index('<',m.start()));typ=s[s.index('<',m.start())+1:end];tail=s[end+1:];binding=re.match(r'\s*(\w+)\s*(?:[;({=])',tail)
  if not binding:raise RuntimeError((p,s[m.start():end+70]))
  name=binding.group(1);enclosing=[(st,en) for st,en in scopes.items() if st<m.start()<en];en=min(en for st,en in enclosing) if enclosing else len(s)
  spans.append((m.start(),end+1,typ,name,'key' if m.group(1)=='Key' else 'data',en))
 # Apply accessor substitutions within the original scopes, then template removals from right to left.
 replacements=[]
 for st,en,typ,name,method,scope_end in spans:
  replacements.append((st,en,typ))
  pattern=r'\b'+re.escape(name)+r'\.'+method+r'\(\)'
  for match in re.finditer(pattern,s[en:scope_end]):replacements.append((en+match.start(),en+match.end(),name))
 for st,en,new in sorted(set(replacements),reverse=True):s=s[:st]+new+s[en:]
 s=s.replace('.key.key()','.key').replace('.val.data()','.val')
 s=s.replace('"util/ferm/key_val_db.h"','"util/ferm/storage_io.h"')
 if used:
  # Collective constructors previously set these storage defaults.
  if collective:
   s=re.sub(r'(\bqdp_db\s*(?:\{\})?\s*;)',r'\1\n      qdp_db.record_options().cache_bytes = 50*1024*1024;\n      qdp_db.record_options().configurations = 1;',s) if 'std::vector<Datalib::TensorStorage::' not in s else s
   s=re.sub(r'(qdp_db\.resize\(1\);)',r'\1\n        qdp_db[0].record_options().cache_bytes = 50*1024*1024;\n        qdp_db[0].record_options().configurations = 1;',s)
  # Rewrite conditional file probes explicitly at the rank boundary.
  parens=pairs(s,'(',')');fix=[];count=0
  for m in re.finditer(r'if\s*\(\s*!\s*(qdp\w*_db|qdp_db)(?:\[0\])?\.fileExists\(',s):
   call=s.index('.fileExists(',m.start());op=call+len('.fileExists');end=parens[op];arg=s[op+1:end];count+=1;name='record_file_exists_'+str(count)
   if collective:
    pre=f'bool {name} = false;\n      if (Layout::primaryNode()) {name} = Datalib::TensorStorage::StorageRecords::record_file_exists({arg});\n      QDPInternal::broadcast({name});\n      '
   else:pre=f'bool {name} = Datalib::TensorStorage::StorageRecords::record_file_exists({arg});\n      '
   fix.append((m.start(),end+1,pre+'if (!'+name))
  for st,en,new in reversed(fix):s=s[:st]+new+s[en:]
  # All store operations use the shared storage API; collective paths execute only on primary.
  parens=pairs(s,'(',')');fix=[];ops={};pattern=r'\b((?:qdp_db|qdp4_db)(?:\[[^\]]+\])?)\.(\w+)\('
  for m in re.finditer(pattern,s):
   obj,method=m.group(1),m.group(2)
   if method in ('resize','size','record_options'):continue
   op=m.end()-1;en=parens[op];arg=s[op+1:en];semi=en+1
   while semi<len(s) and s[semi].isspace():semi+=1
   if semi>=len(s) or s[semi]!=';':raise RuntimeError((p,method,s[m.start():en+20]))
   guard='Layout::primaryNode() && ' if collective else ''
   if method in ('setMaxUserInfoLen','setNumberBuckets'):
    field={'setMaxUserInfoLen':'metadata_capacity','setNumberBuckets':'buckets'}[method];new=(('if (Layout::primaryNode()) ' if collective else '')+obj+'.record_options().'+field+' = '+arg+';')
   else:
    mapped={'open':'open_records','insert':'put_record','insertUserdata':'write_metadata','close':'close_records'}[method]
    if method=='insert':
     # Find the top-level comma, allowing commas in template arguments/calls.
     dep=0;sep=None
     for k,ch in enumerate(arg):
      if ch in '([{':dep+=1
      elif ch in ')]}':dep-=1
      elif ch==',' and dep==0:sep=k;break
     assert sep is not None,(p,arg);arg='encodeStorageRecord('+arg[:sep].strip()+'), encodeStorageRecord('+arg[sep+1:].strip()+')'
    new='if ('+guard+obj+'.'+mapped+'('+arg+') != 0) QDP_error_exit("record storage '+method+' failed");'
   fix.append((m.start(),semi+1,'{ '+new+' }'));ops[method]=ops.get(method,0)+1
  for st,en,new in reversed(fix):s=s[:st]+new+s[en:]
  # Local vector close loops have an alias rather than the qdp_db name.
  s=re.sub(r'\bdb\.close\(\);',r'if (db.close_records() != 0) QDP_error_exit("record storage close failed");',s) if 'for (auto& db : qdp' in s else s
  includes='\n'.join('#include <datalib/superb/'+headers[x]+'>' for x in sorted(used) if x in headers)
  if includes:s=s.replace('#include "util/ferm/storage_io.h"','#include "util/ferm/storage_io.h"\n'+includes)
  # Some producers get codec declarations indirectly from their key headers.
  if '#include "util/ferm/storage_io.h"' not in s:s='#include "util/ferm/storage_io.h"\n'+includes+'\n'+s
  inventory[str(p.relative_to(c))]={'classes':sorted(used),'collective_primary_boundary':collective,'operations':ops,'value_bindings':len(spans)}
 if s!=original:put(p,s)
# Remove the backend wrappers entirely, including public header exports.
for name in ('filedb_qdp.h','key_val_db.h'):(c/'lib/util/ferm'/name).unlink()
p=c/'lib/Makefile.am';s=p.read_text();s=re.sub(r'^.*util/ferm/(?:filedb_qdp|key_val_db)\.h.*\n','',s,flags=re.M);s+='\nnobase_include_HEADERS += util/ferm/storage_io.h\n';p.write_text(s)
p=c/'configure.ac';s=p.read_text().replace('--filedb-','--storage-').replace('Chroma FILEDB support','Chroma storage support').replace("datalib's FILEDB component","datalib's storage component");p.write_text(s)
p=c/'chroma-config.in';s=p.read_text().replace('chroma_cxxflags="@CXXFLAGS@','chroma_cxxflags="@CXXFLAGS@ @REDSTAR_DATALIB_CXXFLAGS@');p.write_text(s)
p=c/'lib/util/ferm/superb_contractions.h';p.write_text(p.read_text().replace('FILEDB','MOD'))
# Clear remaining member accessor uses in callers of the public key/value buffer structs.
for p in files:
 if p.exists():
  s=p.read_text();t=s.replace('.key.key()','.key').replace('.val.data()','.val')
  if s!=t:put(p,t)
# Rename ADAT's remaining utilities, preventing the misleading source-directory name.
(a/'main/dbutil').rename(a/'main/dbtools')
for rel in ('main/Makefile.am','main/CMakeLists.txt','configure.ac'):
 p=a/rel;put(p,p.read_text().replace('main/dbutil','main/dbtools').replace('SUBDIRS = dbutil','SUBDIRS = dbtools').replace('add_subdirectory(dbutil)','add_subdirectory(dbtools)'))
(e/'migration-inventory.json').write_text(json.dumps(inventory,indent=2)+'\n')
print('Migrated',len(inventory),'producer files to shared typed storage; wrapper headers removed.')
