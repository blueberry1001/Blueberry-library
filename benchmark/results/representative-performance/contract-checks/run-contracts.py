#!/usr/bin/env python3
"""Compile temporary contract probes against immutable experiment snapshots."""
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[4]
OUT = Path(__file__).resolve().parent
FAMILIES = {'dynamic-fenwick': 'dynamic-fenwick-tree.hpp',
            'persistent-segment': 'persistent-segment-tree.hpp'}
family = sys.argv[1]
if family not in FAMILIES:
    raise SystemExit('choose dynamic-fenwick or persistent-segment')
source = OUT / ('dynamic-fenwick-contract.cpp' if family == 'dynamic-fenwick' else 'persistent-contract.cpp')
result_path = OUT / (family + '-results.json')
if result_path.exists():
    raise SystemExit('Preserve existing contract results before rerunning')
digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
report = {'family': family, 'source_sha256': digest(source), 'steps': [], 'comparisons': []}
expected = None
configurations = [('g++', 'strict', ['-O2']), ('clang++', 'strict', ['-O2']),
                  ('clang++', 'ubsan', ['-O1', '-g', '-fsanitize=undefined', '-fno-sanitize-recover=all'])]
with tempfile.TemporaryDirectory(prefix='blueberry-contract-') as temporary:
    for compiler, mode, options in configurations:
        for variant in ['baseline', 'candidate']:
            include = ROOT / '.build/representative-performance' / (family + '-release') / variant / 'include'
            header = include / 'blueberry/data-structure' / FAMILIES[family]
            header_hash = digest(header)
            binary = Path(temporary) / f'{family}-{compiler}-{mode}-{variant}'
            command = [compiler, '-std=gnu++20', *options, '-Wall', '-Wextra', '-Wshadow', '-Werror',
                       '-I', str(include), str(source), '-o', str(binary)]
            log_path = OUT / f'{family}-{compiler}-{mode}-{variant}.log'
            with log_path.open('w') as log:
                log.write('$ ' + shlex.join(command) + '\n'); log.flush()
                started = time.perf_counter()
                build = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=60)
                step = {'compiler': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0],
                        'mode': mode, 'variant': variant, 'header': str(header.relative_to(ROOT)),
                        'header_sha256': header_hash, 'compile_command': command,
                        'compile_status': build.returncode, 'compile_seconds': time.perf_counter() - started,
                        'log': str(log_path.relative_to(ROOT))}
                report['steps'].append(step)
                result_path.write_text(json.dumps(report, indent=2) + '\n')
                if build.returncode:
                    raise SystemExit(build.returncode)
                run_command = [str(binary)]
                log.write('$ ' + shlex.join(run_command) + '\n'); log.flush()
                started = time.perf_counter()
                completed = subprocess.run(run_command, capture_output=True, text=True, timeout=30,
                                           env={**os.environ, 'UBSAN_OPTIONS': 'halt_on_error=1:print_stacktrace=1'})
                log.write(completed.stdout); log.write(completed.stderr)
                step.update(run_command=run_command, run_status=completed.returncode,
                            run_seconds=time.perf_counter() - started, stdout=completed.stdout,
                            stderr=completed.stderr, header_unchanged=digest(header) == header_hash)
                result_path.write_text(json.dumps(report, indent=2) + '\n')
                if completed.returncode:
                    raise SystemExit(completed.returncode)
                if expected is None:
                    expected = completed.stdout
                agreement = completed.stdout == expected
                report['comparisons'].append({'mode': mode, 'compiler': compiler, 'variant': variant,
                                              'output_matches': agreement})
                result_path.write_text(json.dumps(report, indent=2) + '\n')
                if not agreement or not step['header_unchanged']:
                    raise SystemExit('Output mismatch or immutable header modified')
                print(f'{family} {compiler} {mode} {variant}: PASS {completed.stdout.strip()}', flush=True)
report['source_unchanged'] = digest(source) == report['source_sha256']
report['all_passed'] = report['source_unchanged'] and all(c['output_matches'] for c in report['comparisons'])
result_path.write_text(json.dumps(report, indent=2) + '\n')
