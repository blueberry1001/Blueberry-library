"""Serial, CPU-pinned comparisons; no compilation runs during measurement.

Run from repository root: python3 docs/development/measurements/frequent-2026-10/core/run.py --prepare
then run without --prepare. Optional --label retains intermediate/rejected results.
"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[5]
HERE = Path(__file__).resolve().parent
BUILD = ROOT / '.build/frequent-core'
BASE = 'd6af3e9'
FLAGS = ['-std=gnu++20', '-O2', '-DNDEBUG', '-I.', '-I.deps/ac-library', '-I'+str(BUILD)]
PROFILES = {'g++': ['g++', *FLAGS], 'clang++': ['clang++', *FLAGS],
            'g++-assert': ['g++', *[flag for flag in FLAGS if flag != '-DNDEBUG']]}
HEADERS = {'fps': 'blueberry/math/formal-power-series.hpp',
           'uf': 'blueberry/data-structure/potential-union-find.hpp'}
parser = argparse.ArgumentParser()
parser.add_argument('--prepare', action='store_true')
parser.add_argument('--label', default='final')
args = parser.parse_args()
os.chdir(ROOT)
BUILD.mkdir(parents=True, exist_ok=True)
if args.prepare:
    for name, path in HEADERS.items():
        source = subprocess.check_output(['git', 'show', BASE+':'+path], text=True)
        (BUILD / ('baseline-'+name+'.hpp')).write_text(source.replace('namespace blueberry', 'namespace baseline_'+name))
    for compiler, command in PROFILES.items():
        subprocess.run([*command, str(HERE/'benchmark.cpp'), '-o', str(BUILD/compiler)], check=True)
    raise SystemExit(0)

cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
environment = {'baseline': BASE, 'platform': platform.platform(), 'cpu_affinity': cpu,
               'cpuinfo': Path('/proc/cpuinfo').read_text().split('model name')[1].splitlines()[0],
               'profiles': PROFILES, 'seed': 20261017, 'warmups': 1, 'runs': 5,
               'scope': 'FPS: operation plus result checksum, source generation excluded, build not measured (NA). UF: generation excluded; build and operations separate. No parse/format or RSS measurement.',
               'headers': {p: hashlib.sha256(Path(p).read_bytes()).hexdigest() for p in HEADERS.values()},
               'measurement_sources': {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                       for p in [HERE/'benchmark.cpp', HERE/'run.py']},
               'compilers': {c: subprocess.check_output([c, '--version'], text=True).splitlines()[0] for c in ['g++','clang++']}}
(HERE/(args.label+'-environment.json')).write_text(json.dumps(environment, indent=2)+'\n')
cases = [('fps', 'integral', n, r) for n,r in [(0,1000),(1,10000),(64,1000),(262144,8)]]
cases += [('fps', op, n, r) for op in ['eval','log','multiply'] for n,r in [(64,1000),(65536,8)]]
cases += [(kind, op, 65536, 1000000) for kind in ['additive','matrix'] for op in ['flat','balanced','mixed']]
rows = []
with (HERE/(args.label+'-samples.csv')).open('w') as stream:
    writer = csv.writer(stream)
    writer.writerow(['compiler','kind','operation','n','q_or_repeats','variant','run','build_ms','operation_ms','checksum'])
    for compiler in PROFILES:
        for kind,op,n,q in cases:
            expected = None
            for run in range(-1,5):
                for variant in (['before','after'] if run%2 else ['after','before']):
                    output = subprocess.check_output([str(BUILD/compiler), variant,kind,op,str(n),str(q)], text=True).strip()
                    build, elapsed, checksum = output.split(',')
                    if expected is None: expected = checksum
                    if checksum != expected: raise RuntimeError((compiler,kind,op,n,q,variant,run,expected,checksum))
                    row = [compiler,kind,op,n,q,variant,run,None if build == 'NA' else float(build),float(elapsed),checksum]
                    writer.writerow(row)
                    stream.flush()
                    if run >= 0: rows.append(row)
            print(compiler,kind,op,n,q,flush=True)
with (HERE/(args.label+'-summary.csv')).open('w') as stream:
    writer = csv.writer(stream)
    writer.writerow(['compiler','kind','operation','n','q_or_repeats','variant','median_build_ms','min_build_ms','median_operation_ms','min_operation_ms'])
    for compiler in PROFILES:
        for kind,op,n,q in cases:
            for variant in ['before','after']:
                selected = [r for r in rows if r[:6] == [compiler,kind,op,n,q,variant]]
                builds = [r[7] for r in selected if r[7] is not None]
                times = [r[8] for r in selected]
                writer.writerow([compiler,kind,op,n,q,variant,
                                 statistics.median(builds) if builds else 'not_measured',
                                 min(builds) if builds else 'not_measured',
                                 statistics.median(times),min(times)])
