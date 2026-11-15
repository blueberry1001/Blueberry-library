import argparse
import hashlib
import json
import os
import platform
import statistics
import subprocess
import tempfile
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
GENERATOR = ROOT / 'docs/development/measurements/fast-set-lc-20261116/generate.cpp'
OFFICIAL = ROOT / 'verify/data-structure/fast-set.test.cpp'
DRIVER = HERE / 'fast-set-end-to-end.cpp'
PROTOTYPE = HERE / 'trusted-input.hpp.txt'
SOURCES = {'403023': '59fd15abf763b5a59bb2069e1916e21ccfebbe291d116fb5240690cbce6357d7',
           '278227': 'ef5b067f04c7686cce6a75ccb1426686d0a152b4afe6599f3ddeaeb39a90219a'}
SHAPES = ('empty', 'full', 'half', 'sparse', 'cluster', 'updates')
VARIANTS = ('safe-adapter', 'trusted-adapter', 'official', 'submission')


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--compiler', default='g++')
    p.add_argument('--submission', choices=SOURCES, default='403023')
    p.add_argument('--external', type=Path)
    p.add_argument('--n', type=int, default=1000000)
    p.add_argument('--q', type=int, default=500000)
    p.add_argument('--seed', type=int, default=1729)
    p.add_argument('--rounds', type=int, default=4)
    p.add_argument('--timeout', type=float, default=120)
    args = p.parse_args()
    if args.output.exists() or not (1 <= args.n <= 10000000 and 1 <= args.q <= 1000000 and args.rounds >= 1 and args.timeout > 0):
        p.error('choose a new output, valid n/q, positive rounds and timeout')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    headers = (ROOT / 'blueberry/data-structure/fast-set.hpp', ROOT / 'blueberry/utility/fast-io.hpp')
    paths = (Path(__file__).resolve(), GENERATOR, OFFICIAL, DRIVER, PROTOTYPE, *headers)
    flags = ['-std=gnu++23', '-O2', '-DNDEBUG', '-mavx2', '-mpopcnt']
    data = {'status': 'running', 'compiler': args.compiler, 'flags': flags,
            'timestamp_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
            'platform': platform.platform(), 'cpu': platform.processor(), 'submission': args.submission,
            'submission_url': f'https://judge.yosupo.jp/submission/{args.submission}',
            'n': args.n, 'q': args.q, 'seed': args.seed, 'rounds': args.rounds, 'warmups': 1,
            'contract': 'all inputs satisfy trusted parser and pinned submission assumptions; invalid inputs are not compared',
            'hashes': {str(path.relative_to(ROOT)): sha(path) for path in paths},
            'builds': [], 'cases': [], 'samples': [], 'failures': []}
    def save():
        args.output.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    save()
    try:
        cpuinfo = Path('/proc/cpuinfo').read_text()
        data['cpu'] = next((line.split(':', 1)[1].strip() for line in cpuinfo.splitlines() if line.startswith('model name')), data['cpu'])
        features = set(next((line.split(':', 1)[1].split() for line in cpuinfo.splitlines() if line.startswith('flags')), []))
        data['cpu_features'] = {feature: feature in features for feature in ('avx2', 'popcnt')}
        if not all(data['cpu_features'].values()): raise RuntimeError('AVX2 or POPCNT unavailable; measurements not run')
        possible = sorted(os.sched_getaffinity(0))
        if not possible: raise RuntimeError('no available CPU for affinity')
        os.sched_setaffinity(0, {possible[0]})
        data['cpu_affinity'] = sorted(os.sched_getaffinity(0))
        if data['cpu_affinity'] != [possible[0]]: raise RuntimeError('CPU affinity could not be fixed')
        version = subprocess.run([args.compiler, '--version'], capture_output=True, text=True, check=True, timeout=args.timeout)
        data['compiler_version'] = version.stdout.splitlines()[0]
        if 'clang' in data['compiler_version'].lower(): flags.append('-fconstexpr-steps=100000000')
        save()
        with tempfile.TemporaryDirectory(prefix='blueberry-trusted-fastset-') as name:
            tmp = Path(name)
            external = tmp / 'submission.cpp'
            if args.external:
                raw = args.external.read_bytes()
                data['hashes']['external_supplied_bytes'] = hashlib.sha256(raw).hexdigest()
                external.write_bytes(raw.replace(b'\r\n', b'\n'))
            else:
                req = urllib.request.Request(f'https://v3.api.judge.yosupo.jp/submissions/{args.submission}', headers={'User-Agent': 'Blueberry-library/performance-study'})
                with urllib.request.urlopen(req, timeout=30) as response:
                    external.write_bytes(json.load(response)['source'].encode('utf-8'))
            data['hashes']['external_normalized'] = sha(external)
            if data['hashes']['external_normalized'] != SOURCES[args.submission]:
                raise RuntimeError('submission source hash differs from pinned value')
            binaries = {'generator': GENERATOR, 'safe-adapter': DRIVER, 'trusted-adapter': DRIVER,
                        'official': OFFICIAL, 'submission': external}
            for variant, source in binaries.items():
                exe = tmp / variant
                cmd = [args.compiler, *flags, '-I' + str(ROOT)]
                if variant == 'trusted-adapter': cmd.append('-DTRUSTED')
                cmd += [str(source), '-o', str(exe)]
                try:
                    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=args.timeout)
                    row = {'variant': variant, 'command': cmd, 'exit': proc.returncode,
                           'stdout': proc.stdout, 'stderr': proc.stderr, 'binary_sha256': sha(exe) if not proc.returncode else None}
                except subprocess.TimeoutExpired as e:
                    row = {'variant': variant, 'command': cmd, 'exit': None, 'timeout': True,
                           'stdout': str(e.stdout), 'stderr': str(e.stderr), 'binary_sha256': None}
                data['builds'].append(row)
                save()
                if row['exit'] != 0: raise RuntimeError(f'{variant} compilation failed or timed out')
            specs = [(shape, args.n, args.q) for shape in SHAPES]
            specs += [('half', n, min(args.q, 1000)) for n in (1, 63, 64, 65, 4095, 4096, 4097)]
            for case_id, (shape, n, q) in enumerate(specs):
                inp, expected = tmp / 'case.in', tmp / 'expected.out'
                cmd = [str(tmp / 'generator'), str(n), str(q), shape, str(args.seed), str(inp), str(expected)]
                begin = time.perf_counter()
                try:
                    gen = subprocess.run(cmd, capture_output=True, text=True, timeout=args.timeout)
                    case = {'id': case_id, 'shape': shape, 'n': n, 'q': q, 'generator_command': cmd,
                            'generator_exit': gen.returncode, 'generator_stdout': gen.stdout, 'generator_stderr': gen.stderr,
                            'generation_oracle_ms': (time.perf_counter() - begin) * 1000}
                except subprocess.TimeoutExpired as e:
                    case = {'id': case_id, 'shape': shape, 'n': n, 'q': q, 'generator_command': cmd,
                            'generator_exit': None, 'timeout': True, 'generator_stdout': str(e.stdout), 'generator_stderr': str(e.stderr)}
                data['cases'].append(case)
                save()
                if case['generator_exit'] != 0: raise RuntimeError(f'{shape} input/oracle generation failed or timed out')
                case.update(input_sha256=sha(inp), input_bytes=inp.stat().st_size,
                            expected_sha256=sha(expected), expected_bytes=expected.stat().st_size)
                save()
                for run in range(-1, args.rounds):
                    offset = (run + 1) % len(VARIANTS)
                    for variant in VARIANTS[offset:] + VARIANTS[:offset]:
                        actual, metrics = tmp / 'actual.out', tmp / 'rss.txt'
                        metrics.unlink(missing_ok=True)
                        cmd = ['/usr/bin/time', '-f', '%M', '-o', str(metrics), str(tmp / variant)]
                        begin = time.perf_counter()
                        try:
                            with inp.open('rb') as source, actual.open('wb') as target:
                                proc = subprocess.run(cmd, stdin=source, stdout=target, stderr=subprocess.PIPE, timeout=args.timeout)
                            sample = {'case_id': case_id, 'shape': shape, 'n': n, 'q': q, 'run': run,
                                      'warmup': run == -1, 'variant': variant, 'command': cmd, 'exit': proc.returncode,
                                      'stderr': proc.stderr.decode('utf-8', errors='replace'), 'wall_ms': (time.perf_counter() - begin) * 1000,
                                      'output_sha256': sha(actual), 'matches': actual.read_bytes() == expected.read_bytes(), 'max_rss_kib': None}
                        except subprocess.TimeoutExpired as e:
                            sample = {'case_id': case_id, 'shape': shape, 'n': n, 'q': q, 'run': run,
                                      'warmup': run == -1, 'variant': variant, 'command': cmd, 'exit': None,
                                      'timeout': True, 'stderr': str(e.stderr), 'wall_ms': None, 'max_rss_kib': None}
                        try:
                            peak = int(metrics.read_text().strip().splitlines()[-1])
                            if peak <= 0: raise ValueError('RSS must be positive')
                            sample['max_rss_kib'] = peak
                        except (OSError, IndexError, ValueError) as e:
                            sample['rss_error'] = str(e)
                        data['samples'].append(sample)
                        save()
                        if sample['exit'] != 0 or not sample.get('matches') or sample['max_rss_kib'] is None:
                            raise RuntimeError(f'{shape}/{variant} failed, timed out, mismatched or lacked RSS')
            data['summary'] = []
            for case in data['cases']:
                for variant in VARIANTS:
                    rows = [r for r in data['samples'] if r['case_id'] == case['id'] and r['variant'] == variant and not r['warmup']]
                    data['summary'].append({'case_id': case['id'], 'shape': case['shape'], 'n': case['n'], 'q': case['q'],
                                            'variant': variant, 'wall_median_ms': statistics.median(r['wall_ms'] for r in rows),
                                            'wall_min_ms': min(r['wall_ms'] for r in rows), 'wall_max_ms': max(r['wall_ms'] for r in rows),
                                            'rss_median_kib': statistics.median(r['max_rss_kib'] for r in rows)})
        data['status'] = 'passed'
    except Exception as e:
        data['status'] = 'failed'
        data['failures'].append({'type': type(e).__name__, 'message': str(e)})
        save()
        raise
    save()
    print(json.dumps(data['summary'], indent=2))


if __name__ == '__main__':
    main()
