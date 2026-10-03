#!/usr/bin/env python3
"""Offline focused FurthestPair validation; source compiler environment first.

prepare verifies cached official bytes and creates independent real case files.
final runs only after the header/tests/docs freeze and before benchmark timing.
Never updates .verification/current.json or downloads data.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import resource
import signal
import shlex
import shutil
import subprocess
import time
import tomllib

ROOT = Path('/workspace/Blueberry-library')
BASE = ROOT / '.verification/furthest-pair-resume'
UPSTREAM = ROOT / '.deps/cache/online-judge-tools/library-checker-problems'
UPSTREAM_REVISION = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
TARGETS = [('verify/geometry/furthest-pair.test.cpp', 'geo/furthest_pair'),
           ('verify/geometry/static-convex-hull.test.cpp', 'geo/static_convex_hull')]
HEADER = 'blueberry/geometry/furthest-pair.hpp'
DOC = 'docs/geometry/furthest-pair.md'
RANDOM = ['tests/random/furthest-pair.cpp', 'tests/random/convex-hull.cpp']
SOURCES = sorted(str(p.relative_to(ROOT)) for p in (ROOT / 'blueberry').rglob('*.hpp')) + [
    DOC, *[target for target, _ in TARGETS], *RANDOM,
    'scripts/check_docs.py', 'scripts/verify_with_metrics.py', '.verify-helper/config.toml']
MODES = [('g++', 'gnu++20'), ('g++', 'gnu++23'), ('clang++', 'gnu++20'), ('clang++', 'gnu++23')]
FULL_REPORT = ROOT / '.verification/current.json'
PAUSE = Path('/tmp/blueberry-setup/furthest-pair-validation.pause')


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha(path):
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for data in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(data)
    return digest.hexdigest()


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n')


def git(*args, cwd=ROOT):
    return subprocess.check_output(['git', *args], cwd=cwd)


def metrics_module():
    spec = importlib.util.spec_from_file_location('furthest_pair_metrics', ROOT / 'scripts/verify_with_metrics.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def official_identity(problem_relative, target):
    require(git('rev-parse', 'HEAD', cwd=UPSTREAM).decode().strip() == UPSTREAM_REVISION,
            'Cached official source revision changed')
    paths = git('ls-files', '-z', problem_relative, 'common', cwd=UPSTREAM).decode().split('\0')
    source_hashes = {}
    for name in filter(None, paths):
        p = UPSTREAM / name
        require(p.is_file() and not p.is_symlink(), 'Missing or symlinked official tracked source: ' + name)
        require(p.read_bytes() == git('show', UPSTREAM_REVISION + ':' + name, cwd=UPSTREAM),
                'Official tracked source differs from pinned Git blob: ' + name)
        source_hashes[name] = sha(p)
    problem = UPSTREAM / problem_relative
    expected_hashes = json.loads((problem / 'hash.json').read_text())
    expected_cases = sum(row['number'] for row in tomllib.loads((problem / 'info.toml').read_text())['tests'])
    require(len(expected_hashes) == 2 * expected_cases, 'Official hash inventory is incomplete')
    selected = []
    for extension, subdir in (('.in', 'in'), ('.out', 'out')):
        files = sorted((problem / subdir).glob('*' + extension))
        require(len(files) == expected_cases, 'Missing cached official ' + subdir + ' cases')
        for p in files:
            require(not p.is_symlink() and p.is_file(), 'Cached testcase is not a real file: ' + str(p))
            require(p.name in expected_hashes and sha(p) == expected_hashes[p.name],
                    'Cached testcase differs from official hash.json: ' + str(p))
            selected.append(p)
    require({p.name for p in selected} == set(expected_hashes), 'Cached official case inventory mismatch')
    checker = problem / 'checker'
    require(checker.is_file() and not checker.is_symlink() and os.access(checker, os.X_OK),
            'Cached official checker is missing/nonexecutable')
    return selected, expected_hashes, {'upstream_revision': UPSTREAM_REVISION,
            'official_tracked_source_sha256': source_hashes,
            'checker': str(checker), 'checker_sha256': sha(checker),
            'checker_source_sha256': sha(problem / 'checker.cpp'),
            'case_count': expected_cases, 'target': target,
            'official_problem': 'https://judge.yosupo.jp/problem/' + problem.name,
            'scope': 'Official direct furthest_pair coverage or dependent convex_hull regression, with the original official checker.'}


def prepare(out):
    full_hash = sha(FULL_REPORT)
    report = {'started_at': datetime.now(timezone.utc).isoformat(), 'succeeded': False,
              'driver_sha256': sha(__file__), 'original_full_report_sha256': full_hash}
    try:
        metrics = metrics_module()
        old = json.loads(FULL_REPORT.read_text())
        datasets = []
        for target, problem_relative in TARGETS:
            originals, hashes, dataset = official_identity(problem_relative, target)
            case_dir = out / 'datasets' / Path(problem_relative).name
            case_dir.mkdir(parents=True)
            for p in originals:
                q = case_dir / p.name
                shutil.copyfile(p, q)
                require(q.is_file() and not q.is_symlink() and sha(q) == hashes[q.name],
                        'Copied official testcase mismatch: ' + q.name)
            digest = metrics.digest_files(list(case_dir.iterdir()), case_dir)
            prior = next((row for row in old['results'] if row['path'] == target), None)
            if prior is not None:
                require(prior['status'] == 'passed' and prior['dataset_sha256'] == digest and
                        prior['case_count'] == dataset['case_count'], 'Official cache differs from preserved full-run dataset')
            dataset['matches_preserved_full_report'] = prior is not None
            dataset.update(directory=str(case_dir), dataset_sha256=digest, files_sha256=hashes,
                           original_full_report_sha256=full_hash, storage='independent real files; no symlink testcase paths')
            datasets.append(dataset)
        write(out / 'datasets.json', datasets)
        report.update(dataset_file=str((out / 'datasets.json').relative_to(ROOT)),
                      dataset_file_sha256=sha(out / 'datasets.json'),
                      suites=[{'target': d['target'], 'case_count': d['case_count'], 'dataset_sha256': d['dataset_sha256']}
                              for d in datasets], official_hashes_verified=True, succeeded=True)
        require(sha(FULL_REPORT) == full_hash, 'Preserved full report changed during cache preparation')
    except BaseException as exc:
        report['failure'] = repr(exc)
        raise
    finally:
        report['finished_at'] = datetime.now(timezone.utc).isoformat()
        write(out / 'report.json', report)
    print('PASS cache preparation: ' + str(out / 'report.json'), flush=True)


def flags(compiler, standard):
    return [compiler, '-std=' + standard, '-O2', '-pipe', '-Wall', '-Wextra', '-Wshadow', '-Werror',
            '-isystem', str(ROOT / '.deps/ac-library'), '-I', str(ROOT)]


class Runner:
    def __init__(self, out, preparation, expected_header):
        self.out = out
        require(sha(ROOT / HEADER) == expected_header, 'Header differs from explicit frozen hash')
        self.metrics = metrics_module()
        self.datasets = json.loads((preparation / 'datasets.json').read_text())
        self.report = {'started_at': datetime.now(timezone.utc).isoformat(),
                       'revision': git('rev-parse', 'HEAD').decode().strip(),
                       'environment': self.metrics.environment_info(),
                       'compiler_versions': {c: subprocess.check_output([c, '--version'], text=True).splitlines()[0]
                                             for c in ('g++', 'clang++')},
                       'source_sha256': {p: sha(ROOT / p) for p in SOURCES},
                       'driver_sha256': sha(__file__), 'original_full_report_sha256': sha(FULL_REPORT),
                       'dataset_file': str((preparation / 'datasets.json').relative_to(ROOT)),
                       'dataset_file_sha256': sha(preparation / 'datasets.json'),
                       'scope': 'Focused furthest_pair correctness/contracts plus official furthest_pair and convex_hull regression; not whole-repository official verification or comparative timing.',
                       'steps': [], 'binaries_sha256': {}, 'succeeded': False}
        require('14.' in self.report['compiler_versions']['g++'] and '19.' in self.report['compiler_versions']['clang++'],
                'Expected provisioned GCC14/Clang19 toolchains')
        for name in SOURCES:
            snapshot = self.out / 'source-snapshot' / name
            snapshot.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, snapshot)
        shutil.copyfile(__file__, self.out / 'validation-driver.py')
        self.check_sources()
        self.check_dataset()
        self.save()

    def save(self):
        write(self.out / 'report.json', self.report)

    def check_sources(self):
        for path, expected in self.report['source_sha256'].items():
            require(sha(ROOT / path) == expected, 'Frozen source changed: ' + path)
        require(sha(FULL_REPORT) == self.report['original_full_report_sha256'], 'Preserved full report changed')

    def check_dataset(self):
        for dataset in self.datasets:
            directory = Path(dataset['directory'])
            require({p.name for p in directory.iterdir()} == set(dataset['files_sha256']), 'Prepared dataset inventory changed')
            for name, expected in dataset['files_sha256'].items():
                p = directory / name
                require(p.is_file() and not p.is_symlink() and sha(p) == expected, 'Prepared dataset changed: ' + name)
            require(self.metrics.digest_files(list(directory.iterdir()), directory) == dataset['dataset_sha256'],
                    'Prepared dataset digest changed')
            require(sha(dataset['checker']) == dataset['checker_sha256'], 'Official checker binary changed')
            require(git('rev-parse', 'HEAD', cwd=UPSTREAM).decode().strip() == dataset['upstream_revision'],
                    'Official source revision changed')

    def run(self, label, command, *, source=None, timeout=180, env=None, expected_returncode=0, expected_text=None):
        while PAUSE.exists():
            write(self.out / 'activity.json', {'state': 'paused', 'next_step': label})
            time.sleep(1)
        self.check_sources()
        row = {'label': label, 'command': command, 'status': 'running', 'expected_returncode': expected_returncode}
        if expected_text is not None: row['expected_output_text'] = expected_text
        if env:
            row['environment'] = {k: env[k] for k in ('BLUEBERRY_RANDOM_SEED', 'ASAN_OPTIONS', 'UBSAN_OPTIONS') if k in env}
        self.report['steps'].append(row)
        self.save()
        write(self.out / 'activity.json', {'state': 'running', 'step': label})
        log = self.out / ('%03d.log' % len(self.report['steps']))
        started = time.monotonic()
        try:
            result = subprocess.run(command, cwd=ROOT, input=source, text=True, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT, timeout=timeout, env=env, check=False)
            log.write_text(result.stdout)
            matched = result.returncode == expected_returncode and (expected_text is None or expected_text in result.stdout)
            row.update(returncode=result.returncode, status='passed' if matched else 'failed')
        except subprocess.TimeoutExpired as exc:
            output = exc.stdout or b''
            if isinstance(output, bytes): output = output.decode(errors='replace')
            log.write_text(output + '\nTIMEOUT\n')
            row.update(returncode=124, status='failed')
        row.update(elapsed_seconds=time.monotonic() - started, log=str(log.relative_to(ROOT)), log_sha256=sha(log))
        self.save()
        print(row['status'].upper() + ' ' + label, flush=True)
        require(row['status'] == 'passed', 'Validation failed; see ' + str(log))

    def binary(self, path):
        self.report['binaries_sha256'][str(path.relative_to(ROOT))] = sha(path)
        self.save()


def final(runner):
    blocks = [code for code in re.findall(r'```cpp\n(.*?)\n```', (ROOT / DOC).read_text(), re.S)
              if re.search(r'\bint\s+main\s*\(', code)]
    require(blocks, 'No complete executable documentation examples')
    runner.report['documentation_examples'] = len(blocks)
    for index, code in enumerate(blocks, 1):
        (runner.out / f'example-{index}.cpp').write_text(code + '\n')
        for compiler, standard in MODES:
            binary = runner.out / f'example-{index}-{compiler}-{standard}'
            runner.run(f'doc {index} compile {compiler} {standard}',
                       flags(compiler, standard) + ['-x', 'c++', '-', '-o', str(binary)], source=code)
            runner.binary(binary)
            runner.run(f'doc {index} execute {compiler} {standard}', [str(binary)], timeout=30)
    official_binaries = {}
    runner.report['negative_contract_policy'] = (
        'Unsigned/floating/128-bit coordinate instantiations must fail at static_assert in debug and release. '
        'Eight out-of-bound x/y singleton/two-point cases must abort in debug; none are executed in release. '
        'A valid exact-boundary control must succeed. Expected rejection exit codes are recorded separately from status.')
    for compiler, standard in MODES:
        mode = compiler + '-' + standard
        for header in (HEADER, 'blueberry/geometry/convex-hull.hpp', 'blueberry/all.hpp'):
            for duplicate in (False, True):
                for release in (False, True):
                    code = ('#include "' + header + '"\n') * (2 if duplicate else 1) + 'int main() {}\n'
                    binary = runner.out / f'header-{Path(header).stem}-{mode}-{int(duplicate)}-{int(release)}'
                    runner.run(f'header {header} {mode} duplicate={duplicate} release={release}',
                               flags(compiler, standard) + (['-DNDEBUG'] if release else []) +
                               ['-x', 'c++', '-', '-o', str(binary)], source=code)
        for typename, label in [('unsigned long long', 'unsigned'), ('float', 'float'), ('__int128', 'int128')]:
            code = '#include "' + HEADER + '"\nint main() {\n'
            code += 'const std::vector<std::pair<' + typename + ', ' + typename + '>> points;\n'
            code += 'return blueberry::furthest_pair(points).first;\n}\n'
            (runner.out / ('invalid-type-' + label + '.cpp')).write_text(code)
            for release in (False, True):
                runner.run(f'contract type {label} {mode} release={release}',
                           flags(compiler, standard) + (['-DNDEBUG'] if release else []) +
                           ['-fsyntax-only', '-x', 'c++', '-'], source=code, expected_returncode=1,
                           expected_text='requires signed integral coordinates up to 64 bits')
        code = '''#include <cstdlib>
#include "blueberry/geometry/furthest-pair.hpp"
int main(int argc, char** argv) {
  if (argc != 2) return 4;
  const int scenario = std::atoi(argv[1]);
  const long long limit = (1LL << 62) - 1;
  if (scenario == 0) {
    const std::vector<std::pair<long long, long long>> points{{-limit,-limit},{limit,limit}};
    return blueberry::furthest_pair(points) == std::pair<int,int>{0,1} ? 0 : 5;
  }
  std::vector<std::pair<long long, long long>> points(scenario <= 4 ? 1 : 2, {0,0});
  const int index = (scenario - 1) % 4;
  auto& coordinate = index < 2 ? points.back().first : points.back().second;
  coordinate = index % 2 == 0 ? limit + 1 : -limit - 1;
  (void)blueberry::furthest_pair(points);
  return 0;
}
'''
        (runner.out / 'coordinate-contract.cpp').write_text(code)
        binary = runner.out / ('coordinate-contract-' + mode)
        runner.run('contract coordinate compile ' + mode,
                   flags(compiler, standard) + ['-x', 'c++', '-', '-o', str(binary)], source=code)
        runner.binary(binary)
        runner.run('contract valid exact boundary ' + mode, [str(binary), '0'])
        for scenario in range(1, 9):
            runner.run(f'contract coordinate assertion {mode} case={scenario}', [str(binary), str(scenario)],
                       expected_returncode=-signal.SIGABRT, expected_text='Assertion')
        for source in RANDOM:
            binary = runner.out / f'random-{Path(source).stem}-{mode}'
            runner.run(f'random compile {source} {mode}', flags(compiler, standard) + [source, '-o', str(binary)])
            runner.binary(binary)
            for seed in range(1, 21):
                runner.run(f'random {source} {mode} seed={seed}', [str(binary), str(seed)],
                           env=dict(os.environ, BLUEBERRY_RANDOM_SEED=str(seed)))
        for target, _ in TARGETS:
            binary = runner.out / f'verify-{Path(target).stem}-{mode}'
            runner.run(f'verify compile {target} {mode}', flags(compiler, standard) +
                       ['-D_GLIBCXX_ASSERTIONS', target, '-o', str(binary)])
            runner.binary(binary)
            if (compiler, standard) == MODES[0]: official_binaries[target] = binary
    for source in RANDOM:
        binary = runner.out / ('sanitized-' + Path(source).stem)
        command = [('-O1' if x == '-O2' else x) for x in flags('clang++', 'gnu++23')]
        command += ['-g', '-fno-omit-frame-pointer', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                    source, '-o', str(binary)]
        runner.run('sanitizer compile ' + source, command)
        runner.binary(binary)
        for seed in (1, 20261003):
            runner.run(f'sanitizer {source} seed={seed}', [str(binary), str(seed)], timeout=300,
                       env=dict(os.environ, BLUEBERRY_RANDOM_SEED=str(seed), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
                                UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'))
    runner.check_dataset()
    runner.report['official'] = []
    for dataset in runner.datasets:
        official_binary = official_binaries[dataset['target']]
        problem = Path(dataset['directory']).name
        names = sorted(Path(name).stem for name in dataset['files_sha256'] if name.endswith('.in'))
        row = {**dataset, 'compiler': 'g++', 'standard': 'gnu++20', 'repeats': 3, 'runs': [], 'status': 'not_run',
               'binary': str(official_binary.relative_to(ROOT)), 'binary_sha256': sha(official_binary)}
        runner.report['official'].append(row)
        samples = []
        for repeat in range(1, 4):
            raw_log = runner.out / f'official-{problem}-{repeat}.raw.json'
            normalized = runner.out / f'official-{problem}-{repeat}.json'
            command = ['oj', 'test', '-c', shlex.quote(str(official_binary)), '-d', dataset['directory'],
                       '--judge-command', dataset['checker'], '--tle', '60', '--log-file', str(raw_log)]
            data = None
            try:
                runner.run(f'official {problem} official checker repeat={repeat}', command, timeout=600)
            finally:
                if raw_log.exists():
                    data = runner.metrics.read_samples(json.loads(raw_log.read_text()), names)
                    write(normalized, data)
                    row['runs'].append({'raw_log': str(raw_log.relative_to(ROOT)), 'raw_log_sha256': sha(raw_log),
                                        'log': str(normalized.relative_to(ROOT)), 'log_sha256': sha(normalized),
                                        'case_count': len(data)})
                    samples.append(data)
                runner.save()
            require(data is not None and all(case['status'] == 'AC' and case['exitcode'] == 0 for case in data),
                    'Official checker rejection or missing raw case log')
        row.update(status='passed', **runner.metrics.summarize(samples))
    runner.check_dataset()
    for path, expected in runner.report['binaries_sha256'].items():
        require(sha(ROOT / path) == expected, 'Compiled validation binary changed: ' + path)
    runner.check_sources()
    runner.report['succeeded'] = True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['prepare', 'final'])
    parser.add_argument('--output', type=Path)
    parser.add_argument('--preparation', type=Path, default=BASE / 'preparation/cache')
    parser.add_argument('--header-sha256')
    args = parser.parse_args()
    out = (args.output or BASE / ('preparation/cache' if args.action == 'prepare' else 'final')).resolve()
    require(out.is_relative_to(BASE), 'Output must remain within owned evidence directory')
    if args.action == 'final': require(args.header_sha256 is not None, 'Supply the explicit frozen header hash')
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    out.mkdir(parents=True, exist_ok=False)
    if args.action == 'prepare':
        prepare(out)
        return
    runner = None
    try:
        runner = Runner(out, args.preparation.resolve(), args.header_sha256)
        final(runner)
    except BaseException as exc:
        if runner is not None: runner.report['failure'] = repr(exc)
        else: write(out / 'initialization-failure.json', {'failure': repr(exc), 'succeeded': False})
        raise
    finally:
        if runner is not None:
            runner.report['finished_at'] = datetime.now(timezone.utc).isoformat()
            runner.save()
            write(out / 'activity.json', {'state': 'finished', 'succeeded': runner.report['succeeded']})
    print('PASS focused validation: ' + str(out / 'report.json'), flush=True)


if __name__ == '__main__':
    main()
