from pathlib import Path
import json,re,subprocess,shlex
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001';a=r/'adat';rows=json.loads((e/'adat-utility-syntax.json').read_text())
s=(a/'main/dbutil/Makefile.am').read_text();m=re.search(r'^bin_PROGRAMS = ((?:.*\\\n)*.*)',s,re.M);active=set(m.group(1).replace('\\\n',' ').split())|{'check_corr_graph'}
assert all(x['returncode']==0 for x in rows if Path(x['source']).stem in active)
old=r/'evidence/arrays-owned-20261108-001/install/adat/bin/adat-config';flags=shlex.split(subprocess.check_output([str(old),'--cxxflags'],text=True));baseline=[]
for row in rows:
 if row['returncode']==0:continue
 name=Path(row['source']).name;src=e/'before/adat'/row['source'];assert src.exists()
 # The inactive source set is retained and its pre-existing failures are recorded, not silently dropped.
 with (e/('baseline-syntax-'+name+'.log')).open('w') as f:p=subprocess.run(['/usr/bin/clang++',*flags,'-fsyntax-only',str(src)],stdout=f,stderr=subprocess.STDOUT)
 assert p.returncode!=0,name;baseline.append({'source':name,'baseline_returncode':p.returncode})
(e/'adat-inactive-utility-limitations.json').write_text(json.dumps({'active_installed_programs':len(active)-1,'active_syntax_pass':True,'inactive_sources_also_fail_on_previous_installation':baseline},indent=2)+'\n')
subprocess.run(['git','-C',str(a),'add','main/dbutil/dbwick_rescale.cc'],check=True);subprocess.run(['git','-C',str(a),'commit','-m','Qualify ENSEM types in retained Wick rescaling utility'],check=True)
s=Path('/tmp/datalib-tools-work/resume_downstream4.py').read_text().replace('adat-004','adat-005').replace("name+'-004' if name=='adat'", "name+'-005' if name=='adat'").replace("name+'-004-", "name+'-005-").replace('downstream-retry4-build.json','downstream-final-build.json')
Path('/tmp/datalib-tools-work/build_downstream_final.py').write_text(s)
