#!/usr/bin/env python3
"""Compare submission 415573 with and without its own PROF instrumentation.

On Linux, for example:
  python3 docs/development/measurements/dijkstra/profile_fastest_linux.py \
    --compiler g++ --output .verification/fastest-profile-gcc.json

Input generation and the independent path oracle are shared with the Linux CSR
runner; the submission itself is fetched only into a temporary directory.
"""

import argparse
import datetime
import hashlib
import json
import os
import platform
import re
import runpy
import statistics
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
SHARED_RUNNER = HERE / 'run_csr_end_to_end_linux.py'
LC_RESULTS = HERE / 'cross-final.json'
URL = 'https://v3.api.judge.yosupo.jp/submissions/415573'
FLAGS = ('-std=gnu++23', '-O3', '-march=native', '-DNDEBUG')
SHAPES = ('sparse', 'zero', 'line', 'disconnected')
VARIANTS = ('plain', 'prof')
MARK = re.compile(r'^(input|build|search|format|write)\s+([0-9]+(?:\.[0-9]+)?) ms$')


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def profile(stderr):
    marks = {}
    for line in stderr.splitlines():
        match = MARK.fullmatch(line.strip())
        if not match or match.group(1) in marks:
            raise ValueError(f'unexpected profiling output: {line!r}')
        marks[match.group(1)] = float(match.group(2))
    if list(marks) not in (['input', 'build', 'search'], ['input', 'build', 'search', 'format', 'write']):
        raise ValueError(f'missing or reordered profiling marks: {list(marks)}')
    values = list(marks.values())
    if any(a > b for a, b in zip(values, values[1:])):
        raise ValueError('nonmonotonic cumulative profile marks')
    stages = {'input_ms': marks['input'], 'build_ms': marks['build'] - marks['input'],
              'search_ms': marks['search'] - marks['build'], 'format_ms': None, 'write_ms': None}
    if 'format' in marks:
        stages['format_ms'] = marks['format'] - marks['search']
        stages['write_ms'] = marks['write'] - marks['format']
    return {'cumulative_ms': marks, 'stages_ms': stages}


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
              'seed': 20261115, 'rounds': args.rounds, 'warmups': 1,
              'runner_sha256': sha(Path(__file__)), 'shared_runner_sha256': sha(SHARED_RUNNER),
              'lc_manifest_sha256': sha(LC_RESULTS), 'reference_url': URL,
              'note': 'PROF marks are cumulative from the program profiling start. Stage values are differences of rounded 0.01 ms marks. The prof build performs clock reads and stderr writes and is a separate measurement condition; never attribute its extra work to the plain build. On an unreachable target the submission returns before format/write marks; both are null, not zero. Wall time includes /usr/bin/time and opening files; max_rss_kib is its process peak RSS. These are local generated inputs, not official checker verdicts.',
              'builds': [], 'inputs': {}, 'warmup_samples': [], 'samples': [],
              'verification': {}, 'failures': []}

    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')

    save()
    try:
        if sys.platform != 'linux':
            raise RuntimeError('this runner requires Linux and /usr/bin/time')
        try:
            lines = Path('/proc/cpuinfo').read_text(encoding='utf-8').splitlines()
            result['cpu_model'] = next((line.split(':', 1)[1].strip() for line in lines if line.startswith('model name')), None)
            cpu = min(os.sched_getaffinity(0))
            os.sched_setaffinity(0, {cpu})
            result['cpu_affinity'] = [cpu]
        except (OSError, ValueError, AttributeError) as exc:
            result['cpu_affinity'] = None
            result['failures'].append({'phase': 'affinity', 'message': str(exc)})
        version = subprocess.run([args.compiler, '--version'], capture_output=True, text=True, check=True)
        result['compiler_version'] = version.stdout.splitlines()[0]
        baseline = json.loads(LC_RESULTS.read_text(encoding='utf-8'))
        expected_hashes = {row['source_sha256'] for row in baseline['builds'] if row.get('variant') == 'fastest'}
        if len(expected_hashes) != 1:
            raise ValueError('cannot determine unique submission hash from cross-final.json')
        result['expected_source_sha256'] = next(iter(expected_hashes))
        shared = runpy.run_path(str(SHARED_RUNNER))
        prepare, oracle, verify = shared['prepare_lc'], shared['oracle'], shared['verify']
        with tempfile.TemporaryDirectory(prefix='blueberry-fastest-prof-') as temp:
            work = Path(temp)
            request = urllib.request.Request(URL, headers={'User-Agent': 'blueberry-library-fastest-profile'})
            with urllib.request.urlopen(request, timeout=30) as response:
                source = json.load(response)['source'].encode('utf-8')
            result['source_sha256'] = hashlib.sha256(source).hexdigest()
            save()
            if result['source_sha256'] != result['expected_source_sha256']:
                raise ValueError('submission source hash differs from cross-final.json')
            source_path = work / 'submission-415573.cpp'
            source_path.write_bytes(source)
            binaries = {}
            for variant in VARIANTS:
                binary = work / variant
                command = [args.compiler, *FLAGS]
                if variant == 'prof':
                    command.append('-DPROF')
                command += [str(source_path), '-o', str(binary)]
                proc = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
                result['builds'].append({'variant': variant, 'command': command, 'exit': proc.returncode,
                                         'stdout': proc.stdout, 'stderr': proc.stderr,
                                         'binary_sha256': sha(binary) if proc.returncode == 0 else None})
                save()
                if proc.returncode:
                    raise RuntimeError(f'compile failed for {variant}')
                binaries[variant] = binary
            for shape in SHAPES:
                input_path = work / f'{shape}.in'
                n, m = prepare(shape, input_path)
                expected = baseline['inputs'][shape]['sha256']
                actual = sha(input_path)
                result['inputs'][shape] = {'vertices': n, 'edges': m, 'sha256': actual, 'expected_sha256': expected}
                save()
                if actual != expected:
                    raise ValueError(f'input hash mismatch: {shape}')
                outputs = {}
                for round_num in range(-1, args.rounds):
                    order = VARIANTS[round_num % 2:] + VARIANTS[:round_num % 2]
                    for variant in order:
                        answer = work / f'{shape}-{variant}-{round_num}.out'
                        rss = work / 'max-rss.txt'
                        command = ['/usr/bin/time', '-f', '%M', '-o', str(rss), str(binaries[variant])]
                        begin = time.perf_counter()
                        with input_path.open('rb') as inp, answer.open('wb') as out:
                            proc = subprocess.run(command, stdin=inp, stdout=out, stderr=subprocess.PIPE)
                        wall = (time.perf_counter() - begin) * 1000
                        stderr = proc.stderr.decode('utf-8', errors='replace')
                        sample = {'shape': shape, 'variant': variant, 'round': round_num,
                                  'wall_ms': wall, 'exit': proc.returncode, 'stderr': stderr,
                                  'output_sha256': sha(answer), 'max_rss_kib': None, 'profile': None}
                        samples = result['warmup_samples'] if round_num < 0 else result['samples']
                        samples.append(sample)
                        save()
                        if proc.returncode:
                            raise RuntimeError(f'{shape}/{variant}/{round_num} exit {proc.returncode}')
                        sample['max_rss_kib'] = int(rss.read_text(encoding='ascii').strip())
                        if sample['max_rss_kib'] <= 0:
                            raise ValueError(f'missing RSS: {shape}/{variant}/{round_num}')
                        if variant == 'prof':
                            sample['profile'] = profile(stderr)
                        elif stderr:
                            raise ValueError(f'unexpected stderr from plain build: {shape}/{round_num}')
                        outputs.setdefault(sample['output_sha256'], answer)
                        save()
                graph, start, target, distance = oracle(input_path)
                verified = {}
                for output_hash, path in outputs.items():
                    verified[output_hash] = verify(path, graph, start, target, distance)
                for row in result['warmup_samples'] + result['samples']:
                    if row['shape'] == shape and row['variant'] == 'prof':
                        stages = row['profile']['stages_ms']
                        expected_output_marks = distance is not None
                        if (stages['format_ms'] is not None) != expected_output_marks:
                            raise ValueError(f'profiling marks do not match reachability: {shape}')
                result['verification'][shape] = {'distance': distance, 'outputs': verified}
                del graph
                save()
            if sha(SHARED_RUNNER) != result['shared_runner_sha256'] or sha(LC_RESULTS) != result['lc_manifest_sha256']:
                raise ValueError('input generator or manifest changed during measurement')
            result['summary'] = {}
            for shape in SHAPES:
                result['summary'][shape] = {}
                for variant in VARIANTS:
                    rows = [row for row in result['samples'] if row['shape'] == shape and row['variant'] == variant]
                    summary = {'wall_ms': statistics.median(row['wall_ms'] for row in rows),
                               'max_rss_kib': statistics.median(row['max_rss_kib'] for row in rows)}
                    if variant == 'prof':
                        for field in ('input_ms', 'build_ms', 'search_ms', 'format_ms', 'write_ms'):
                            vals = [row['profile']['stages_ms'][field] for row in rows]
                            summary[field] = statistics.median(vals) if all(v is not None for v in vals) else None
                    result['summary'][shape][variant] = summary
            result['status'] = 'passed'
    except Exception as exc:
        result['status'] = 'failed'
        result['failures'].append({'phase': 'run', 'type': type(exc).__name__, 'message': str(exc)})
    result['finished_utc'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save()
    print(json.dumps({'status': result['status'], 'failures': result['failures']}, indent=2))
    return 0 if result['status'] == 'passed' else 2


if __name__ == '__main__':
    raise SystemExit(main())
