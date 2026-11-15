#!/usr/bin/env python3
"""Compare public Dijkstra graph layouts and, separately, LC submission 415573.

Run on Linux: python3 docs/development/measurements/dijkstra/run_csr_end_to_end_linux.py
    --compiler g++ --output .verification/csr-linux-gcc.json
The original submission is fetched only into a temporary directory. Its stage
timings are unavailable; its process time and peak RSS include its own I/O.
"""

import argparse
import datetime
import gc
import hashlib
import heapq
import json
import os
import platform
import random
import statistics
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
DRIVER = HERE / 'csr-end-to-end.cpp'
HEADER = ROOT / 'blueberry/graph/dijkstra.hpp'
IO = ROOT / 'blueberry/utility/fast-io.hpp'
WINDOWS_RESULTS = HERE / 'csr-end-to-end-msvc-current.json'
LC_RESULTS = HERE / 'cross-final.json'
SEED = 20261115
WINDOWS_SHAPES = ('random', 'zero', 'line', 'dag', 'disconnected')
LC_SHAPES = ('sparse', 'zero', 'line', 'disconnected')
VARIANTS = ('stream-nested', 'buffered-nested', 'buffered-csr')
REFERENCE = 'submission-415573'
REFERENCE_URL = 'https://v3.api.judge.yosupo.jp/submissions/415573'
FLAGS = ('-std=gnu++23', '-O3', '-march=native', '-DNDEBUG')
N, EXTRA = 100000, 400000


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def prepare_windows(shape, path):
    rng = random.Random(SEED)
    edges = []
    limit = 1000 if shape == 'disconnected' else N
    for i in range(1, limit):
        edges.append((i - 1, i, 0 if shape == 'zero' else 1))
    if shape != 'line':
        for _ in range(EXTRA):
            if shape == 'disconnected' and rng.randrange(2):
                a, b = rng.randrange(1000, N), rng.randrange(1000, N)
            else:
                a, b = rng.randrange(limit), rng.randrange(limit)
            if shape == 'dag':
                if a > b:
                    a, b = b, a
                if a == b:
                    continue
            edges.append((a, b, 0 if shape == 'zero' else rng.randrange(1_000_000_000)))
    with path.open('w', encoding='ascii', newline='\n') as out:
        out.write(f'{N} {len(edges)} 0 {N - 1}\n')
        out.writelines(f'{a} {b} {c}\n' for a, b, c in edges)
    return N, len(edges)


def prepare_lc(shape, path):
    n, m = (500000, 499999) if shape == 'line' else (200000, 500000)
    rng = random.Random(SEED)
    with path.open('w', encoding='ascii', newline='\n') as out:
        out.write(f'{n} {m} 0 {n - 1}\n')
        if shape == 'line':
            for i in range(1, n):
                out.write(f'{i - 1} {i} 1\n')
        else:
            seen = set()
            if shape != 'disconnected':
                for i in range(1, n):
                    out.write(f'{i - 1} {i} {0 if shape == "zero" else 1}\n')
                    seen.add((i - 1) * n + i)
            while len(seen) < m:
                lo, hi = ((0, 1000) if len(seen) < m // 2 else (1000, n)) if shape == 'disconnected' else (0, n)
                u, v = rng.randrange(lo, hi), rng.randrange(lo, hi)
                key = u * n + v
                if u == v or key in seen:
                    continue
                seen.add(key)
                out.write(f'{u} {v} {0 if shape == "zero" else rng.randrange(1000000001)}\n')
    return n, m


def oracle(path):
    with path.open(encoding='ascii') as inp:
        n, m, start, target = map(int, inp.readline().split())
        graph = [[] for _ in range(n)]
        count = 0
        for line in inp:
            a, b, cost = map(int, line.split())
            graph[a].append((b, cost))
            count += 1
    if count != m:
        raise ValueError('input edge count mismatch')
    dist = [None] * n
    dist[start] = 0
    pending = [(0, start)]
    while pending:
        cost, v = heapq.heappop(pending)
        if dist[v] != cost:
            continue
        if v == target:
            break
        for u, weight in graph[v]:
            nxt = cost + weight
            if dist[u] is None or nxt < dist[u]:
                dist[u] = nxt
                heapq.heappush(pending, (nxt, u))
    return graph, start, target, dist[target]


def verify(path, graph, start, target, expected):
    with path.open(encoding='ascii') as answer:
        first = list(map(int, answer.readline().split()))
        if first == [-1]:
            if expected is not None or answer.read().strip():
                raise ValueError('invalid unreachable output')
            return {'distance': None, 'edges': 0}
        if len(first) != 2:
            raise ValueError('invalid output header')
        distance, count = first
        if count < 0 or count >= len(graph):
            raise ValueError('invalid path length')
        current, total, visited = start, 0, {start}
        for _ in range(count):
            a, b = map(int, answer.readline().split())
            if a != current or b in visited:
                raise ValueError('invalid path order or repeated vertex')
            matches = [cost for vertex, cost in graph[current] if vertex == b]
            if not matches:
                raise ValueError('missing path edge')
            total += min(matches)
            current = b
            visited.add(b)
        if current != target or total != distance or distance != expected or answer.read().strip():
            raise ValueError('invalid distance, path weight, or trailing output')
    return {'distance': distance, 'edges': count}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler', choices=('g++', 'clang++'), required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=5)
    args = parser.parse_args()
    if args.rounds < 1:
        parser.error('--rounds must be positive')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    result = {'status': 'running', 'started_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'compiler': args.compiler, 'flags': list(FLAGS), 'platform': platform.platform(),
              'machine': platform.machine(), 'python_version': platform.python_version(),
              'seed': SEED, 'rounds': args.rounds, 'warmups': 1,
              'note': 'Windows-input and LC-constraint suites are separate. Only the LC-constraint suite includes submission 415573. The submission uses different I/O and graph contracts; its stage timings are not measured. Wall time includes the /usr/bin/time wrapper and opening input/output files. On Linux the driver peak_working_set_bytes field is ru_maxrss in bytes; max_rss_kib is reported by /usr/bin/time.',
              'runner_sha256': sha(Path(__file__)), 'windows_manifest_sha256': sha(WINDOWS_RESULTS),
              'lc_manifest_sha256': sha(LC_RESULTS),
              'driver_sha256': sha(DRIVER), 'header_sha256': sha(HEADER), 'io_sha256': sha(IO),
              'builds': [], 'inputs': {}, 'samples': [], 'verification': {}, 'failures': [],
              'reference': {'id': 415573, 'url': REFERENCE_URL, 'status': 'not_measured'}}

    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')

    save()
    try:
        if sys.platform != 'linux':
            raise RuntimeError('this runner requires Linux and /usr/bin/time')
        try:
            lines = Path('/proc/cpuinfo').read_text(encoding='utf-8').splitlines()
            result['cpu_model'] = next((line.split(':', 1)[1].strip() for line in lines if line.startswith('model name')), None)
            allowed = os.sched_getaffinity(0)
            cpu = min(allowed)
            os.sched_setaffinity(0, {cpu})
            result['cpu_affinity'] = [cpu]
        except (OSError, ValueError, AttributeError) as exc:
            result['cpu_affinity'] = None
            result['failures'].append({'phase': 'affinity', 'message': str(exc)})
        version = subprocess.run([args.compiler, '--version'], capture_output=True, text=True, check=True)
        result['compiler_version'] = version.stdout.splitlines()[0]
        baseline = json.loads(WINDOWS_RESULTS.read_text(encoding='utf-8'))['inputs']
        lc_baseline = json.loads(LC_RESULTS.read_text(encoding='utf-8'))
        expected_sources = {row['source_sha256'] for row in lc_baseline['builds'] if row.get('variant') == 'fastest'}
        if len(expected_sources) != 1:
            raise ValueError('cannot determine unique reference source hash from cross-final.json')
        result['reference']['expected_source_sha256'] = next(iter(expected_sources))
        with tempfile.TemporaryDirectory(prefix='blueberry-csr-linux-') as temp:
            work = Path(temp)
            binaries = {}
            for variant in VARIANTS:
                binary = work / variant
                define = {'stream-nested': '-DSTREAM_NESTED', 'buffered-csr': '-DCSR_GRAPH'}.get(variant)
                command = [args.compiler, *FLAGS, '-I', str(ROOT)]
                if define:
                    command.append(define)
                command += [str(DRIVER), '-o', str(binary)]
                proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
                result['builds'].append({'variant': variant, 'command': command, 'exit': proc.returncode,
                                         'stdout': proc.stdout, 'stderr': proc.stderr,
                                         'binary_sha256': sha(binary) if proc.returncode == 0 else None})
                save()
                if proc.returncode:
                    raise RuntimeError(f'compile failed for {variant}')
                binaries[variant] = binary
            if (sha(DRIVER), sha(HEADER), sha(IO)) != (result['driver_sha256'], result['header_sha256'], result['io_sha256']):
                raise ValueError('source changed during compilation')
            try:
                request = urllib.request.Request(REFERENCE_URL, headers={'User-Agent': 'blueberry-library-csr-benchmark'})
                with urllib.request.urlopen(request, timeout=30) as response:
                    source = json.load(response)['source'].encode('utf-8')
                actual = hashlib.sha256(source).hexdigest()
                result['reference']['source_sha256'] = actual
                if actual != result['reference']['expected_source_sha256']:
                    raise ValueError('reference source hash differs from saved cross-final.json')
                ref_source = work / 'submission-415573.cpp'
                ref_source.write_bytes(source)
                ref_binary = work / REFERENCE
                command = [args.compiler, *FLAGS, str(ref_source), '-o', str(ref_binary)]
                proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
                result['builds'].append({'variant': REFERENCE, 'command': command, 'exit': proc.returncode,
                                         'stdout': proc.stdout, 'stderr': proc.stderr,
                                         'binary_sha256': sha(ref_binary) if proc.returncode == 0 else None})
                if proc.returncode:
                    raise RuntimeError('reference compilation failed')
                binaries[REFERENCE] = ref_binary
                result['reference']['status'] = 'built'
            except Exception as exc:
                result['reference']['status'] = 'not_measured'
                result['failures'].append({'phase': 'reference', 'type': type(exc).__name__, 'message': str(exc)})
            save()
            for suite, shapes in (('windows-inputs', WINDOWS_SHAPES), ('lc-constraints', LC_SHAPES)):
                for shape in shapes:
                    input_path = work / f'{suite}-{shape}.in'
                    n, m = (prepare_windows(shape, input_path) if suite == 'windows-inputs'
                            else prepare_lc(shape, input_path))
                    expected_hash = (baseline[shape]['sha256'] if suite == 'windows-inputs'
                                     else lc_baseline['inputs'][shape]['sha256'])
                    actual_hash = sha(input_path)
                    result['inputs'][f'{suite}/{shape}'] = {'vertices': n, 'edges': m, 'sha256': actual_hash,
                                                           'expected_sha256': expected_hash}
                    save()
                    if actual_hash != expected_hash:
                        raise ValueError(f'input hash mismatch: {suite}/{shape}')
                    variants = VARIANTS + ((REFERENCE,) if suite == 'lc-constraints' and REFERENCE in binaries else ())
                    expected_output = None
                    output_paths = {}
                    for round_num in range(-1, args.rounds):
                        order = variants[round_num % len(variants):] + variants[:round_num % len(variants)]
                        for variant in order:
                            answer = work / f'{suite}-{shape}-{variant}-{round_num}.out'
                            rss = work / 'max-rss.txt'
                            command = ['/usr/bin/time', '-f', '%M', '-o', str(rss), str(binaries[variant])]
                            begin = time.perf_counter()
                            with input_path.open('rb') as inp, answer.open('wb') as out:
                                proc = subprocess.run(command, stdin=inp, stdout=out, stderr=subprocess.PIPE)
                            wall = (time.perf_counter() - begin) * 1000
                            if proc.returncode:
                                raise RuntimeError(f'{suite}/{shape}/{variant} exit {proc.returncode}: {proc.stderr.decode(errors="replace")}')
                            metrics = {} if variant == REFERENCE else json.loads(proc.stderr.decode('ascii'))
                            peak = int(rss.read_text(encoding='ascii').strip())
                            if peak <= 0 or (variant != REFERENCE and not metrics.get('peak_working_set_bytes')):
                                raise ValueError(f'missing RSS: {suite}/{shape}/{variant}')
                            answer_hash = sha(answer)
                            output_paths.setdefault(answer_hash, answer)
                            if variant != REFERENCE:
                                if expected_output is None:
                                    expected_output = answer_hash
                                elif expected_output != answer_hash:
                                    raise ValueError(f'public variants produced different output: {suite}/{shape}/{variant}')
                            if round_num >= 0:
                                result['samples'].append({'suite': suite, 'shape': shape, 'variant': variant,
                                                          'round': round_num, 'wall_ms': wall, 'max_rss_kib': peak,
                                                          'output_sha256': answer_hash, **metrics})
                            save()
                    graph, start, target, distance = oracle(input_path)
                    verified = {}
                    for output_hash, path in output_paths.items():
                        verified[output_hash] = verify(path, graph, start, target, distance)
                    result['verification'][f'{suite}/{shape}'] = {'distance': distance, 'outputs': verified}
                    del graph
                    gc.collect()
                    save()
            if (sha(DRIVER), sha(HEADER), sha(IO)) != (result['driver_sha256'], result['header_sha256'], result['io_sha256']):
                raise ValueError('source changed during measurement')
            result['summary'] = {}
            for sample in result['samples']:
                key = '/'.join((sample['suite'], sample['shape'], sample['variant']))
                result['summary'].setdefault(key, []).append(sample)
            for key, rows in result['summary'].items():
                fields = ('wall_ms', 'max_rss_kib', 'parse_ms', 'build_ms', 'parse_build_ms', 'search_ms', 'output_ms')
                result['summary'][key] = {field: statistics.median(row[field] for row in rows)
                                          if all(row.get(field) is not None for row in rows) else None for field in fields}
            if REFERENCE in binaries:
                result['reference']['status'] = 'measured_and_verified'
            result['status'] = 'passed' if REFERENCE in binaries else 'passed_with_unmeasured_reference'
    except Exception as exc:
        result['status'] = 'failed'
        result['failures'].append({'phase': 'run', 'type': type(exc).__name__, 'message': str(exc)})
    result['finished_utc'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save()
    print(json.dumps({'status': result['status'], 'reference': result['reference'], 'failures': result['failures']}, indent=2))
    return 0 if result['status'] == 'passed' else 2


if __name__ == '__main__':
    raise SystemExit(main())
