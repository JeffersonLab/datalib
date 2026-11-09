from pathlib import Path
import time,json
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/tools-filedb-20261109-001'
while True:
 try:
  rows=json.loads((e/'final-build.json').read_text())
  if any(x['step']=='final-datalib-filedb-install' and x['returncode']==0 for x in rows):break
 except (FileNotFoundError,json.JSONDecodeError):pass
 time.sleep(5)
s=Path('/tmp/datalib-tools-work/package_checks.py').read_text().replace("src=e/'sources/datalib'","src=e/'sources/datalib-final'").replace("e/'build/datalib-external'","e/'build/datalib-external-final'").replace("'external-'+step", "'external-final-'+step").replace("e/'build/datalib-filedb'", "e/'build/datalib-filedb-final'").replace('/build/datalib-filedb/datalib-0.1','/build/datalib-filedb-final/datalib-0.1').replace("'datalib-dist.log'","'datalib-dist-final.log'").replace("'package-check.json'","'package-check-final.json'")
# The source archive path expression is a literal longer than the directory expression.
s=s.replace("e/'build/datalib-filedb/datalib-0.1.tar.gz'", "e/'build/datalib-filedb-final/datalib-0.1.tar.gz'")
exec(compile(s,'package_checks_final','exec'))
