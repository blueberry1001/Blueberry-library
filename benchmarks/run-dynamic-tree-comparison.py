#!/usr/bin/env python3
"""Compare fixed-topology tree workloads; run serially on an idle host."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=ROOT / 'docs/development/measurements/dynamic-tree-comparison')
OUT = parser.parse_args().output.resolve()
OUT.mkdir(parents=True, exist_ok=True)
if (OUT / 'samples.jsonl').exists():
    raise FileExistsError('Preserve previous measurements; select a new --output.')
EXE = ROOT / '.build/dynamic-tree-comparison'
SOURCES = ['benchmarks/dynamic-tree-comparison.cpp', 'benchmarks/run-dynamic-tree-comparison.py',
           *['blueberry/graph/' + name + '.hpp' for name in ['dynamic-top-tree', 'static-top-tree',
              'link-cut-tree', 'heavy-light-decomposition', 'euler-tour-tree']]]
FLAGS = ['-std=gnu++20', '-O2', '-DNDEBUG', '-I.', '-isystem', '.deps/ac-library']
def command(args):
    return subprocess.check_output(args, cwd=ROOT, text=True).strip()

environment = {
    'timestamp_utc': datetime.now(timezone.utc).isoformat(),
    'platform': platform.platform(), 'compiler': command(['g++', '--version']),
    'flags': FLAGS, 'revision': command(['git', 'rev-parse', 'HEAD']),
    'working_tree': command(['git', 'status', '--short']), 'cpu': command(['lscpu']),
    'acl_revision': command(['git', '-C', '.deps/ac-library', 'rev-parse', 'HEAD']),
    'sources_sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES},
    'runs': 5, 'seed': 20260919, 'q': 20000, 'sizes': [1024, 16384],
    'note': 'Fixed random-recursive tree rooted at 0. No link/cut query. Serial alternating method order; no warmup or CPU pinning.'
}
(OUT / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
with (OUT / 'compile.log').open('w') as log:
    subprocess.run(['g++', *FLAGS, SOURCES[0], '-o', str(EXE)], cwd=ROOT, check=True, stdout=log, stderr=log)
samples = []
with (OUT / 'samples.jsonl').open('w') as log:
    for target in ['dp', 'path', 'subtree']:
        for n in environment['sizes']:
            for run in range(5):
                for method in (['dynamic', 'fixed'] if run % 2 == 0 else ['fixed', 'dynamic']):
                    row = json.loads(command([str(EXE), target, method, str(n), str(environment['q']), str(environment['seed'])]))
                    row['run'] = run
                    log.write(json.dumps(row) + '\n'); log.flush()
                    samples.append(row)
                    print(target, n, method, run, row['operations_ms'], flush=True)
summary = []
for target in ['dp', 'path', 'subtree']:
    for n in environment['sizes']:
        group = [s for s in samples if s['target'] == target and s['n'] == n]
        assert len({s['checksum'] for s in group}) == 1, (target, n, 'checksum mismatch')
        assert len({s['input_hash'] for s in group}) == 1, (target, n, 'input mismatch')
        for method in ['dynamic', 'fixed']:
            rows = [s for s in group if s['method'] == method]
            record = {'target': target, 'n': n, 'q': environment['q'], 'method': method, 'checksum': rows[0]['checksum']}
            for metric in ['build_ms', 'operations_ms']:
                record[metric + '_median'] = statistics.median(s[metric] for s in rows)
                record[metric + '_min'] = min(s[metric] for s in rows)
            summary.append(record)
for source, digest in environment['sources_sha256'].items():
    assert hashlib.sha256((ROOT / source).read_bytes()).hexdigest() == digest, (source, 'changed during run')
(OUT / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
