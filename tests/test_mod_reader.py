#!/usr/bin/env python3
"""Independent QDP MOD wire-format fixtures; no lattice/QDP dependency.
Usage: python3 test_mod_reader.py /path/to/dbutil
"""
import pathlib
import struct
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zlib

exe = pathlib.Path(sys.argv[1]).resolve()
u32 = lambda n: struct.pack('>I', n)
pos = lambda n: n.to_bytes(16, 'big')
desc = lambda b: u32(len(b)) + b
magic = desc(b'XXXXQDPLazyDiskMapObjFileXXXX')

def fixture(path, kind='eigenVecsTimeSlice', keys=None, index=None):
    meta = f'<MODMetaData><id>{kind}</id></MODMetaData>'.encode()
    if keys is None:
        keys = [struct.pack('>ii', 7, 11), struct.pack('>ii', 2, 3)]
    header = magic + u32(1) + desc(meta)
    value_start = len(header) + 16
    if index is None:
        index = value_start + 19
    # Arbitrary undecodable payload. A sparse >4 GiB fixture verifies seeking.
    table = u32(len(keys)) + b''.join(desc(k) + pos(value_start) for k in keys)
    with path.open('wb') as f:
        f.write(header + pos(index))
        f.write(b'not a lattice value')
        f.seek(index)
        f.write(table + u32(zlib.crc32(table)))
    return meta.decode(), index

def run(path, *args, ok=True):
    p = subprocess.run([str(exe), str(path), *map(str, args)], capture_output=True, text=True)
    assert (p.returncode == 0) == ok, (args, p.stdout, p.stderr)
    return p.stdout

with tempfile.TemporaryDirectory() as tmp:
    root = pathlib.Path(tmp)
    db = root/'eigen.mod'
    meta, index = fixture(db)
    assert run(db, 'meta').strip() == meta
    assert run(db, 'keys').splitlines() == [
        'KeyTimeSliceColorVec_t: t_slice= 7 colorvec= 11',
        'KeyTimeSliceColorVec_t: t_slice= 2 colorvec= 3']
    out = root/'keys.xml'
    run(db, 'keysxml', out)
    assert [(int(e.findtext('t_slice')), int(e.findtext('colorvec')))
            for e in ET.parse(out).getroot()] == [(7,11),(2,3)]
    run(db, 'get', out, ok=False)
    run(db, 'keys', 'extra', ok=False)
    raw = db.read_bytes()
    db.write_bytes(raw[:-1])
    run(db, 'keys', ok=False)
    assert run(db, 'meta').strip() == meta  # metadata never needs the index
    db.write_bytes(raw[:-1] + bytes([raw[-1] ^ 1]))
    run(db, 'keys', ok=False)
    db.write_bytes(raw[:index] + u32(0xffffffff) + raw[index+4:])
    run(db, 'keys', ok=False)
    # Header index offsets and per-entry value offsets must stay in bounds.
    header_offset = len(magic) + 4 + 4 + len(meta.encode())
    db.write_bytes(raw[:header_offset] + pos(1) + raw[header_offset+16:])
    run(db, 'keys', ok=False)
    db.write_bytes(raw[:header_offset] + pos(2**80) + raw[header_offset+16:])
    run(db, 'keys', ok=False)
    bad_table = u32(1) + desc(struct.pack('>ii', 0, 0)) + pos(index + 1)
    db.write_bytes(raw[:index] + bad_table + u32(zlib.crc32(bad_table)))
    run(db, 'keys', ok=False)
    fixture(db, keys=[b'wrong key'])
    run(db, 'keys', ok=False)
    fixture(db, keys=[])
    assert run(db, 'keys') == ''
    fixture(db, kind='futureType')
    run(db, 'meta')
    run(db, 'keys', ok=False)
    fixture(db, keys=[struct.pack('>ii', 19, 31)], index=2**32 + 73)
    assert 't_slice= 19 colorvec= 31' in run(db, 'keys')
    mass = b'light & strange'
    key = struct.pack('>iiiii', 4, 8, 12, 1, 2) + desc(mass)
    fixture(db, kind='propDistillation', keys=[key])
    assert run(db, 'keys').strip() == ('KeyPropDistillation_t: t_slice= 8 t_source= 4 '
        'spin_snk= 2 spin_src= 1 colorvec_src= 12 mass= light & strange')
    run(db, 'keysxml', out)
    e = ET.parse(out).getroot()[0]
    assert e.findtext('mass') == mass.decode()
    assert e.findtext('colorvec_src') == '12'
    fixture(db)
    raw = db.read_bytes()
    db.write_bytes(raw[:len(magic)] + u32(2) + raw[len(magic)+4:])
    run(db, 'meta', ok=False)
print('MOD reader integration tests passed')
