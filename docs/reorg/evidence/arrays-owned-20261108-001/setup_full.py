from pathlib import Path
import json
import shutil
import subprocess
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/arrays-owned-20261108-001'
old=root/'evidence/native-double-clean-full-20261107-001'
last=root/'evidence/native-double-clean-full-20261107-002'
envs=json.loads((e/'environments.json').read_text())
(e/'environment.json').write_text(json.dumps(envs['downstream'],indent=2)+'\n')
validation=json.loads((e/'reorg-validation-heads.json').read_text())
for name,item in validation.items():
    item['local_devel']=subprocess.check_output(['git','--no-optional-locks','-C',str(root/name),'for-each-ref','--format=%(objectname)','refs/heads/devel'],text=True)
(e/'reorg-validation-heads.json').write_text(json.dumps(validation,indent=2)+'\n')
shutil.copy2(old/'changes.json',e/'changes.json')
shutil.copy2(old/'export_correlators.cc',e/'export_correlators.cc')
(e/'probe.cc').write_text((old/'direct-api-probe.cc').read_text().split('#if __has_include',1)[0])
for name in ('check_api.py','check_aliases.py','openmp_check.py'):
    s=(old/name).read_text().replace('evidence/native-double-clean-full-20261107-001','evidence/arrays-owned-20261108-001')
    s=s.replace("root/'evidence/standalone-clean-20261107-001/install/chroma/bin/chroma'", "e/'install/chroma/bin/chroma'")
    (e/name).write_text(s)
s=(last/'run_full.py').read_text()
s=s.replace("e=root/'evidence/native-double-clean-full-20261107-002'", "e=root/'evidence/arrays-owned-20261108-001'")
s=s.replace("stack=root/'evidence/native-double-clean-full-20261107-001'",'stack=e')
s=s.replace("clean=root/'evidence/standalone-clean-20261107-001'",'clean=e')
s=s.replace('after retaining a resource-limited first attempt. Clean-source standalone datalib and ADAT-free Chroma', 'with the established allocation-cache cap. Bundled-array datalib without external xpath_reader, with ADAT-free Chroma')
(e/'run_full.py').write_text(s)
s=(last/'audit.py').read_text().replace('evidence/native-double-clean-full-20261107-002','evidence/arrays-owned-20261108-001')
s=s.replace('    resource = load(evidence / "resource-retry.json")\n    assert resource["SB_CACHEGB_CPU"] == "1" and resource["max_rss_mb"] == 8192 and resource["tensor_gb"] == 2\n    assert load(Path(resource["targeted_retry"]))["status"] == "passed"\n    assert load(Path(resource["failed_full_attempt"]))["status"] == "attention_required"', '    resource = {"SB_CACHEGB_CPU": "1", "max_rss_mb": 8192, "tensor_gb": 2}\n    assert load(evidence / "environment.json")["SB_CACHEGB_CPU"] == "1"')
s=s.replace('load(evidence / "reorg-before.json")','load(evidence / "reorg-validation-heads.json")')
s=s.replace('    clean = reorg / "evidence/standalone-clean-20261107-001"\n    verification = load(clean / "verification.json")', '    clean = evidence\n    verification = load(evidence / "stack-verification.json")')
s=s.replace('before = reorg / "evidence/adat-direct-20261107-001"','before = reorg / "evidence/native-double-clean-full-20261107-002"')
s=s.replace('"resource_retry": resource','"resource_configuration": resource')
(e/'audit.py').write_text(s)
# Use the existing tabulation/historical audit presentation, replacing only this checkpoint's scope.
s=(last/'report.py').read_text().replace('relative = Path("evidence/native-double-clean-full-20261107-002")','relative = Path("evidence/arrays-owned-20261108-001")')
s=s.replace('# Full native-double regression with the clean datalib stack','# Bundled array headers and full native-double regression')
s=s.replace('7 November 2026. Fresh ADAT, ColorVec and Redstar builds used the same\nclean standalone datalib installation as the freshly built ADAT-free Chroma.', '8 November 2026. Datalib now owns its rank-1 through rank-4 array headers\nunder `<datalib/model/xml_array*.h>` and has no external xpath_reader\nconfigure, compiler, link or installed-header dependency. Fresh ADAT,\nColorVec, Redstar and Chroma builds used that installation.')
start=s.index('All five application/library components were built from clean source checkouts')
end=s.index('Both evaluators used the matching',start)
s=s[:start]+'''All five components were built from clean source checkouts on
`rge/devel-datalib-reorg` into empty build/install directories with Apple
Clang, libc++ and libomp. Datalib configured without `--with-xpath` and
passed four core and five FILEDB checks, with ADAT, QDP/QMP and external
xpath_reader filesystem access denied. Both builds reproduced the existing
frozen formats and the additional rank-1/2/3/4 fixture captured from the old
implementation before extraction. No existing fixture was regenerated.

The four copied headers retain their `XMLArray` namespace, guards, layout
and algorithms. Only internal include paths and trailing whitespace were
changed. The source notices and COPYING.xpath_reader remain in the package;
[array provenance](evidence/arrays-owned-20261108-001/array-provenance.json)
records the upstream commit and hashes. The dependency removal does not
remove libxml2, OpenMP, or the optional FILEDB dependency.

Chroma's QDP array header has pre-existing rank-1 empty-comparison/operator!=
differences. Its two datalib consumers explicitly select QDP's array headers
first, preserving their previous behavior. Installed-header checks and
both include orders with QDP and standalone xpath_reader passed the same
frozen array wire fixtures. No top-level xml_array compatibility headers
were installed by datalib.

Chroma rebuilt with ADAT and external standalone xpath_reader access denied.
ADAT, ColorVec and Redstar rebuilt against the same new datalib installation
and passed `make check`. QDP++/QMP, SuperbBLAS, Hadron, tensor, FILEDB and
PRIMME support installations were reused; those unchanged support libraries
were not rebuilt. ADAT and QDP retain their own XML support dependencies;
they are not datalib dependencies.

[Build commands and return codes](evidence/arrays-owned-20261108-001/build.json),
[owned-header/dependency verification](evidence/arrays-owned-20261108-001/owned-array-verification.json)
and [complete stack verification](evidence/arrays-owned-20261108-001/stack-verification.json)
record the checks. Actual OpenMP teams of one and four were observed from
the fresh Redstar flags. Both evaluators link libomp, with no libgomp or
libstdc++ in either process.

The fresh install prefixes are under `evidence/arrays-owned-20261108-001/install/`.
The installed ADAT probe also confirmed that the 37 removed compatibility
headers and representative legacy aliases remain absent. Every Chroma job
ran through an executor denying ADAT and external standalone xpath_reader
access. The audit separately rechecked the real executable, wrapper, sandbox
profile and datalib archive hashes.

'''+s[end:]
start=s.index('## Resource configuration and retained first attempt')
end=s.index('## Paired results',start)
s=s[:start]+'''## Resource configuration

The full paired matrix and NPT-only lane used the established
`SB_CACHEGB_CPU=1` reusable-cache cap, unchanged 8 GiB RSS guard and 2 GiB
tensor planner. Physics inputs, thread/rank counts, numerical gate and
user deferrals were unchanged. The preceding checkpoint's retained
memory-limit failure and cache-capped rerun are documented in the
[prior full clean-stack report](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md).

'''+s[end:]
s=s.replace('Against the immediately preceding full direct-API reorg run,','Against the immediately preceding cache-capped clean-stack run,')
s=s.replace('NATIVE-DOUBLE-CLEAN-REGRESSIONS.md").write_text(doc)','ARRAYS-OWNED-REGRESSIONS.md").write_text(doc)')
s=s.replace('print(reorg / "NATIVE-DOUBLE-CLEAN-REGRESSIONS.md")','print(reorg / "ARRAYS-OWNED-REGRESSIONS.md")')
s=s.replace('link = "[Full clean-stack regression](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md)"','link = "[Bundled-array regression](ARRAYS-OWNED-REGRESSIONS.md)"')
(e/'report.py').write_text(s)
print('Prepared fresh full-matrix runners and scope-specific audit/report scripts.')
