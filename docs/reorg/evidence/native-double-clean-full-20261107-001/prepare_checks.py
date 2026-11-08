from pathlib import Path
import shutil
root=Path.home()/'Downloads/macos-datalib-reorg'
e=root/'evidence/native-double-clean-full-20261107-001'
prior=root/'evidence/adat-direct-20261107-001'
shutil.copy2(prior/'changes.json',e/'changes.json')
shutil.copy2(prior/'export_correlators.cc',e/'export_correlators.cc')
(e/'probe.cc').write_text((prior/'direct-api-probe.cc').read_text().split('#if __has_include',1)[0])
s=(prior/'check_api.py').read_text().replace("e=root/'evidence/adat-direct-20261107-001'","e=root/'evidence/native-double-clean-full-20261107-001'")
s=s.replace("prefix=root/'install/adat-direct-001'","prefix=e/'install/adat'")
(e/'check_api.py').write_text(s)
s=(prior/'check_aliases.py').read_text().replace("e=root/'evidence/adat-direct-20261107-001'","e=root/'evidence/native-double-clean-full-20261107-001'")
s=s.replace("cfg=root/'install/adat-direct-001/bin/adat-config'","cfg=e/'install/adat/bin/adat-config'")
(e/'check_aliases.py').write_text(s)
