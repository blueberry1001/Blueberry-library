#!/usr/bin/env python3
"""Serial focused offline validation; source /tmp/blueberry-setup/env.sh first.

Use a fresh --output for each run. The independent random matrix is not repeated;
this runner adds include/API/rollback probes, sanitizers, and official19x3 verdicts.
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
BASE = ROOT / '.verification/offline-dynamic-connectivity'
HEADER = 'blueberry/graph/offline-dynamic-component-sum.hpp'
RANDOM = 'tests/random/offline-dynamic-component-sum.cpp'
VERIFY = 'verify/graph/offline-dynamic-component-sum.test.cpp'
LC_REVISION = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
SMOKE = r'''
#include <utility>
#include <vector>
struct Value {
  long long value;
  Value() = delete;
  explicit Value(long long v) : value(v) {}
  Value(const Value&) = default;
  Value& operator=(const Value&) = default;
  Value(Value&&) = delete;
  Value& operator=(Value&&) = delete;
  friend Value operator+(const Value& a, const Value& b) { return Value(a.value + b.value); }
};
bool equal(const std::vector<Value>& a, const std::vector<long long>& b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i) if (a[i].value != b[i]) return false;
  return true;
}
int main() {
  blueberry::OfflineDynamicComponentSum<Value> empty;
  if (empty.size() != 0 || !empty.solve().empty()) return 1;
  std::vector<Value> values{Value(4), Value(7), Value(11)};
  blueberry::OfflineDynamicComponentSum<Value> g(values);
  if (g.size() != 3 || g.query(0) != 0) return 2;
  g.add_edge(0, 1); g.add_edge(1, 0); g.add_value(1, Value(5));
  if (g.query(0) != 1 || !g.remove_edge(1, 0) || g.query(1) != 2) return 3;
  if (!g.remove_edge(0, 1) || g.remove_edge(0, 1)) return 4;
  if (g.query(0) != 3 || g.query(1) != 4) return 5;
  g.add_edge(2, 2);
  if (g.query(2) != 5 || !g.remove_edge(2, 2) || g.remove_edge(2, 2)) return 6;
  const auto& constant = g;
  const std::vector<long long> expected{4, 16, 16, 4, 12, 11};
  if (!equal(constant.solve(), expected) || !equal(constant.solve(), expected)) return 7;
  const auto copied = g;
  g.add_edge(0, 2); g.add_value(2, Value(-3));
  if (g.query(0) != 6 || !equal(g.solve(), {4, 16, 16, 4, 12, 11, 12})) return 8;
  if (!equal(copied.solve(), expected) || values[1].value != 7) return 9;
  return 0;  // Active checks also run under NDEBUG.
}
'''
ROLLBACK = r'''
#include "blueberry/data-structure/rollback-union-find.hpp"
int main() {
  blueberry::RollbackUnionFind empty(0);
  empty.snapshot(); empty.rollback();
  if (empty.size() != 0 || empty.components() != 0 || empty.state() != 0) return 1;
  blueberry::RollbackUnionFind uf(5);
  if (!uf.merge(0, 1) || uf.state() != 1 || uf.components() != 4) return 2;
  uf.snapshot();
  if (uf.merge(1, 0) || uf.state() != 2 || uf.components() != 4) return 3;
  if (!uf.merge(2, 3) || !uf.merge(0, 2) || uf.components() != 2 || uf.comp_size(3) != 4) return 4;
  const int state = uf.state();
  if (!uf.same(1, 3) || uf.component_size(0) != 4 || uf.state() != state) return 5;
  uf.undo();
  if (uf.state() != 3 || uf.components() != 3 || uf.same(0, 2) || !uf.same(2, 3)) return 6;
  uf.rollback();
  if (uf.state() != 1 || uf.components() != 4 || uf.same(2, 3) || !uf.same(0, 1)) return 7;
  uf.undo(); uf.rollback(0);
  if (uf.state() != 0 || uf.components() != 5 || uf.same(0, 1)) return 8;
  for (int i = 0; i < 5; ++i) if (uf.component_size(i) != 1 || uf.leader(i) != i) return 9;
  return 0;
}
'''
BOUNDS = r'''
#include <cstdlib>
#include <vector>
#include "blueberry/graph/offline-dynamic-component-sum.hpp"
int main(int argc, char** argv) {
  if (argc != 4) return 2;
  const int operation = std::atoi(argv[1]), n = std::atoi(argv[2]), bad = std::atoi(argv[3]);
  blueberry::OfflineDynamicComponentSum<long long> g(std::vector<long long>(n, 1));
  switch (operation) {
    case 0: g.add_edge(bad, 0); break;
    case 1: g.add_edge(0, bad); break;
    case 2: g.remove_edge(bad, 0); break;
    case 3: g.remove_edge(0, bad); break;
    case 4: g.add_value(bad, 1); break;
    case 5: g.query(bad); break;
    default: return 3;
  }
  return 0;
}
'''


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def now():
    return datetime.now(timezone.utc).isoformat()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=BASE / 'focused')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    for name in ('sources', 'bin', 'logs', 'official'):
        (out / name).mkdir()
    shutil.copyfile(__file__, out / 'runner.py')
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    source_paths = sorted(set(ROOT.glob('blueberry/**/*.hpp')) | {ROOT / RANDOM, ROOT / VERIFY, Path(__file__).resolve()})
    frozen = {str(p.relative_to(ROOT)): sha(p) for p in source_paths}
    root_base_path, global_path = BASE / 'root-base.json', ROOT / '.verification/current.json'
    root_base = json.loads(root_base_path.read_text())
    root_base_hash = sha(root_base_path)
    report = {'schema_version': 1, 'started_at': now(), 'succeeded': False,
              'scope': 'Focused offline correctness; independent random matrix and full repository gate are separate.',
              'base_head': git('rev-parse', 'HEAD'), 'root_base_sha256': root_base_hash,
              'source_sha256': frozen, 'global_report_sha256': root_base['global_report_sha256'],
              'platform': platform.platform(), 'compilers': {}, 'binaries': {}, 'steps': [], 'official': [],
              'environment': {key: os.environ.get(key, '') for key in ('CPLUS_INCLUDE_PATH', 'CPATH', 'LD_LIBRARY_PATH')}}

    def save():
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')

    def unchanged():
        if sha(root_base_path) != root_base_hash or sha(global_path) != root_base['global_report_sha256']:
            raise RuntimeError('Recorded root base or global failed report changed')
        if git('rev-parse', 'HEAD') != root_base['base_commit']:
            raise RuntimeError('HEAD differs from recorded root base')
        refs = git('for-each-ref', '--format=%(refname) %(objectname)', 'refs/heads').splitlines()
        if sorted(refs) != sorted(root_base['refs']):
            raise RuntimeError('Branch refs differ from recorded root base')
        for path, expected in frozen.items():
            if sha(ROOT / path) != expected:
                raise RuntimeError('Frozen source changed: ' + path)

    def execute(kind, command, *, expected=0, stdin=None, stdout=None, env=None, timeout=180):
        log = out / 'logs' / (f'{len(report["steps"]):03d}-' + kind + '.log')
        row = {'kind': kind, 'command': [str(x) for x in command], 'cwd': str(ROOT), 'expected_returncode': expected,
               'started_at': now(), 'timeout_seconds': timeout, 'log': str(log.relative_to(out)), 'status': 'running'}
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
            with log.open('wb') as stream:
                input_file = Path(stdin).open('rb') if stdin else None
                output_file = Path(stdout).open('wb') if stdout else None
                try:
                    result = subprocess.run(row['command'], cwd=ROOT, stdin=input_file, stdout=output_file or stream,
                        stderr=stream, timeout=timeout, env={**os.environ, **(env or {})}, check=False)
                finally:
                    if input_file:
                        input_file.close()
                    if output_file:
                        output_file.close()
            row['returncode'] = result.returncode
            row['status'] = 'passed' if result.returncode == expected else 'failed'
            if stdout:
                row['stdout_sha256'] = sha(stdout)
            if row['status'] != 'passed':
                raise RuntimeError('Unexpected outcome; see ' + str(log))
        except Exception as exc:
            row['status'], row['error'] = 'failed', str(exc)
            raise
        finally:
            row['seconds'], row['finished_at'], row['log_sha256'] = time.monotonic() - start, now(), sha(log)
            save()
        return row

    def compile_source(cxx, standard, mode, source, binary, kind, extra=()):
        flags = [cxx, '-std=' + standard, '-O2', '-Wall', '-Wextra', '-Wshadow', '-Werror',
                 '-I', str(ROOT), '-isystem', str(ROOT / '.deps/ac-library')]
        if mode == 'release':
            flags.append('-DNDEBUG')
        execute(kind, flags + list(extra) + [str(source), '-o', str(binary)])
        report['binaries'][str(binary.relative_to(out))] = sha(binary)
        save()

    try:
        unchanged()
        dataset_path = BASE / 'datasets/report.json'
        data = json.loads(dataset_path.read_text())
        case_dir, checker = Path(data['dataset_directory']), Path(data['checker_path'])
        cases, hashes = sorted(case_dir.glob('*.in')), data['generated_files_sha256']
        if not data['succeeded'] or data['upstream_revision'] != LC_REVISION or data['case_count'] != 19 or len(cases) != 19:
            raise RuntimeError('Official dataset identity/count mismatch')
        if set(p.name for p in case_dir.iterdir()) != set(hashes):
            raise RuntimeError('Official dataset inventory mismatch')
        for name, expected in hashes.items():
            if sha(case_dir / name) != expected:
                raise RuntimeError('Official case changed: ' + name)
        if sha(checker) != data['checker_sha256']:
            raise RuntimeError('Official checker changed')
        report['datasets'] = {'report': str(dataset_path), 'report_sha256': sha(dataset_path),
            'upstream_revision': LC_REVISION, 'files_sha256': hashes, 'checker': str(checker), 'checker_sha256': sha(checker)}
        for cxx, version_prefix in [('g++', '14.2'), ('clang++', '19.1')]:
            path = shutil.which(cxx)
            version = subprocess.check_output([cxx, '--version'], text=True)
            if not path or version_prefix not in version:
                raise RuntimeError('Unexpected compiler version: ' + cxx)
            report['compilers'][cxx] = {'path': path, 'version': version, 'launcher_sha256': sha(path)}
        fixtures = {}
        for name, text in {'standalone': '#include "' + HEADER + '"\n' + SMOKE,
                           'duplicate': ('#include "' + HEADER + '"\n') * 2 + SMOKE,
                           'umbrella': '#include "blueberry/all.hpp"\n' + SMOKE,
                           'rollback': ROLLBACK, 'bounds': BOUNDS}.items():
            fixtures[name] = out / 'sources' / (name + '.cpp')
            fixtures[name].write_text(text)
        for cxx in ('g++', 'clang++'):
            for standard in ('gnu++20', 'gnu++23'):
                for mode in ('assert', 'release'):
                    unchanged()
                    tag = cxx.replace('+', 'p') + '-' + standard.replace('+', 'p') + '-' + mode
                    print('START ' + tag, flush=True)
                    for name in ('standalone', 'duplicate', 'umbrella', 'rollback'):
                        binary = out / 'bin' / (tag + '-' + name)
                        kind = 'rollback' if name == 'rollback' else 'smoke'
                        compile_source(cxx, standard, mode, fixtures[name], binary, kind + '-compile')
                        execute(kind + '-run', [binary])
                    if mode == 'assert':
                        binary = out / 'bin' / (tag + '-bounds')
                        compile_source(cxx, standard, mode, fixtures['bounds'], binary, 'bounds-compile')
                        probes = [(op, 2, bad) for op in range(6) for bad in (-1, 2)]
                        probes += [(op, 0, 0) for op in (0, 2, 4, 5)]
                        for op, count, bad in probes:
                            execute('bounds-rejection', [binary, str(op), str(count), str(bad)], expected=-signal.SIGABRT)
        unchanged()
        print('START sanitizer', flush=True)
        binary = out / 'bin/sanitizer'
        compile_source('clang++', 'gnu++20', 'assert', ROOT / RANDOM, binary, 'sanitizer-compile',
                       ['-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer'])
        for seed in (1, 20261003):
            execute('sanitizer-run', [binary, str(seed)], timeout=300,
                    env={'ASAN_OPTIONS': 'detect_leaks=1:halt_on_error=1', 'UBSAN_OPTIONS': 'halt_on_error=1'})
        unchanged()
        print('START official 19 cases x 3', flush=True)
        binary = out / 'bin/official'
        compile_source('g++', 'gnu++20', 'assert', ROOT / VERIFY, binary, 'official-compile')
        for repeat in range(1, 4):
            for case in cases:
                actual = out / 'official' / (case.stem + '-' + str(repeat) + '.actual')
                solve = execute('official-solve', [binary], stdin=case, stdout=actual, timeout=120)
                check = execute('official-check', [checker, case, actual, case.with_suffix('.out')], timeout=120)
                report['official'].append({'case': case.stem, 'repeat': repeat, 'status': 'accepted',
                    'actual': str(actual.relative_to(out)), 'actual_sha256': sha(actual),
                    'solve_log': solve['log'], 'checker_log': check['log']})
                save()
            print('PASS official repeat ' + str(repeat), flush=True)
        unchanged()
        if sha(dataset_path) != report['datasets']['report_sha256'] or sha(checker) != data['checker_sha256']:
            raise RuntimeError('Official report/checker changed during run')
        for name, expected in hashes.items():
            if sha(case_dir / name) != expected:
                raise RuntimeError('Official data changed during run: ' + name)
        for name, expected in report['binaries'].items():
            if sha(out / name) != expected:
                raise RuntimeError('Compiled binary changed: ' + name)
        report['source_unchanged'] = report['root_base_unchanged'] = report['global_report_unchanged'] = True
        report['datasets_unchanged'] = report['binaries_unchanged'] = report['succeeded'] = True
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
