#!/usr/bin/env python3
"""Serial, five-run comparable-input measurements; run only on an idle host."""
import hashlib
import argparse
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=ROOT / 'docs/development/measurements/advanced-range-expansion')
OUT = parser.parse_args().output.resolve()
OUT.mkdir(parents=True, exist_ok=True)
if (OUT / 'samples.jsonl').exists():
    raise FileExistsError('Preserve the previous measurements before rerunning this script.')
EXE = ROOT / '.build/advanced-range-expansion'
SOURCES = ['benchmarks/advanced-range-expansion.cpp', 'benchmarks/run-advanced-range-expansion.py',
           'blueberry/data-structure/static-range-lis.hpp', 'blueberry/data-structure/wavelet-matrix.hpp',
           'blueberry/data-structure/range-sort-range-product.hpp',
           'blueberry/data-structure/partially-retroactive-priority-queue.hpp']
FLAGS = ['-std=gnu++20', '-O2', '-DNDEBUG', '-I.', '-isystem', '.deps/ac-library']
def command(args):
    return subprocess.check_output(args, cwd=ROOT, text=True).strip()

environment = {
    'timestamp_utc': datetime.now(timezone.utc).isoformat(),
    'platform': platform.platform(), 'compiler': command(['g++', '--version']),
    'flags': FLAGS, 'revision': command(['git', 'rev-parse', 'HEAD']),
    'working_tree': command(['git', 'status', '--short']),
    'cpu': command(['lscpu']), 'acl_revision': command(['git', '-C', '.deps/ac-library', 'rev-parse', 'HEAD']),
    'sources_sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES},
    'runs': 5, 'seed': 20260919,
    'note': 'No warmup. Serial alternating method order. Input generation excluded. Construction included separately. No CPU pinning.'
}
(OUT / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
with (OUT / 'compile.log').open('w') as log:
    subprocess.run(['g++', *FLAGS, SOURCES[0], '-o', str(EXE)], cwd=ROOT, check=True, stdout=log, stderr=log)
samples = []
with (OUT / 'samples.jsonl').open('w') as log:
    for target in ['lis', 'range-sort', 'retro']:
        for n in [1024, 4096]:
            q = 5000
            for run in range(5):
                for method in (['library', 'baseline'] if run % 2 == 0 else ['baseline', 'library']):
                    row = json.loads(command([str(EXE), target, method, str(n), str(q), '20260919']))
                    row['run'] = run
                    log.write(json.dumps(row) + '\n')
                    log.flush()
                    samples.append(row)
                    print(target, n, method, run, row['operations_ms'], flush=True)
summary = []
for target in ['lis', 'range-sort', 'retro']:
    for n in [1024, 4096]:
        group = [s for s in samples if s['target'] == target and s['n'] == n]
        assert len({s['checksum'] for s in group}) == 1, (target, n, 'checksum mismatch')
        assert len({s['input_hash'] for s in group}) == 1, (target, n, 'input mismatch')
        for method in ['library', 'baseline']:
            rows = [s for s in group if s['method'] == method]
            record = {'target': target, 'n': n, 'q': 5000, 'method': method, 'checksum': rows[0]['checksum']}
            for metric in ['build_ms', 'operations_ms']:
                record[metric + '_median'] = statistics.median(s[metric] for s in rows)
                record[metric + '_min'] = min(s[metric] for s in rows)
            summary.append(record)
(OUT / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
for source, digest in environment['sources_sha256'].items():
    assert hashlib.sha256((ROOT / source).read_bytes()).hexdigest() == digest, (source, 'changed during run')
