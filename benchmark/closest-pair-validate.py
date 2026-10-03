#!/usr/bin/env python3
"""Focused, serial, offline closest_pair validation. Source env.sh before running.

Use a fresh --output directory for every run; existing evidence is never replaced.
This is correctness evidence, not a benchmark or full-repository official verdict.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import shutil
import signal
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
HEADER = 'blueberry/geometry/closest-pair.hpp'
RANDOM = 'tests/random/closest-pair.cpp'
VERIFY = 'verify/geometry/closest-pair.test.cpp'
LC_REVISION = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
SMOKE = r'''
#include <limits>
#include <utility>
#include <vector>
using Indices = std::pair<int, int>;
template<class C> bool check(const std::vector<std::pair<C,C>>& p, Indices expected) {
  const auto before = p;
  const auto* storage = p.data();
  const auto capacity = p.capacity();
  const auto result = blueberry::closest_pair(p);
  return result == expected && p == before && p.data() == storage && p.capacity() == capacity;
}
template<class C> bool typed() {
  return check<C>({}, {-1,-1}) && check<C>({{1,-1}}, {-1,-1}) &&
      check<C>({{1,1},{1,1}}, {0,1}) &&
      check<C>({{1,0},{2,0},{0,0},{3,0}}, {0,1}) &&
      check<C>({{4,4},{-2,-2},{4,4},{-2,-2}}, {0,2}) &&
      check<C>({{2,2},{0,0},{2,0},{0,2}}, {0,2});
}
int main() {
  constexpr long long b = (1LL << 62) - 1;
  if (!typed<signed char>() || !typed<short>() || !typed<int>() ||
      !typed<long>() || !typed<long long>()) return 1;
  if (!check<long long>({{b,-b}}, {-1,-1}) ||
      !check<long long>({{-b,b},{b,-b}}, {0,1}) ||
      !check<long long>({{-b,-b},{b,b},{-b,b},{b,-b}}, {0,2})) return 2;
  return 0;  // Checks stay active under -DNDEBUG.
}
'''
BOUNDS = r'''
#include <cstdlib>
#include <utility>
#include <vector>
#include "blueberry/geometry/closest-pair.hpp"
int main(int argc, char** argv) {
  if (argc != 4) return 2;
  const int axis = std::atoi(argv[1]), sign = std::atoi(argv[2]), n = std::atoi(argv[3]);
  std::vector<std::pair<long long,long long>> p(n, {0,0});
  const long long outside = sign * (1LL << 62);  // +(B+1) or -(B+1).
  if (axis == 0) p.back().first = outside; else p.back().second = outside;
  (void)blueberry::closest_pair(p);
  return 0;
}
'''


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def now():
    return datetime.now(timezone.utc).isoformat()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / '.verification/closest-pair/focused')
    parser.add_argument('--datasets', type=Path, default=ROOT / '.verification/closest-pair/datasets/report.json')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    for name in ('sources', 'bin', 'logs', 'official'):
        (out / name).mkdir()
    shutil.copyfile(__file__, out / 'runner.py')
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    source_paths = sorted(set(ROOT.glob('blueberry/**/*.hpp')) | {ROOT / RANDOM, ROOT / VERIFY, Path(__file__).resolve()})
    frozen = {str(p.relative_to(ROOT)): sha(p) for p in source_paths}
    frozen_global = ROOT / '.verification/current.json'
    global_hash = sha(frozen_global) if frozen_global.is_file() else None
    report = {'schema_version': 1, 'started_at': now(), 'succeeded': False,
              'scope': 'Focused serial offline correctness checks; no full official suite or publication claim.',
              'base_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
              'source_sha256': frozen, 'global_report_sha256_before': global_hash,
              'environment': {key: os.environ.get(key, '') for key in ('CPLUS_INCLUDE_PATH', 'CPATH', 'LD_LIBRARY_PATH')},
              'platform': platform.platform(), 'python': platform.python_version(),
              'compilers': {}, 'binaries': {}, 'steps': [], 'official': []}

    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')

    def unchanged():
        for path, expected in frozen.items():
            if sha(ROOT / path) != expected:
                raise RuntimeError('Source changed during focused run: ' + path)
        if (sha(frozen_global) if frozen_global.is_file() else None) != global_hash:
            raise RuntimeError('Global verification report changed during focused run')

    def execute(kind, command, *, expected='zero', timeout=180, stdin=None, stdout=None, env=None):
        index = len(report['steps'])
        log = out / 'logs' / f'{index:03d}-{kind}.log'
        row = {'kind': kind, 'command': [str(x) for x in command], 'cwd': str(ROOT),
               'started_at': now(), 'expected': expected, 'timeout_seconds': timeout,
               'log': str(log.relative_to(out)), 'status': 'running'}
        if stdin is not None:
            row['stdin'] = str(stdin)
        if stdout is not None:
            row['stdout'] = str(stdout)
        if env:
            row['extra_environment'] = env
        report['steps'].append(row)
        save()
        start = time.monotonic()
        try:
            with log.open('wb') as log_file:
                input_file = Path(stdin).open('rb') if stdin is not None else None
                output_file = Path(stdout).open('wb') if stdout is not None else None
                try:
                    result = subprocess.run(row['command'], cwd=ROOT, stdin=input_file,
                        stdout=output_file or log_file, stderr=log_file, timeout=timeout,
                        env={**os.environ, **(env or {})}, check=False)
                finally:
                    if input_file:
                        input_file.close()
                    if output_file:
                        output_file.close()
            row['returncode'] = result.returncode
            log_text = log.read_text(errors='replace')
            passed = (result.returncode == 0 if expected == 'zero' else
                      result.returncode == -signal.SIGABRT if expected == 'SIGABRT' else
                      result.returncode > 0 and 'closest_pair requires signed integral coordinates up to 64 bits' in log_text)
            row['status'] = 'passed' if passed else 'failed'
            if stdout is not None:
                row['stdout_sha256'] = sha(stdout)
            if not passed:
                raise RuntimeError('Unexpected result: ' + str(command) + '; see ' + str(log))
        except Exception as exc:
            row['status'] = 'failed'
            row['error'] = str(exc)
            raise
        finally:
            row['seconds'] = time.monotonic() - start
            row['finished_at'] = now()
            row['log_sha256'] = sha(log)
            save()
        return row

    def compile_source(cxx, standard, mode, source, binary, kind='compile', extra=()):
        flags = [cxx, '-std=' + standard, '-O2', '-Wall', '-Wextra', '-Wshadow', '-Werror',
                 '-I', str(ROOT), '-isystem', str(ROOT / '.deps/ac-library')]
        if mode == 'release':
            flags.append('-DNDEBUG')
        flags.extend(extra)
        execute(kind, flags + [str(source), '-o', str(binary)])
        report['binaries'][str(binary.relative_to(out))] = sha(binary)
        save()

    try:
        dataset_report = args.datasets.resolve()
        datasets = json.loads(dataset_report.read_text())
        if not datasets['succeeded'] or datasets['upstream_revision'] != LC_REVISION or datasets['case_count'] != 29:
            raise RuntimeError('Unexpected official dataset preparation identity')
        case_dir, checker = Path(datasets['dataset_directory']), Path(datasets['checker_path'])
        cases = sorted(case_dir.glob('*.in'))
        expected_files = datasets['generated_files_sha256']
        if len(cases) != 29 or set(p.name for p in case_dir.iterdir()) != set(expected_files):
            raise RuntimeError('Official case inventory differs')
        for name, expected in expected_files.items():
            if sha(case_dir / name) != expected:
                raise RuntimeError('Official dataset hash differs: ' + name)
        if sha(checker) != datasets['checker_sha256']:
            raise RuntimeError('Official checker binary differs')
        report['datasets'] = {'report': str(dataset_report), 'report_sha256': sha(dataset_report),
            'upstream_revision': LC_REVISION, 'case_count': 29, 'files_sha256': expected_files,
            'checker': str(checker), 'checker_sha256': sha(checker)}
        for cxx in ('g++', 'clang++'):
            executable = shutil.which(cxx)
            if executable is None:
                raise RuntimeError('Compiler unavailable: ' + cxx)
            version = subprocess.check_output([cxx, '--version'], text=True)
            required = '14.2' if cxx == 'g++' else '19.1'
            if required not in version:
                raise RuntimeError('Unexpected compiler version: ' + version.splitlines()[0])
            report['compilers'][cxx] = {'path': executable, 'launcher_sha256': sha(executable), 'version': version}
        smoke_sources = {}
        for name, includes in {
            'standalone': '#include "blueberry/geometry/closest-pair.hpp"\n',
            'duplicate': '#include "blueberry/geometry/closest-pair.hpp"\n' * 2,
            'umbrella': '#include "blueberry/all.hpp"\n',
        }.items():
            file = out / 'sources' / (name + '.cpp')
            file.write_text(includes + SMOKE)
            smoke_sources[name] = file
        bounds = out / 'sources/bounds.cpp'
        bounds.write_text(BOUNDS)
        rejected = {}
        for label, coord in [('float', 'float'), ('double', 'double'), ('unsigned', 'unsigned'), ('int128', '__int128')]:
            file = out / 'sources' / ('reject-' + label + '.cpp')
            file.write_text('#include "blueberry/geometry/closest-pair.hpp"\n' +
                f'int main() {{ const std::vector<std::pair<{coord},{coord}>> p; (void)blueberry::closest_pair(p); }}\n')
            rejected[label] = file
        for cxx in ('g++', 'clang++'):
            for standard in ('gnu++20', 'gnu++23'):
                for mode in ('assert', 'release'):
                    unchanged()
                    tag = cxx.replace('+', 'p') + '-' + standard.replace('+', 'p') + '-' + mode
                    print('START ' + tag, flush=True)
                    binary = out / 'bin' / (tag + '-random')
                    compile_source(cxx, standard, mode, ROOT / RANDOM, binary, 'random-compile')
                    for seed in range(1, 6):
                        execute('random-run', [binary, str(seed)])
                    for name, source in smoke_sources.items():
                        binary = out / 'bin' / (tag + '-' + name)
                        compile_source(cxx, standard, mode, source, binary, 'smoke-compile')
                        execute('smoke-run', [binary])
                    for label, source in rejected.items():
                        command = [cxx, '-std=' + standard, '-I', str(ROOT), '-fsyntax-only', str(source)]
                        if mode == 'release':
                            command.append('-DNDEBUG')
                        execute('type-rejection', command, expected='static-assert')
                    if mode == 'assert':
                        binary = out / 'bin' / (tag + '-bounds')
                        compile_source(cxx, standard, mode, bounds, binary, 'bounds-compile')
                        for axis in (0, 1):
                            for sign in (-1, 1):
                                for count in (1, 2, 4):
                                    execute('bounds-rejection', [binary, str(axis), str(sign), str(count)], expected='SIGABRT')
        unchanged()
        print('START sanitizer', flush=True)
        binary = out / 'bin/clangpp-sanitized'
        compile_source('clang++', 'gnu++20', 'assert', ROOT / RANDOM, binary, 'sanitizer-compile',
                       ['-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'])
        for seed in (1, 20261003):
            execute('sanitizer-run', [binary, str(seed)], timeout=300,
                    env={'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1', 'UBSAN_OPTIONS': 'halt_on_error=1'})
        unchanged()
        print('START official 29 cases x 3', flush=True)
        binary = out / 'bin/official'
        compile_source('g++', 'gnu++20', 'assert', ROOT / VERIFY, binary, 'official-compile')
        for repeat in range(1, 4):
            for case in cases:
                actual = out / 'official' / (case.stem + '-repeat' + str(repeat) + '.actual')
                solution = execute('official-solve', [binary], stdin=case, stdout=actual, timeout=120)
                verdict = execute('official-check', [checker, case, actual, case.with_suffix('.out')], timeout=120)
                report['official'].append({'case': case.stem, 'repeat': repeat, 'status': 'accepted',
                    'input_sha256': expected_files[case.name], 'expected_sha256': expected_files[case.with_suffix('.out').name],
                    'actual': str(actual.relative_to(out)), 'actual_sha256': sha(actual),
                    'solve_log': solution['log'], 'checker_log': verdict['log']})
                save()
            print('PASS official repeat ' + str(repeat), flush=True)
        unchanged()
        for name, expected in expected_files.items():
            if sha(case_dir / name) != expected:
                raise RuntimeError('Official case changed during run: ' + name)
        if sha(checker) != datasets['checker_sha256'] or sha(dataset_report) != report['datasets']['report_sha256']:
            raise RuntimeError('Official checker/preparation report changed during run')
        for name, expected in report['binaries'].items():
            if sha(out / name) != expected:
                raise RuntimeError('Compiled binary changed: ' + name)
        report['source_unchanged'] = report['dataset_unchanged'] = report['binaries_unchanged'] = True
        report['global_report_unchanged'] = True
        report['succeeded'] = True
    except Exception as exc:
        report['failure'] = str(exc)
        raise
    finally:
        report['finished_at'] = now()
        report['counts'] = dict(Counter(row['kind'] for row in report['steps']))
        report['passed_steps'] = sum(row['status'] == 'passed' for row in report['steps'])
        report['official_accepted'] = len(report['official'])
        report['generated_source_sha256'] = {str(p.relative_to(out)): sha(p) for p in sorted((out / 'sources').glob('*.cpp'))}
        save()
        print(json.dumps({'succeeded': report['succeeded'], 'passed_steps': report['passed_steps'],
                          'official_accepted': report['official_accepted'], 'report': str(out / 'report.json')}, indent=2), flush=True)


if __name__ == '__main__':
    main()
