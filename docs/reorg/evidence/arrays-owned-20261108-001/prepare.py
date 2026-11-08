import hashlib
import json
from pathlib import Path
import re
import shlex
import shutil
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
e.mkdir(exist_ok=True)
for name in ('sources','build','install','before','frozen-arrays'):(e/name).mkdir(exist_ok=True)
def git(repo,*args):return subprocess.check_output(['git','--no-optional-locks','-C',str(repo),*args],text=True)
protected={}
for name in ('adat.devel','redstar.devel','chroma.devel','colorvec.devel','redstar_kanoni.devel'):
    repo=root.parent/name
    item={'devel':git(repo,'rev-parse','refs/heads/devel')}
    if name=='redstar_kanoni.devel':
        item.update(status=git(repo,'status','--porcelain=v1','--branch'),diff=hashlib.sha256(git(repo,'diff','--binary','HEAD').encode()).hexdigest())
    protected[name]=item
(e/'protected-before.json').write_text(json.dumps(protected,indent=2)+'\n')
state={}
for name in ('datalib','adat','colorvec','redstar','chroma','xpath_reader'):
    repo=root/name
    assert git(repo,'branch','--show-current').strip()=='rge/devel-datalib-reorg'
    state[name]={'head':git(repo,'rev-parse','HEAD').strip(),'status':git(repo,'status','--short')}
(e/'reorg-before.json').write_text(json.dumps(state,indent=2)+'\n')
# Freeze additional rank-3/4 wire coverage before editing any source.
config=root/'evidence/standalone-clean-20261107-001/install/datalib-core/bin/datalib-config'
def flags(option):return shlex.split(subprocess.check_output([str(config),'--'+option],text=True))
probe=Path(__file__).parent/'arrays_probe.cc'
args=['/usr/bin/clang++','-DLEGACY_ARRAY_HEADERS',*flags('cxxflags'),str(probe),'-o',str(e/'reference-arrays'),*flags('ldflags'),*flags('libs')]
with (e/'freeze-build.log').open('w') as log:subprocess.run(args,stdout=log,stderr=subprocess.STDOUT,check=True)
with (e/'freeze-run.log').open('w') as log:subprocess.run([str(e/'reference-arrays'),'emit',str(e/'frozen-arrays')],stdout=log,stderr=subprocess.STDOUT,check=True)
(e/'freeze-command.json').write_text(json.dumps(args,indent=2)+'\n')
fixture_manifest={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in (e/'frozen-arrays').iterdir()}
(e/'frozen-arrays/manifest.json').write_text(json.dumps({'producer':'Unmodified datalib 3a87bcc / clean-installed b3b1d42 code, external xpath_reader 05cbc2d6','files':fixture_manifest},indent=2)+'\n')

repo=root/'datalib'
assert not git(repo,'diff','--name-only').strip() and not git(repo,'diff','--cached','--name-only').strip()
upstream=root/'xpath_reader'
assert git(upstream,'rev-parse','HEAD').strip()=='05cbc2d6e03cfe32e88433d3a3df77c6fd69f244'
records=[]
headers=('xml_array.h','xml_array2d.h','xml_array3d.h','xml_array4d.h')
for name in headers:
    original=subprocess.check_output(['git','-C',str(upstream),'show','HEAD:include/'+name])
    owned=re.sub(rb'(#include\s*)[<"](xml_array(?:2d|3d|4d)?\.h)[>"]',rb'\1<datalib/model/\2>',original)
    target=repo/'include/datalib/model'/name
    assert not target.exists()
    target.write_bytes(owned)
    records.append({'source':'include/'+name,'source_commit':state['xpath_reader']['head'],'source_sha256':hashlib.sha256(original).hexdigest(),
                    'destination':str(target.relative_to(repo)),'extracted_sha256':hashlib.sha256(owned).hexdigest(),
                    'change':'Internal array include path relocation only; namespaces, guards, class bodies and operators unchanged.'})
for folder in ('include','src','tests'):
    for p in (repo/folder).rglob('*'):
        if p.is_file() and p.suffix in ('.cc','.h'):
            before=p.read_bytes()
            after=re.sub(rb'(#include\s*)[<"](xml_array(?:2d|3d|4d)?\.h)[>"]',rb'\1<datalib/model/\2>',before)
            if after!=before:
                backup=e/'before/datalib'/p.relative_to(repo);backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(before)
                p.write_bytes(after)
(repo/'array-provenance.json').write_text(json.dumps({'upstream':'usqcd-software/xpath_reader','license':'COPYING.xpath_reader','headers':records,
 'compatibility':'XMLArray public types and legacy guards are retained for ADAT/QDP type identity. QDP has existing rank-1 empty equality/operator!= differences; Chroma selects its QDP headers explicitly before datalib. No array algorithms or formats changed.'},indent=2)+'\n')
p=repo/'configure.ac';s=p.read_text()
s=re.sub(r'^AC_ARG_WITH\(\[xpath\].*\n','',s,flags=re.M)
s=re.sub(r'^AC_SUBST\(\[XPATH_CXXFLAGS\].*\n','',s,flags=re.M)
s=s.replace('CPPFLAGS="$CPPFLAGS $XPATH_CXXFLAGS"\n','')
p.write_text(s)
p=repo/'datalib-config.in';p.write_text(p.read_text().replace(' @XPATH_CXXFLAGS@',''))
p=repo/'Makefile.am';s=p.read_text().replace(' @XPATH_CXXFLAGS@','')
s+='\n# Array definitions are owned by datalib; no external xpath_reader headers.\nnobase_datalibheaders_HEADERS += '+' '.join('include/datalib/model/'+name for name in headers)+'\nEXTRA_DIST += array-provenance.json\n'
s+='\ncheck_PROGRAMS += arrays_probe\narrays_probe_SOURCES = tests/arrays_probe.cc\narrays_probe_LDADD = libdatalib_serial.a @DATALIB_DEP_LIBS@\nTESTS += tests/check-arrays.sh\nEXTRA_DIST += tests/check-arrays.sh tests/fixtures/arrays-ranks.bin tests/fixtures/arrays-ranks.xml tests/fixtures/arrays-ranks-reference.json\n'
p.write_text(s)
shutil.copy2(probe,repo/'tests/arrays_probe.cc')
(repo/'tests/check-arrays.sh').write_text('#!/bin/sh\nset -eu\n./arrays_probe check "$srcdir/tests/fixtures"\n')
(repo/'tests/check-arrays.sh').chmod(0o755)
for name in ('arrays-ranks.bin','arrays-ranks.xml'):shutil.copy2(e/'frozen-arrays'/name,repo/'tests/fixtures'/name)
shutil.copy2(e/'frozen-arrays/manifest.json',repo/'tests/fixtures/arrays-ranks-reference.json')
p=repo/'README.md';s=p.read_text().replace('The serial build needs the standalone xpath_reader array headers, libxml2,\na C++20 compiler, and OpenMP.', 'The serial build needs libxml2, a C++20 compiler, and OpenMP. The array\nheaders are installed under `<datalib/model/xml_array*.h>` and retain the\nexisting `XMLArray` public types; no xpath_reader installation is needed.')
s=s.replace('  --with-xpath=/path/to/xpath_reader --with-libxml2=/path/to/libxml2 \\\n','  --with-libxml2=/path/to/libxml2 \\\n')
p.write_text(s)
p=repo/'runtime-provenance.json';s=json.loads(p.read_text());s['xml']['notes']='Parser/writer isolated under Datalib::Serial::Detail. Array headers are bundled under datalib/model and require no external xpath_reader installation; see array-provenance.json.';p.write_text(json.dumps(s,indent=2)+'\n')
p=repo/'src/xml/README.md';p.write_text(p.read_text()+'\nThe rank-1 through rank-4 array headers from that same source revision are\ninstalled under include/datalib/model. Their existing XMLArray namespace and\nguards are retained for consumer type identity. See array-provenance.json.\n')
for name in ('inline_corr_superb_w.cc','inline_disco_prob_3d_defl_superb_w.cc'):
    p=root/'chroma/lib/meas/inline/hadron'/name
    before=p.read_bytes();backup=e/'before/chroma'/p.relative_to(root/'chroma');backup.parent.mkdir(parents=True,exist_ok=True);backup.write_bytes(before)
    s=before.decode();position=s.index('#include "datalib/')
    s=s[:position]+'// Keep QDP\'s XMLArray definitions at the Chroma boundary.\n'+''.join('#include <'+header+'>\n' for header in headers)+s[position:]
    p.write_text(s)
(e/'array-provenance.json').write_bytes((repo/'array-provenance.json').read_bytes())
print('Frozen reference arrays, bundled four headers, removed xpath build flags and preserved Chroma header selection.')
