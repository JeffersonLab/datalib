#!/usr/bin/env python3
"""Integration test using dbmerge/dbutil as independent readers.
Usage: test_dbreplicate.py DBREPLICATE DBMERGE DBUTIL SINGLE_CONFIG_DB NEW_WORKDIR
The input must contain at least one nonzero correlator sample.
"""
import hashlib
import json
import math
from pathlib import Path
import statistics
import subprocess
import sys
import xml.etree.ElementTree as ET

replicate, merge, util, source, work = map(lambda p: Path(p).resolve(), sys.argv[1:])
work.mkdir(parents=True, exist_ok=False)

def run(args, cwd=work, success=True):
    p = subprocess.run(list(map(str, args)), cwd=cwd, capture_output=True, text=True)
    with (work/'commands.log').open('a') as log:
        log.write(' '.join(map(str,args))+'\n'+p.stdout+p.stderr+'\n')
    assert (p.returncode == 0) == success, (args, p.returncode, p.stdout, p.stderr)
    return p.stdout

def export(db, label):
    out = work/label
    out.mkdir()
    run([util,db,'keysxml',out/'keys.xml'],out)
    run([util,db,'get',out/'keys.xml'],out)
    keys = ET.parse(out/'keys.xml').getroot()
    data = {}
    for p in out.glob('*.dat'):
        lines = p.read_text().splitlines()
        header = list(map(int,lines[0].split()))
        n, nt = header[:2]
        rows = [list(map(float,line.split())) for line in lines[1:] if line.strip()]
        assert len(rows)==n*nt
        assert all(int(row[0])==i%nt and all(math.isfinite(x) for x in row) for i,row in enumerate(rows))
        data[p.name] = (n,nt,[(row[1],row[2]) for row in rows])
    assert len(data)==len(keys)>0
    return sorted(ET.canonicalize(ET.tostring(key, encoding="unicode"), strip_text=True) for key in keys),data

original_hash=hashlib.sha256(source.read_bytes()).hexdigest()
keys, original = export(source,'original')
assert all(x[0]==1 for x in original.values())
listing=work/'merge.list'
listing.write_text(''.join(f'{i} {source}\n' for i in range(3)))
merged=work/'dbmerge.sdb'
run([merge,merged,listing])
zero=work/'zero.sdb'
run([replicate,zero,source,3,0,12345])
mkeys,mdata=export(merged,'merged')
zkeys,zdata=export(zero,'zero')
assert keys==mkeys==zkeys
assert mdata==zdata, 'Zero noise differs from standard dbmerge output'
assert run([util,source,'meta'])==run([util,zero,'meta'])==run([util,merged,'meta'])

n=4096; factor=0.01
samples=work/'samples.sdb';again=work/'again.sdb';other=work/'other.sdb'
for out,seed in ((samples,12345),(again,12345),(other,54321)):
    run([replicate,out,source,n,factor,seed])
skeys,sdata=export(samples,'samples')
assert skeys==keys
assert export(again,'again')[1]==sdata, 'Seed is not reproducible'
assert export(other,'other')[1]!=sdata, 'Different seeds generated identical values'
residuals=[]
for name,(count,nt,values) in sdata.items():
    assert count==n
    baseline=original[name][2]
    for t,c in enumerate(baseline):
        sigma=factor*abs(complex(*c))
        for component in (0,1):
            delta=[values[s*nt+t][component]-c[component] for s in range(n)]
            if sigma==0:
                assert all(d==0 for d in delta)
                continue
            z=[d/sigma for d in delta]
            assert abs(statistics.mean(z))<0.08, (name,t,component,'mean')
            assert abs(statistics.stdev(z)-1)<0.08, (name,t,component,'sigma')
            residuals.extend(z)
assert residuals

for i,(sample_count,width) in enumerate(((0,'0.1'),(-1,'0.1'),('2junk','0.1'),(2,'-0.1'),(2,'nan'),(2,'inf'),(2,'0.1junk'))):
    bad=work/f'invalid-{i}.sdb'
    run([replicate,bad,source,sample_count,width],success=False)
    assert not bad.exists()
run([replicate,work/'not-single.sdb',merged,2,'0.1'],success=False)
assert not (work/'not-single.sdb').exists()
before=hashlib.sha256(samples.read_bytes()).hexdigest()
run([replicate,samples,source,2,'0.1'],success=False)
assert hashlib.sha256(samples.read_bytes()).hexdigest()==before
assert hashlib.sha256(source.read_bytes()).hexdigest()==original_hash
summary={'passed':True,'keys':len(original),'samples':n,'factor':factor,
         'standardized_noise_mean':statistics.mean(residuals),
         'standardized_noise_stddev':statistics.stdev(residuals),
         'zero_noise_matches_dbmerge':True,'metadata_preserved':True,
         'seed_reproducible':True,'invalid_input_and_overwrite_checks':True}
(work/'results.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
