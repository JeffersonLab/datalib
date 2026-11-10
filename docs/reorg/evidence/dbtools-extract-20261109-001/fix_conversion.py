from pathlib import Path
import subprocess,shlex,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');d=r/'datalib';e=r/'evidence/dbtools-extract-20261109-001'
addition='''    // Register the complete output key dictionary before writing any values.
    std::vector<Hadron::KeyHadronSUNClassNPartNPtCorr_t> converted_keys;
    converted_keys.reserve(old_keys.size());
    for (const auto& key : old_keys)
      converted_keys.push_back(Hadron::convertHadronNPart(key));
    corr_db.insert_keys(converted_keys);

'''
for p in [d/'main/dbconvert_corr_util.cc',e/'sources/datalib/main/dbconvert_corr_util.cc']:
 s=p.read_text();needle='    // Get keys, convert them, and write out the correlator\n    for';assert needle in s;s=s.replace(needle,addition+needle);p.write_text(s)
 subprocess.run(['/opt/homebrew/bin/clang-format','-style=file:/Users/admin/.clang-format','-i',str(p)],check=True)
s=(e/'before/adat/main/dbtools/dbconvert_corr_util.cc').read_text();needle='    // Get keys, convert them, and write out the correlator\n    for';assert needle in s;s=s.replace(needle,addition+needle)
source=e/'baseline-dbconvert-fixed.cc';source.write_text(s)
cfg=r/'evidence/chroma-storage-20261109-001/install/adat/bin/adat-config'
def flags(name):return shlex.split(subprocess.check_output([cfg,'--'+name],text=True))
cmd=['/usr/bin/clang++',*flags('cxxflags'),str(source),'-o',str(e/'baseline-dbconvert-fixed'),*flags('ldflags'),*flags('libs')]
p=subprocess.run(cmd,capture_output=True,text=True);(e/'baseline-dbconvert-fixed-build.log').write_text(p.stdout+p.stderr);p.check_returncode()
env=json.loads((e/'environments.json').read_text())['datalib']
for cmd in [['make','dbconvert_corr_util'],['make','install-binPROGRAMS']]:
 p=subprocess.run(['/usr/bin/sandbox-exec','-f',str(e/'datalib.sb'),*cmd],cwd=e/'build/datalib-filedb',env=env,capture_output=True,text=True);(e/('conversion-'+cmd[-1]+'.log')).write_text(p.stdout+p.stderr);p.check_returncode()
