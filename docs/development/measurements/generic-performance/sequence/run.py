#!/usr/bin/env python3
"""Paired, isolated sequence-structure benchmarks; never overwrite evidence."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
HEADERS = [f'blueberry/data-structure/{name}.hpp' for name in
           ('fast-set', 'wavelet-matrix', 'static-range-lis')]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline', default='e0d8e71')
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--assertions', action='store_true')
    parser.add_argument('--runs', type=int, default=5)
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--kind', choices=['all', 'set', 'wavelet', 'lis'], default='all')
    args = parser.parse_args()
    if args.runs < 1:
        parser.error('--runs must be positive')
    out = args.output.resolve()
    if out.exists() and any(out.iterdir()):
        parser.error('Output must be absent or empty; earlier results are preserved.')
    out.mkdir(parents=True, exist_ok=True)
    files = [*HEADERS, str((HERE / 'benchmark.cpp').relative_to(ROOT)), str(Path(__file__).resolve().relative_to(ROOT))]
    def hashes():
        return {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in files}
    initial = hashes()
    revision = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    flags = ['-std=gnu++20', '-O2'] + ([] if args.assertions else ['-DNDEBUG'])
    environment = {'timestamp_utc': datetime.now(timezone.utc).isoformat(),
                   'platform': platform.platform(), 'cpuinfo': Path('/proc/cpuinfo').read_text().split('\n\n')[0],
                   'compiler': subprocess.check_output([args.compiler, '--version'], text=True),
                   'flags': flags, 'baseline_revision': revision, 'current_source_sha256': initial,
                   'runs': args.runs, 'warmups': 1, 'seed': 20260920, 'cpu_affinity': args.cpu,
                   'order': 'Fresh process per sample; paired order alternates every repetition.',
                   'timing': 'Input generation excluded; construction and operations separated, checksum included; no I/O in timed sections.',
                   'memory': 'Peak process RSS, includes identical input/query buffers and standard library.'}
    (out / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    cases = [('set', d, 1 << 20, 1000000) for d in ('sparse', 'mixed', 'dense', 'duplicate', 'fill')]
    cases += [('wavelet', d, 500000, 200000) for d in ('random', 'duplicates', 'equal', 'sorted', 'reverse')]
    cases += [('lis', d, 65536, 100000) for d in ('random', 'duplicates', 'equal', 'sorted', 'reverse')]
    if args.kind != 'all': cases = [c for c in cases if c[0] == args.kind]
    samples = []
    try:
        with tempfile.TemporaryDirectory(prefix='blueberry-sequence-') as temporary:
            temp = Path(temporary)
            for header in HEADERS:
                target = temp / 'baseline' / header
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(subprocess.check_output(['git', 'show', f'{revision}:{header}'], cwd=ROOT))
            environment['baseline_source_sha256'] = {p: hashlib.sha256((temp / 'baseline' / p).read_bytes()).hexdigest() for p in HEADERS}
            (out / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
            binaries = {}
            with (out / 'compile.log').open('w') as log:
                for variant in ('baseline', 'current'):
                    binary = temp / variant / 'benchmark'
                    binary.parent.mkdir(parents=True, exist_ok=True)
                    include = temp / 'baseline' if variant == 'baseline' else ROOT
                    command = [args.compiler, *flags, '-I' + str(include), str(HERE / 'benchmark.cpp'), '-o', str(binary)]
                    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
                    log.write(' '.join(command) + '\n' + result.stdout + result.stderr); log.flush()
                    result.check_returncode(); binaries[variant] = binary
            with (out / 'samples.jsonl').open('w') as log:
                for kind, distribution, n, q in cases:
                    checksums = set(); inputs = set()
                    for run in range(-1, args.runs):
                        order = ('baseline', 'current') if run % 2 else ('current', 'baseline')
                        for variant in order:
                            command = ['taskset', '-c', str(args.cpu), str(binaries[variant]), kind, distribution, str(n), str(q)]
                            result = subprocess.run(command, text=True, capture_output=True, timeout=180)
                            if result.returncode:
                                (out / 'failure.json').write_text(json.dumps({'command': command, 'returncode': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}, indent=2))
                            result.check_returncode()
                            row = json.loads(result.stdout)
                            row.update(kind=kind, distribution=distribution, n=n, queries=q, variant=variant, run=run, warmup=run < 0)
                            row['total_ms'] = row['build_ms'] + row['operations_ms']
                            checksums.add(row['checksum']); inputs.add(row['input_hash'])
                            log.write(json.dumps(row) + '\n'); log.flush()
                            if run >= 0: samples.append(row)
                    assert len(checksums) == len(inputs) == 1, (kind, distribution, checksums, inputs)
                    print('PASS', kind, distribution, flush=True)
        assert hashes() == initial, 'Source changed during measurement; evidence retained.'
        summary = []
        for kind, distribution, n, q in cases:
            for variant in ('baseline', 'current'):
                group = [r for r in samples if (r['kind'], r['distribution'], r['variant']) == (kind, distribution, variant)]
                row = {'kind': kind, 'distribution': distribution, 'n': n, 'queries': q, 'variant': variant, 'runs': len(group)}
                for field in ('build_ms', 'operations_ms', 'total_ms', 'peak_process_rss_kib'):
                    row[field] = {'median': statistics.median(r[field] for r in group), 'min': min(r[field] for r in group), 'max': max(r[field] for r in group)}
                summary.append(row)
        (out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    except Exception as error:
        (out / 'error.txt').write_text(repr(error) + '\n')
        raise

if __name__ == '__main__':
    main()
