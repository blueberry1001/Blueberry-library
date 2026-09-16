#!/usr/bin/env python3
"""Reproduce independent DS candidate comparisons (three paired repetitions)."""
import argparse
import csv
import json
import platform
import statistics
import subprocess
from datetime import datetime, timezone
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('--size',type=int,default=200000)
p.add_argument('--output',type=Path,default=Path('benchmark/results/lc-expansion-ds'))
args=p.parse_args()
args.output.mkdir(parents=True,exist_ok=True)
binary=Path('.build/lc-expansion-ds/benchmark')
flags=['-std=c++20','-O2','-DNDEBUG','-Wall','-Wextra','-Wshadow','-Werror','-I.']
subprocess.run(['g++',*flags,'benchmark/data-structure-expansion.cpp','-o',str(binary)],check=True)
environment={'timestamp':datetime.now(timezone.utc).isoformat(),'platform':platform.platform(),
             'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
             'flags':flags,'size':args.size,'runs':3,'seed':20260917,
             'cpu':subprocess.check_output(['lscpu'],text=True),
             'note':'Shared WSL host: other agents may run verification. Paired runs alternate order; small differences are inconclusive.'}
(args.output/'environment.json').write_text(json.dumps(environment,indent=2)+'\n')
rows=[]
for workload in ['uf-add','uf-matrix','set-random','set-monotone','persistent-linear','persistent-branch']:
    for repeat in range(3):
        pair=[]
        for candidate in (['pool','alternative'] if repeat%2==0 else ['alternative','pool']):
            memory_file=args.output/'rss.tmp'
            command=['/usr/bin/time','-f','%M','-o',str(memory_file),str(binary),workload,candidate,str(args.size)]
            result=subprocess.run(command,text=True,capture_output=True)
            record={'workload':workload,'candidate':candidate,'repeat':repeat,'exitcode':result.returncode,
                    'stdout':result.stdout,'stderr':result.stderr,'rss_kib_raw':memory_file.read_text()}
            with (args.output/'raw.jsonl').open('a') as f: f.write(json.dumps(record)+'\n')
            result.check_returncode()
            fields=result.stdout.strip().split(',')
            row={'workload':workload,'candidate':candidate,'repeat':repeat,'size':args.size,
                 'seconds':float(fields[3]),'checksum':fields[4],'rss_kib':int(memory_file.read_text())}
            rows.append(row);pair.append(row)
            print(row,flush=True)
        assert pair[0]['checksum']==pair[1]['checksum'],pair
with (args.output/'runs.csv').open('w',newline='') as f:
    writer=csv.DictWriter(f,fieldnames=rows[0].keys());writer.writeheader();writer.writerows(rows)
summary=[]
for workload in sorted({r['workload'] for r in rows}):
    for candidate in ['pool','alternative']:
        selected=[r for r in rows if r['workload']==workload and r['candidate']==candidate]
        summary.append({'workload':workload,'candidate':candidate,'median_seconds':statistics.median(r['seconds'] for r in selected),
                        'min_seconds':min(r['seconds'] for r in selected),'max_rss_kib':max(r['rss_kib'] for r in selected)})
(args.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
(args.output/'rss.tmp').unlink()
