from pathlib import Path
import subprocess,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');c=r/'chroma';e=r/'evidence/chroma-storage-20261109-001'
paths=['lib/meas/inline/hadron/inline_stoch_hadron_w.cc','lib/meas/inline/hadron/inline_stoch_hadron_w.h']
for rel in paths:(c/rel).unlink()
for rel in ('lib/Makefile.am','lib/CMakeLists.txt','lib/meas/inline/hadron/inline_hadron_aggregate.cc'):
 p=c/rel;s=p.read_text();s='\n'.join(line for line in s.splitlines() if 'inline_stoch_hadron_w.' not in line and 'InlineStochHadronEnv::registerAll' not in line)+'\n';p.write_text(s);paths.append(rel)
p=c/'docs/removals.md';p.write_text('''# Reorg removal list

These removals apply to `rge/devel-datalib-reorg`.

| Component | Status | Decision |
| --- | --- | --- |
| `STOCH_HADRON` (`InlineStochHadronEnv`) | Removed, 9 November 2026 | User-directed removal of the legacy noise-dilution meson/baryon operator measurement. Its source, public header, registration and Autotools/CMake entries are deleted. No compatibility implementation, new storage adapter or format migration is provided for it. |

Inputs requesting `STOCH_HADRON` are no longer accepted by Chroma's measurement
factory. Historical ChangeLog entries remain as history. Other stochastic
measurements and the current distillation/SUPERB producers are separate code.
''');paths.append('docs/removals.md')
p=c/'README';s=p.read_text()+'\nReorg removal decisions: docs/removals.md\n';p.write_text(s);paths.append('README')
p=c/'Makefile.am';p.write_text(p.read_text()+'\nEXTRA_DIST += docs/removals.md\n');paths.append('Makefile.am')
subprocess.run(['git','-C',str(c),'add','--',*paths],check=True);subprocess.run(['git','-C',str(c),'diff','--cached','--check'],check=True);subprocess.run(['git','-C',str(c),'commit','-m','Remove obsolete STOCH_HADRON measurement'],check=True)
(e/'stoch-hadron-removal.json').write_text(json.dumps({'status':'source removed','commit':subprocess.check_output(['git','-C',str(c),'rev-parse','HEAD'],text=True).strip(),'removed_files':paths[:2],'reason':'Explicit user direction; no further support or modernization.','historical_changelog_retained':True},indent=2)+'\n')
print('STOCH_HADRON removed from source, registration and both build systems; removal list recorded')
