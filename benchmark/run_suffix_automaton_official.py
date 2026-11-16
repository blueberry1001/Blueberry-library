#!/usr/bin/env python3
"""Compare two Suffix Automaton headers on generated Library Checker cases."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import shlex
import statistics
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
HEADER = Path('blueberry/string/suffix-automaton.hpp')
DRIVERS = {
    'number_of_substrings': Path('verify/string/suffix-automaton-substrings.test.cpp'),
    'longest_common_substring': Path('verify/string/suffix-automaton-lcs.test.cpp'),
}

def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def stats(values):
    return {'median': statistics.median(values), 'min': min(values), 'max': max(values), 'samples': values}

def cpu_model():
    try:
        return next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                    if line.startswith('model name'))
    except (OSError, StopIteration):
        return 'unknown'

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--problems-root', type=Path, required=True,
                        help='generated yosupo06/library-checker-problems checkout')
    parser.add_argument('--baseline', type=Path, required=True, help='baseline include root')
    parser.add_argument('--candidate', type=Path, required=True, help='candidate include root')
    parser.add_argument('--source-root', type=Path, default=ROOT,
                        help='repository holding the existing verify drivers')
    parser.add_argument('--problems', default=','.join(DRIVERS), help='comma-separated problem names')
    parser.add_argument('--cases', default='all_same_00,max_random_00', help='case basenames without .in')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--flags', default='-std=gnu++20 -O2 -DNDEBUG')
    parser.add_argument('--warmups', type=int, default=1)
    parser.add_argument('--repeats', type=int, default=5)
    parser.add_argument('--cpu', type=int)
    parser.add_argument('--timeout', type=float, default=120, help='seconds per solution process')
    parser.add_argument('--checker-timeout', type=float, default=60)
    parser.add_argument('--output', type=Path, default=ROOT / '.benchmark/suffix-automaton-official.json')
    args = parser.parse_args()
    names = args.problems.split(',')
    cases = args.cases.split(',')
    if (not names or len(names) != len(set(names)) or any(p not in DRIVERS for p in names) or
        not cases or len(cases) != len(set(cases)) or
        any(not c or not all(ch.isalnum() or ch in '_-' for ch in c) for c in cases) or
        args.warmups < 0 or args.repeats <= 0 or args.timeout <= 0 or args.checker_timeout <= 0):
        parser.error('invalid problems, cases, counts, or timeouts')
    roots = {'baseline': args.baseline.resolve(), 'candidate': args.candidate.resolve()}
    problems_root, source_root = args.problems_root.resolve(), args.source_root.resolve()
    allowed = sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None
    if args.cpu is not None and (allowed is None or args.cpu not in allowed):
        parser.error(f'CPU {args.cpu} not in allowed affinity: {allowed}')
    compiler = subprocess.run([args.cxx, '--version'], capture_output=True, text=True, check=True).stdout.splitlines()[0]
    flags = shlex.split(args.flags)
    build = ROOT / '.build/suffix-automaton-official'
    build.mkdir(parents=True, exist_ok=True)
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    report = {
        'timestamp_utc': datetime.now(timezone.utc).isoformat(),
        'environment': {'platform': platform.platform(), 'cpu': cpu_model(), 'available_affinity': allowed,
                        'pinned_cpu': args.cpu, 'python': sys.version, 'compiler': compiler,
                        'compiler_command': args.cxx, 'flags': flags},
        'inputs': {'problem_root': str(problems_root), 'source_root': str(source_root),
                   'roots': {k: str(v) for k,v in roots.items()}, 'problems': names, 'cases': cases,
                   'warmups': args.warmups, 'repeats': args.repeats, 'solution_timeout': args.timeout,
                   'checker_timeout': args.checker_timeout},
        'timing_scope': 'process wall time for each solution: launch, file input, computation, file output, exit; excludes opening input/output files, checker, hashing, compilation, and input generation',
        'compile': {}, 'runs': [], 'summary': {}, 'status': 'running'}
    def save():
        tmp = output.with_name(output.name + '.tmp')
        tmp.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
        tmp.replace(output)
    def pin():
        if args.cpu is not None: os.sched_setaffinity(0, {args.cpu})
    save()
    try:
        revision = subprocess.run(['git','-C',str(problems_root),'rev-parse','HEAD'],
                                  capture_output=True,text=True,check=True).stdout.strip()
        dirty = subprocess.run(['git','-C',str(problems_root),'status','--porcelain','--untracked-files=no'],
                               capture_output=True,text=True,check=True).stdout
        report['inputs']['problem_revision'] = revision
        report['inputs']['problem_tracked_status'] = dirty
        report['inputs']['runner_sha256'] = digest(HERE / 'run_suffix_automaton_official.py')
        report['inputs']['header_sha256'] = {}
        for variant, root in roots.items():
            header = root / HEADER
            if not header.is_file(): raise RuntimeError('missing header: ' + str(header))
            report['inputs']['header_sha256'][variant] = digest(header)
        report['inputs']['driver_sha256'] = {}
        report['inputs']['cases_sha256'] = {}
        report['inputs']['checker_sha256'] = {}
        report['inputs']['checker_source_sha256'] = {}
        for problem in names:
            driver = source_root / DRIVERS[problem]
            folder = problems_root / 'string' / problem
            checker, checker_source = folder / 'checker', folder / 'checker.cpp'
            for path in (driver, checker, checker_source):
                if not path.is_file(): raise RuntimeError('missing file: ' + str(path))
            if not os.access(checker, os.X_OK): raise RuntimeError('checker is not executable: ' + str(checker))
            report['inputs']['driver_sha256'][problem] = digest(driver)
            report['inputs']['checker_sha256'][problem] = digest(checker)
            report['inputs']['checker_source_sha256'][problem] = digest(checker_source)
            report['inputs']['cases_sha256'][problem] = {}
            for case in cases:
                inp, ans = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
                for path in (inp, ans):
                    if not path.is_file(): raise RuntimeError('missing generated case: ' + str(path))
                report['inputs']['cases_sha256'][problem][case] = {
                    'input': digest(inp), 'expected': digest(ans), 'input_bytes': inp.stat().st_size}
        save()
        binaries = {}
        for problem in names:
            for variant, root in roots.items():
                binary = build / (problem + '-' + variant)
                command = [args.cxx, *flags, '-I', str(root), str(source_root / DRIVERS[problem]), '-o', str(binary)]
                result = subprocess.run(command, capture_output=True, text=True)
                record = {'command': command, 'returncode': result.returncode,
                          'stdout': result.stdout, 'stderr': result.stderr}
                report['compile'][problem + ':' + variant] = record
                if result.returncode:
                    save()
                    raise RuntimeError('compile failed: ' + problem + ' ' + variant)
                record['binary_sha256'] = digest(binary)
                binaries[(problem,variant)] = binary
                save()
        for problem_index, problem in enumerate(names):
            folder = problems_root / 'string' / problem
            checker = folder / 'checker'
            for case_index, case in enumerate(cases):
                inp, ans = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
                expected_hash = None
                result_dir = build / 'outputs' / problem / case
                result_dir.mkdir(parents=True, exist_ok=True)
                for run in range(args.warmups + args.repeats):
                    order = (('baseline','candidate') if (problem_index + case_index + run) % 2 == 0
                             else ('candidate','baseline'))
                    for variant in order:
                        actual = result_dir / f'{variant}-{run}.out'
                        command = [str(binaries[(problem,variant)])]
                        row = {'problem': problem, 'case': case, 'variant': variant, 'run': run,
                               'warmup': run < args.warmups, 'command': command,
                               'input': str(inp), 'expected': str(ans), 'actual': str(actual)}
                        report['runs'].append(row)
                        try:
                            with inp.open('rb') as input_file, actual.open('wb') as output_file:
                                begin = time.monotonic_ns()
                                result = subprocess.run(command, stdin=input_file, stdout=output_file,
                                                        stderr=subprocess.PIPE, timeout=args.timeout,
                                                        preexec_fn=pin if args.cpu is not None else None)
                                row['process_wall_ns'] = time.monotonic_ns() - begin
                        except subprocess.TimeoutExpired as exc:
                            row['failure'] = 'solution timeout'
                            row['timeout_seconds'] = exc.timeout
                            save()
                            raise RuntimeError(f'solution timeout: {problem} {case} {variant} {run}') from exc
                        row['returncode'] = result.returncode
                        row['stderr'] = result.stderr.decode('utf-8','replace')
                        if result.returncode:
                            save()
                            raise RuntimeError(f'solution failed: {problem} {case} {variant} {run}')
                        row['output_sha256'] = digest(actual)
                        row['output_bytes'] = actual.stat().st_size
                        checker_command = [str(checker), str(inp), str(actual), str(ans)]
                        try:
                            checked = subprocess.run(checker_command, capture_output=True, text=True,
                                                     timeout=args.checker_timeout,
                                                     preexec_fn=pin if args.cpu is not None else None)
                        except subprocess.TimeoutExpired as exc:
                            row['checker'] = {'command': checker_command, 'failure': 'timeout', 'seconds': exc.timeout}
                            save()
                            raise RuntimeError(f'checker timeout: {problem} {case} {variant} {run}') from exc
                        row['checker'] = {'command': checker_command, 'returncode': checked.returncode,
                                          'stdout': checked.stdout, 'stderr': checked.stderr}
                        if checked.returncode:
                            save()
                            raise RuntimeError(f'checker failed: {problem} {case} {variant} {run}')
                        if expected_hash is not None and row['output_sha256'] != expected_hash:
                            save()
                            raise RuntimeError(f'baseline/candidate outputs differ: {problem} {case} {variant} {run}')
                        expected_hash = row['output_sha256']
                        print(f"{problem} {case} {variant} run={run} warmup={row['warmup']} wall={row['process_wall_ns']/1e6:.3f}ms", flush=True)
                        save()
        for problem in names:
            report['summary'][problem] = {}
            for case in cases:
                report['summary'][problem][case] = {}
                for variant in roots:
                    rows = [r for r in report['runs'] if r['problem']==problem and r['case']==case and
                            r['variant']==variant and not r['warmup']]
                    report['summary'][problem][case][variant] = stats([r['process_wall_ns'] for r in rows])
        for problem in names:
            folder = problems_root / 'string' / problem
            if digest(folder / 'checker') != report['inputs']['checker_sha256'][problem]:
                raise RuntimeError('checker changed during measurement: ' + problem)
            for case in cases:
                initial = report['inputs']['cases_sha256'][problem][case]
                if (digest(folder / 'in' / (case + '.in')) != initial['input'] or
                    digest(folder / 'out' / (case + '.out')) != initial['expected']):
                    raise RuntimeError('case changed during measurement: ' + problem + '/' + case)
        for variant, root in roots.items():
            if digest(root / HEADER) != report['inputs']['header_sha256'][variant]:
                raise RuntimeError('header changed during measurement: ' + variant)
        report['status'] = 'complete'
    except BaseException as exc:
        report['status'] = 'failed'
        report['failure'] = f'{type(exc).__name__}: {exc}'
        raise
    finally:
        save()
    print('results:', output)

if __name__ == '__main__':
    main()
