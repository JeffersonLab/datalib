#!/usr/bin/env python3
"""Check dbbin using an independent standard-complex codec and Python means.

Usage: test_dbbin.py DBBIN DBBIN_FIXTURE NEW_WORKDIR
Only synthetic software fixtures are used; this is not a physics regression.
"""
import hashlib
import json
import math
from pathlib import Path
import statistics
import subprocess
import sys

binning, fixture, work = (Path(p).resolve() for p in sys.argv[1:])
work.mkdir(parents=True, exist_ok=False)


def run(args, success=True):
    result = subprocess.run(list(map(str, args)), capture_output=True,
                            text=True, timeout=15, cwd=work)
    with (work / 'commands.log').open('a') as log:
        log.write(' '.join(map(str, args)) + '\n' + result.stdout + result.stderr)
    assert (result.returncode == 0) == success, (args, result.returncode,
                                               result.stdout, result.stderr)
    return result.stdout


def read(path):
    lines = run([fixture, 'read', path]).splitlines()
    return json.loads(next(line.removeprefix('RESULT:') for line in lines
                           if line.startswith('RESULT:')))


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


checks = []
for count, sizes in ((5, (1, 2, 3, 5, 10)), (6, (2, 3))):
    source = work / f'input-{count}.sdb'
    run([fixture, 'create', source, count])
    before = digest(source)
    original = read(source)
    assert original['configurations'] == count and len(original['keys']) == 2
    for size in sizes:
        output = work / f'output-{count}-{size}.sdb'
        run([binning, output, source, size])
        actual = read(output)
        expected = {}
        for key, rows in original['keys'].items():
            expected[key] = []
            for first in range(0, count, size):
                chunk = rows[first:first + size]
                expected[key].append([
                    [statistics.fmean(row[t][component] for row in chunk)
                     for component in range(2)]
                    for t in range(len(rows[0]))])
        assert actual['metadata'] == original['metadata']
        assert actual['key_bytes'] == original['key_bytes']
        assert actual['configurations'] == math.ceil(count / size)
        assert actual['keys'] == expected, (count, size, actual, expected)
        assert all(math.isfinite(x) for rows in actual['keys'].values()
                   for row in rows for z in row for x in z)
        checks.append({'configurations': count, 'bin_size': size,
                       'output_configurations': actual['configurations']})
    for i, argument in enumerate(('0', '-1', 'junk', '2junk',
                                  '99999999999999999999')):
        output = work / f'invalid-{count}-{i}.sdb'
        run([binning, output, source, argument], success=False)
        assert not output.exists()
    run([binning, source, source, 2], success=False)
    assert digest(source) == before, 'Input changed'

summary = {'passed': True, 'cases': checks, 'keys_per_case': 2,
           'metadata_preserved': True, 'encoded_keys_preserved': True, 'input_hashes_preserved': True,
           'invalid_sizes_rejected_before_output_creation': True,
           'in_place_write_rejected': True,
           'reader': 'datalib std::vector<std::complex<double>>, no ENSEM'}
(work / 'results.json').write_text(json.dumps(summary, indent=2) + '\n')
print(json.dumps(summary, indent=2))
