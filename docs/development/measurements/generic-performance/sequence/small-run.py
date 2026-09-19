#!/usr/bin/env python3
"""Choose the small-N compression path with matching full-width 64/128-bit inputs."""
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
HEADER = 'blueberry/data-structure/wavelet-matrix.hpp'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--compiler', default='g++')
    parser.add_argument('--runs', type=int, default=5)
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--baseline', default='e0d8e71')
    args = parser.parse_args()
    if args.runs < 1: parser.error('--runs must be positive')
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()): parser.error('Refusing to overwrite previous evidence')
    output.mkdir(parents=True, exist_ok=True)
    revision = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    sources = {
        'baseline': subprocess.check_output(['git', 'show', f'{revision}:{HEADER}'], cwd=ROOT),
        'comparison_sort': (HERE / 'wavelet-copy-first.hpp.txt').read_bytes(),
        'current': (ROOT / HEADER).read_bytes(),
    }
    flags = ['-std=gnu++20', '-O2', '-DNDEBUG']
    metadata = {
        'timestamp_utc': datetime.now(timezone.utc).isoformat(), 'platform': platform.platform(),
        'cpuinfo': Path('/proc/cpuinfo').read_text().split('\n\n')[0], 'cpu_affinity': args.cpu,
        'compiler': subprocess.check_output([args.compiler, '--version'], text=True), 'flags': flags,
        'baseline_revision': revision, 'header_sha256': {k: hashlib.sha256(v).hexdigest() for k, v in sources.items()},
        'benchmark_sha256': hashlib.sha256((HERE / 'small.cpp').read_bytes()).hexdigest(),
        'runner_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        'runs': args.runs, 'warmups': 1, 'seed': 20260921,
        'timing': 'Repeated construction of the same input; sum of per-build steady-clock intervals. Input generation, get/checksum, destruction excluded. Clock overhead is included equally.',
        'order': 'Fresh process per sample, candidates rotate by run. Minimum 64 builds per process; at least 262144 input elements across repeated builds. Both 128-bit halves are hashed.',
    }
    (output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    cases = [(n, width, distribution) for n in (32, 63, 64, 65, 128, 256, 512, 1024, 2048, 4096)
             for width in (64, 128) for distribution in ('full', 'duplicates')]
    samples = []
    try:
        with tempfile.TemporaryDirectory(prefix='blueberry-small-wavelet-') as temporary:
            temp = Path(temporary)
            binaries = {}
            with (output / 'compile.log').open('w') as log:
                for variant, source in sources.items():
                    header = temp / variant / HEADER
                    header.parent.mkdir(parents=True, exist_ok=True); header.write_bytes(source)
                    binary = temp / variant / 'benchmark'
                    command = [args.compiler, *flags, '-I' + str(temp / variant), str(HERE / 'small.cpp'), '-o', str(binary)]
                    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
                    log.write(' '.join(command) + '\n' + result.stdout + result.stderr); log.flush()
                    result.check_returncode(); binaries[variant] = binary
            with (output / 'samples.jsonl').open('w') as log:
                for n, width, distribution in cases:
                    repetitions = max(64, 262144 // n)
                    inputs = set(); checksums = set()
                    for run in range(-1, args.runs):
                        order = list(sources)
                        rotation = run % len(order)
                        order = order[rotation:] + order[:rotation]
                        for variant in order:
                            command = ['taskset', '-c', str(args.cpu), str(binaries[variant]), str(width), distribution, str(n), str(repetitions)]
                            result = subprocess.run(command, text=True, capture_output=True, timeout=120)
                            if result.returncode:
                                (output / 'failure.json').write_text(json.dumps({'command': command, 'returncode': result.returncode, 'stdout': result.stdout, 'stderr': result.stderr}, indent=2))
                            result.check_returncode()
                            row = json.loads(result.stdout)
                            row.update(n=n, width=width, distribution=distribution, variant=variant,
                                       repetitions=repetitions, run=run, warmup=run < 0)
                            log.write(json.dumps(row) + '\n'); log.flush()
                            inputs.add(row['input_hash']); checksums.add(row['checksum'])
                            if run >= 0: samples.append(row)
                    assert len(inputs) == len(checksums) == 1, (n, width, distribution, inputs, checksums)
                    print('PASS', n, width, distribution, flush=True)
        assert (ROOT / HEADER).read_bytes() == sources['current'], 'Current header changed during measurement'
        assert hashlib.sha256((HERE / 'small.cpp').read_bytes()).hexdigest() == metadata['benchmark_sha256']
        assert hashlib.sha256(Path(__file__).read_bytes()).hexdigest() == metadata['runner_sha256']
        summary = []
        for n, width, distribution in cases:
            for variant in sources:
                group = [row for row in samples if (row['n'], row['width'], row['distribution'], row['variant']) == (n, width, distribution, variant)]
                summary.append({'n': n, 'width': width, 'distribution': distribution, 'variant': variant,
                                'build_us_each_median': statistics.median(r['build_us_each'] for r in group),
                                'build_us_each_min': min(r['build_us_each'] for r in group),
                                'build_us_each_max': max(r['build_us_each'] for r in group), 'runs': len(group)})
        (output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    except Exception as error:
        (output / 'error.txt').write_text(repr(error) + '\n'); raise

if __name__ == '__main__': main()
