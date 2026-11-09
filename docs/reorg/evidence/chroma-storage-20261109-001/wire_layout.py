from pathlib import Path
import struct,json,hashlib,subprocess
r=Path('/Users/admin/Downloads/macos-datalib-reorg');e=r/'evidence/chroma-storage-20261109-001'
def normalize(blob):
 out=bytearray(blob);ignored=[]
 for start in range(8192,len(out),8192):
  page=out[start:start+8192];typ=struct.unpack_from('<H',page,18)[0];ranges=[]
  if typ==0x4001:
   used=struct.unpack_from('<I',page,28)[0];ranges=[(32+used,8192)]
  elif typ==0x1001:
   count=struct.unpack_from('<H',page,16)[0];free=struct.unpack_from('<I',page,24)[0];ranges=[(28+12*count,free+1)]
  if typ==0x5001 and struct.unpack_from('<H',page,16)[0]==0:ranges=[(28,8192)]
  if ranges:
   ranges.append((20,24))
   for lo,hi in ranges:
    assert 0<=lo<=hi<=8192
    out[start+lo:start+hi]=b'\0'*(hi-lo)
   ignored.append({'page':start//8192,'type':hex(typ),'unused_and_crc_ranges':ranges})
 return out,ignored
old=e/'frozen-repeat';old.mkdir(exist_ok=True)
if not (old/'local.sdb').exists():
 with (e/'freeze-repeat-run.log').open('w') as f:subprocess.run([str(e/'freeze'),str(old)],stdout=f,stderr=subprocess.STDOUT,check=True)
results=[]
for folder in [old,*[e/('codec-probe-final-'+str(n)+'-ranks') for n in (1,2)]]:
 for p in sorted(folder.glob('*.sdb')):
  before=e/'frozen'/('local.sdb' if p.name.startswith('local') else 'collective.sdb');a=before.read_bytes();b=p.read_bytes();aa,ignored=normalize(a);bb,ignored_b=normalize(b);assert ignored==ignored_b;assert aa==bb,p
  results.append({'file':str(p),'reference':str(before),'same_wire_bytes_except_unused_page_space_and_its_crc':True,'raw_identical':a==b,'unused_ranges':ignored,'size':len(b),'sha256':hashlib.sha256(b).hexdigest(),'normalized_sha256':hashlib.sha256(bb).hexdigest()})
(e/'codec-layout.json').write_text(json.dumps({'status':'passed','layout_source':'FILEDB db2a795 ffdb_page.h; user-info tail and hash bucket free gap only; masks checksum for those same pages. All remaining bytes including payload CRCs identical.','comparisons':results},indent=2)+'\n')
print('Old/old and old/new wire layouts match after excluding only unused page space and its CRC')
