#!/usr/bin/env python3
"""Check moved tools against frozen original-ADAT results on synthetic inputs.
Usage: test_legacy_tools.py BIN_DIR FIXTURE LEGACY_FIXTURE_DIR NEW_WORKDIR
"""
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys
bins, fixture, fixtures, work = (Path(p).resolve() for p in sys.argv[1:])
work.mkdir(parents=True, exist_ok=False)

def run(args):
    process = subprocess.run(list(map(str,args)), cwd=work, capture_output=True,
                             text=True, timeout=120)
    with (work/'commands.log').open('a') as log:
        log.write(' '.join(map(str,args))+'\n'+process.stdout+process.stderr)
    assert process.returncode == 0, (args, process.stdout, process.stderr)
    return process.stdout

inputs = work/'inputs'
run([fixture,'create',inputs,fixtures])
for stem in ('legacy-sun','su2-populated','converted'):
    for ext in ('.bin','.xml'):
        assert (inputs/(stem+ext)).read_bytes() == (fixtures/(stem+ext)).read_bytes()
input_hashes = {p:hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs.iterdir()}
reference = json.loads((fixtures/'legacy-tools-reference.json').read_text())
results=[]
for case in reference['cases']:
    output=work/(case['case']+'.sdb')
    extra=[inputs/x if x.endswith('.sdb') else x for x in case['extra']]
    run([bins/case['tool'],output,inputs/case['input'],*extra])
    text=run([fixture,'dump',output])
    actual=json.loads(next(line[7:] for line in text.splitlines() if line.startswith('RESULT:')))
    expected=case['expected']
    assert actual['metadata']==expected['metadata'], case['case']
    assert actual['configurations']==expected['configurations'], case['case']
    assert actual['records'].keys()==expected['records'].keys(), case['case']
    scale=max(abs(complex(*z)) for rows in expected['records'].values() for row in rows for z in row)
    max_error=0.0
    for key, rows in expected['records'].items():
        assert len(rows)==len(actual['records'][key])
        for a,b in zip(rows,actual['records'][key]):
            assert len(a)==len(b)
            for za,zb in zip(a,b):
                za,zb=complex(*za),complex(*zb)
                assert math.isfinite(zb.real) and math.isfinite(zb.imag)
                error=abs(za-zb)
                assert error <= 1e-9*abs(za)+1e-12*max(scale,1e-30), (case['case'],za,zb)
                max_error=max(max_error,error)
    results.append({'case':case['case'],'passed':True,'max_absolute_error':max_error})
assert all(hashlib.sha256(p.read_bytes()).hexdigest()==sha for p,sha in input_hashes.items())
(work/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps({'passed':True,'cases':len(results),'max_absolute_error':max(x['max_absolute_error'] for x in results)},indent=2))
