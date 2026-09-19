#!/usr/bin/env python3
"""Run the finite DynamicSqrtTree comparison and preserve every sample."""
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[4]
OUT = Path(__file__).resolve().parent
FILES = [ROOT / 'benchmarks/dynamic-sqrt-tree.cpp',
         ROOT / 'blueberry/data-structure/dynamic-sqrt-tree.hpp',
         ROOT / 'blueberry/data-structure/sqrt-tree.hpp',
         *sorted((ROOT / '.deps/ac-library/atcoder').glob('*'))]

def hashes():
    return {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in FILES if path.is_file()}

initial_hashes = hashes()
metadata = {
    'timestamp_utc': datetime.now(timezone.utc).isoformat(),
    'platform': platform.platform(),
    'cpuinfo': Path('/proc/cpuinfo').read_text().split('processor\t: 1')[0],
    'compiler': subprocess.check_output(['g++', '--version'], text=True),
    'flags': ['-std=gnu++20', '-O2', '-DNDEBUG', '-I.', '-I.deps/ac-library'],
    'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
    'acl_revision': subprocess.check_output(['git', '-C', '.deps/ac-library', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
    'source_sha256': initial_hashes,
    'n': [4096, 4097, 65536, 65537], 'q': 50000,
    'update_percent': [0, 1, 50], 'seed': 20260919, 'runs': 5,
    'memory': 'Linux /proc/self/status VmHWM, KiB; current executable including identical generated inputs',
    'order': 'Rotate implementation order each repetition; one fresh process per sample; no warmup.'
}
(OUT / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
rows = []
with tempfile.TemporaryDirectory(prefix='blueberry-dynamic-sqrt-benchmark-') as temporary:
    binary = Path(temporary) / 'benchmark'
    command = ['g++', *metadata['flags'], 'benchmarks/dynamic-sqrt-tree.cpp', '-o', str(binary)]
    compiled = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
    (OUT / 'compile.log').write_text(' '.join(command) + '\n' + compiled.stdout + compiled.stderr)
    compiled.check_returncode()
    with (OUT / 'samples.jsonl').open('w') as stream:
        for n in metadata['n']:
            for rate in metadata['update_percent']:
                variants = ['static', 'dynamic', 'acl'] if rate == 0 else ['dynamic', 'acl']
                for run in range(metadata['runs']):
                    order = variants[run % len(variants):] + variants[:run % len(variants)]
                    for variant in order:
                        command = [str(binary), variant, str(n), str(metadata['q']), str(rate), str(metadata['seed'])]
                        sample = subprocess.run(command, cwd=ROOT, text=True, capture_output=True, timeout=60)
                        if sample.returncode:
                            (OUT / 'failure.json').write_text(json.dumps({'command': command, 'returncode': sample.returncode,
                                'stdout': sample.stdout, 'stderr': sample.stderr}, indent=2))
                            sample.check_returncode()
                        row = json.loads(sample.stdout)
                        row['run'] = run
                        stream.write(json.dumps(row) + '\n'); stream.flush()
                        rows.append(row)
                group = [r for r in rows if r['n'] == n and r['update_percent'] == rate]
                assert len({r['input_hash'] for r in group}) == 1, ('input mismatch', n, rate)
                assert len({r['checksum'] for r in group}) == 1, ('result mismatch', n, rate)
                print('PASS n=', n, ' update_percent=', rate, flush=True)
assert hashes() == initial_hashes, 'Source changed during measurement; keep logs, do not treat as comparable.'
summary = []
for n in metadata['n']:
    for rate in metadata['update_percent']:
        for variant in ['static', 'dynamic', 'acl']:
            group = [r for r in rows if r['n'] == n and r['update_percent'] == rate and r['implementation'] == variant]
            if not group:
                continue
            item = {'n': n, 'update_percent': rate, 'implementation': variant,
                    'input_hash': group[0]['input_hash'], 'checksum': group[0]['checksum'], 'runs': len(group)}
            for field in ['build_ms', 'operations_ms', 'peak_process_rss_kib']:
                item[field + '_median'] = statistics.median(r[field] for r in group)
                item[field + '_min'] = min(r[field] for r in group)
            summary.append(item)
(OUT / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
print('Complete:', len(rows), 'samples; all equal-input checksums matched.')
