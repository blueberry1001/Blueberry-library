#!/usr/bin/env python3
"""Equal-input portable comparison of new algebra implementations and textbook references."""
import argparse
from datetime import datetime, timezone
import hashlib,json,os,platform,statistics,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];os.chdir(root)
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--repeats',type=int,default=5)
parser.add_argument('--output',type=Path,default=Path('benchmark/results/algebra-expansion.json'))
args=parser.parse_args();build=Path('.build/algebra/benchmark');build.mkdir(parents=True,exist_ok=True)
compiler=os.environ.get('CXX','g++');flags=['-std=gnu++20','-O2','-DNDEBUG','-I.','-isystem','.deps/ac-library']
binary=build/'algebra'
subprocess.run([compiler,*flags,'benchmark/algebra-expansion.cpp','-o',str(binary)],check=True)
rows=[]
for family in ['and','xor','subset','matrix']:
 for run in range(args.repeats+1):
  for mode in ['modern','reference'] if run%2==0 else ['reference','modern']:
   rss=build/'rss.txt'
   completed=subprocess.run(['/usr/bin/time','-f','%M','-o',str(rss),str(binary),family,mode],capture_output=True,text=True)
   if completed.returncode:
    rows.append(dict(family=family,mode=mode,run=run,failed=True,returncode=completed.returncode,stderr=completed.stderr))
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(dict(runs=rows),indent=2)+'\n')
    raise RuntimeError(completed.stderr)
   row=json.loads(completed.stdout);row.update(run=run,warmup=run==0,peak_rss_kib=int(rss.read_text().strip()));rows.append(row)
   print(family,mode,run,row['ms'],flush=True)
 assert len({r['checksum'] for r in rows if r['family']==family})==1
summary=[]
for family in ['and','xor','subset','matrix']:
 for mode in ['modern','reference']:
  group=[r for r in rows if r['family']==family and r['mode']==mode and not r['warmup']]
  summary.append(dict(family=family,mode=mode,median_ms=statistics.median(r['ms'] for r in group),min_ms=min(r['ms'] for r in group),median_peak_rss_kib=statistics.median(r['peak_rss_kib'] for r in group)))
paths=[Path('benchmark/algebra-expansion.cpp'),*map(Path,['blueberry/math/bitwise-convolution.hpp','blueberry/math/subset-convolution.hpp','blueberry/math/matrix.hpp'])]
result=dict(generated_at=datetime.now(timezone.utc).isoformat(),compiler=subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],flags=flags,platform=platform.platform(),cpu=next((s.split(':',1)[1].strip() for s in Path('/proc/cpuinfo').read_text().splitlines() if s.startswith('model name')),'unknown'),seed=1729,repeats=args.repeats,warmups=1,timing_scope='Algorithm, required input copies/output checksum and destruction; input generation and process startup excluded. Matrix includes shape construction and elimination copy. RSS includes process/input.',source_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},summary=summary,runs=rows)
args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2)+'\n');print(args.output)
