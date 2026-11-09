from pathlib import Path
import subprocess,re,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');q=r/'qdpxx';ref=r.parent/'redstar_kanoni.devel/runs/no1pt-clean-20261103-002/sources/qdpxx';c=r/'chroma';e=r/'evidence/tools-filedb-20261109-001'
subprocess.run(['git','clone','--no-local',str(ref),str(q)],check=True)
subprocess.run(['git','-C',str(q),'switch','-c','rge/devel-datalib-reorg'],check=True)
subprocess.run(['git','-C',str(q),'remote','set-url','origin','git@github.com:usqcd-software/qdpxx.git'],check=True)
# Relocate QDP-aware communication wrappers to Chroma, preserving algorithms and wire formats.
s=(q/'include/qdp_db_imp.h').read_text().replace('QDP_DB_IMP_H','CHROMA_FILEDB_QDP_H').replace('#include "qdp_layout.h"','#include "chromabase.h"').replace('namespace QDP','namespace Chroma').replace('end namespace QDP','end namespace Chroma')
(c/'lib/util/ferm/filedb_qdp.h').write_text(s)
p=c/'lib/util/ferm/key_val_db.h';p.write_text(p.read_text().replace('#include "qdp_db.h"','#include "util/ferm/filedb_qdp.h"'))
p=c/'lib/Makefile.am';s=p.read_text();s+='\nnobase_include_HEADERS += util/ferm/filedb_qdp.h\n';p.write_text(s)
# All Chroma database features now obtain their FILEDB headers/libs from datalib.
p=c/'configure.ac';s=p.read_text();mark='AS_IF([test "x$DATALIB_PREFIX" != xno], [';idx=s.index(mark);s=s[:idx]+'AS_IF([test "x$DATALIB_PREFIX" = xno],[AC_MSG_ERROR([--with-datalib=DIR is required for Chroma FILEDB support])])\n'+s[idx:];p.write_text(s)
# Remove submodule and all real/stub wrappers from QDP; no QDP FILEDB dependency remains.
subprocess.run(['git','-C',str(q),'rm','other_libs/filedb','include/qdp_db.h','include/qdp_db_imp.h','include/qdp_db_stub.h','examples/t_db.cc'],check=True)
p=q/'.gitmodules';p.write_text(re.sub(r'\[submodule "other_libs/filedb"\]\n(?:[^\[].*\n)*','',p.read_text()))
p=q/'configure.ac';s=p.read_text();start=s.index('dnl database wrapper');end=s.index('dnl --enable-sse',start);s=s[:start]+s[end:];start=s.index('dnl Build filedb');end=s.index('dnl Use Jie Chens threads library',start);s=s[:start]+s[end:];p.write_text(s)
p=q/'qdp++-config.in';s=p.read_text();start=s.index('filedb_dir=');end=s.index('precision=',start);p.write_text(s[:start]+s[end:])
p=q/'other_libs/Makefile.am';p.write_text(p.read_text().replace(' @FILEDB_DIR@','').replace('libintrin filedb','libintrin'))
for filename in ('include/Makefile.am','include/CMakeLists.txt'):
 p=q/filename;s=p.read_text();s=re.sub(r'^.*qdp_db(?:_imp|_stub)?\.h.*\n','',s,flags=re.M);p.write_text(s)
p=q/'include/qdp_config_internal.h.cmake.in';p.write_text(re.sub(r'^.*BUILD_FILEDB.*\n','',p.read_text(),flags=re.M))
p=q/'examples/Makefile.am';s=p.read_text();s=re.sub(r'if BUILD_FILEDB\n.*?endif\n','',s,flags=re.S);s=re.sub(r'\bt_db\b','',s);p.write_text(s)
p=q/'CMakeLists.txt';s=p.read_text();s=re.sub(r'^option\(QDP_USE_FILEDB[^\n]*\n','',s,flags=re.M);s=re.sub(r'if\s*\(\s*QDP_USE_FILEDB\s*\).*?endif\(\)','',s,flags=re.S|re.I);p.write_text(s)
p=q/'lib/CMakeLists.txt';s=p.read_text();s=re.sub(r'if\s*\(\s*QDP_USE_FILEDB\s*\).*?endif\(\)','',s,flags=re.S|re.I);p.write_text(s)
# Initialize remaining exact pinned submodules from existing local objects only.
for line in subprocess.check_output(['git','-C',str(q),'config','-f','.gitmodules','--get-regexp',r'^submodule\..*\.path$'],text=True).splitlines():
 key,path=line.split(None,1);subprocess.run(['git','-C',str(q),'config',key[:-4]+'url',str(ref/path)],check=True)
subprocess.run(['git','-C',str(q),'-c','protocol.file.allow=always','submodule','update','--init','--recursive'],check=True)
(e/'qdpxx-base.json').write_text(json.dumps({'source':str(ref),'base_commit':subprocess.check_output(['git','-C',str(q),'rev-parse','HEAD'],text=True).strip(),'new_checkout':str(q)},indent=2)+'\n')
print('QDP FILEDB removed; communication adapter relocated to Chroma.')
