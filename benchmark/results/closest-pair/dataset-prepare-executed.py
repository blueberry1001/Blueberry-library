#!/usr/bin/env python3
"""Generate pinned, offline official closest_pair cases in an isolated copy."""

import hashlib
import importlib.util
import json
import logging
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
from datetime import datetime, timezone


ROOT = Path('/workspace/Blueberry-library')
CACHE = ROOT / '.deps/cache/online-judge-tools/library-checker-problems'
REVISION = '1814c4e5205517e368bb57a8d1127eb961cfeaae'
OUT = Path(__file__).resolve().parent
SOURCE = OUT / 'source'
PROBLEM = SOURCE / 'geo/closest_pair'
CASES = OUT / 'cases'


def now():
    return datetime.now(timezone.utc).isoformat()


def digest(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1048576), b''):
            h.update(block)
    return h.hexdigest()


def git(*args):
    return subprocess.check_output(['git', *args], cwd=CACHE)


def save():
    (OUT / 'report.json').write_text(json.dumps(report, indent=2) + '\n')


def descriptor(value):
    return str(getattr(value, 'name', value))


def logged_run(command, **kwargs):
    """Keep each command, failure, timeout, and diagnostic stream before raising."""
    command = [str(x) for x in command]
    number = len(report['commands']) + 1
    logpath = OUT / 'logs' / f'{number:03d}.log'
    entry = {'command': command, 'started_at': now(), 'status': 'running',
             'log': str(logpath.relative_to(OUT))}
    report['commands'].append(entry)
    check = kwargs.pop('check', False)
    kwargs.setdefault('timeout', 180)
    kwargs.setdefault('cwd', SOURCE)
    entry['cwd'] = str(kwargs['cwd'])
    entry['timeout_seconds'] = kwargs['timeout']
    if 'stdin' in kwargs:
        entry['stdin'] = descriptor(kwargs['stdin'])
    if 'stdout' in kwargs:
        entry['stdout'] = descriptor(kwargs['stdout'])
    save()
    begin = time.monotonic()
    try:
        with logpath.open('wb') as log:
            kwargs.setdefault('stdout', log)
            kwargs.setdefault('stderr', log)
            result = subprocess.run(command, **kwargs)
            for captured in (result.stdout, result.stderr):
                if captured is not None:
                    log.write(captured.encode() if isinstance(captured, str) else captured)
        entry['returncode'] = result.returncode
        entry['status'] = 'passed' if result.returncode == 0 else 'failed'
    except BaseException as exc:
        entry['status'] = 'timeout' if isinstance(exc, subprocess.TimeoutExpired) else 'failed'
        entry['exception'] = repr(exc)
        if isinstance(exc, subprocess.TimeoutExpired):
            with logpath.open('ab') as log:
                for captured in (exc.stdout, exc.stderr):
                    if captured:
                        log.write(captured.encode() if isinstance(captured, str) else captured)
        raise
    finally:
        entry['finished_at'] = now()
        entry['elapsed_seconds'] = time.monotonic() - begin
        if logpath.exists():
            entry['log_sha256'] = digest(logpath)
        save()
    if check:
        result.check_returncode()
    return result


def logged_check_call(command, **kwargs):
    logged_run(command, check=True, **kwargs)
    return 0


if (OUT / 'report.json').exists() or SOURCE.exists() or CASES.exists():
    raise RuntimeError('Existing dataset preparation must be preserved; refusing overwrite')
(OUT / 'logs').mkdir(exist_ok=False)
report = {'schema_version': 1, 'started_at': now(), 'succeeded': False,
          'upstream_revision': REVISION, 'upstream_cache': str(CACHE),
          'driver_sha256': digest(__file__), 'commands': [],
          'scope': 'Official offline dataset preparation only; not a Blueberry verdict or benchmark.',
          'source_policy': 'Byte-identical tracked sources copied to an isolated directory; cache remains read-only.',
          'generator_mode': 'DEFAULT; no --dev, no hash rewriting; one subprocess at a time.'}
save()
try:
    actual_revision = git('rev-parse', 'HEAD').decode().strip()
    if actual_revision != REVISION:
        raise RuntimeError('Unexpected cached Library Checker revision: ' + actual_revision)
    names = git('ls-tree', '-r', '--name-only', '-z', REVISION, '--',
                'generate.py', 'geo/closest_pair', 'common', 'LICENSE', 'README.md').decode().split('\0')
    hashes = {}
    for name in filter(None, names):
        origin = CACHE / name
        if origin.is_symlink() or not origin.is_file():
            raise RuntimeError('Not a regular cached source: ' + name)
        pinned = git('show', REVISION + ':' + name)
        if origin.read_bytes() != pinned:
            raise RuntimeError('Cached source differs from pinned revision: ' + name)
        destination = SOURCE / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(pinned)
        hashes[name] = digest(destination)
    report['official_tracked_source_sha256'] = hashes
    expected = json.loads((PROBLEM / 'hash.json').read_text())
    if len(expected) != 58 or sum(name.endswith('.in') for name in expected) != 29:
        raise RuntimeError('Unexpected official testcase manifest size')
    report['official_hash_json_sha256'] = digest(PROBLEM / 'hash.json')
    compiler = shutil.which('g++')
    if not compiler:
        raise RuntimeError('g++ not found')
    os.environ['CXX'] = compiler
    os.environ['CXXFLAGS'] = '-O2 -std=c++17 -Wall -Wextra -Werror -Wno-unused-result'
    report['compiler'] = {'path': compiler, 'resolved_path': str(Path(compiler).resolve()),
                          'sha256': digest(Path(compiler).resolve()),
                          'version': subprocess.check_output([compiler, '--version'], text=True)}
    report['environment'] = {key: os.environ.get(key, '') for key in
                             ['CXX', 'CXXFLAGS', 'CPLUS_INCLUDE_PATH', 'CPATH', 'LD_LIBRARY_PATH']}
    report['python'] = {'executable': sys.executable, 'version': sys.version}
    save()
    logging.basicConfig(filename=OUT / 'generation.log', level=logging.DEBUG,
                        format='%(asctime)s %(levelname)s %(message)s')
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location('pinned_closest_pair_generator', SOURCE / 'generate.py')
    generator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(generator)
    generator.check_call = logged_check_call
    generator.run = logged_run
    problem = generator.Problem(SOURCE, PROBLEM)
    print('Pinned source copy ready; starting serial official generation.', flush=True)
    problem.generate(generator.Problem.Mode.DEFAULT)
    problem.compile_verifier()
    problem.verify_inputs()
    for name in sorted(n for n in expected if n.endswith('.in')):
        answer = PROBLEM / 'out' / (Path(name).stem + '.out')
        logged_check_call([PROBLEM / 'checker', PROBLEM / 'in' / name, answer, answer])
    actual = {}
    for suffix, directory in [('.in', 'in'), ('.out', 'out')]:
        for path in sorted((PROBLEM / directory).glob('*' + suffix)):
            if path.is_symlink() or not path.is_file():
                raise RuntimeError('Generated case is not a regular file: ' + str(path))
            actual[path.name] = digest(path)
    if actual != expected:
        report['generated_files_sha256'] = actual
        raise RuntimeError('Generated cases differ from pinned official hash.json')
    for name, wanted in hashes.items():
        if digest(SOURCE / name) != wanted or digest(CACHE / name) != wanted:
            raise RuntimeError('Pinned source changed during preparation: ' + name)
    CASES.mkdir()
    for name, wanted in sorted(actual.items()):
        source = PROBLEM / ('in' if name.endswith('.in') else 'out') / name
        shutil.copyfile(source, CASES / name)
        if digest(CASES / name) != wanted or (CASES / name).is_symlink():
            raise RuntimeError('Final case copy mismatch: ' + name)
    binaries = {}
    for path in sorted(PROBLEM.rglob('*')):
        if path.is_file() and os.access(path, os.X_OK):
            binaries[str(path.relative_to(OUT))] = digest(path)
    report.update(succeeded=True, case_count=29, generated_files_sha256=actual,
                  dataset_directory=str(CASES), checker_path=str(PROBLEM / 'checker'),
                  checker_sha256=digest(PROBLEM / 'checker'), binary_sha256=binaries,
                  generated_params_sha256=digest(PROBLEM / 'params.h'),
                  source_files_unchanged=True, official_hash_json_unchanged=True,
                  input_verifier_cases_passed=29, checker_self_cases_passed=29)
    print('PASS: 29 official cases, all 58 hashes match; verifier/checker self-checks passed.', flush=True)
except BaseException as exc:
    report['failure'] = repr(exc)
    raise
finally:
    report['finished_at'] = now()
    if (OUT / 'generation.log').exists():
        report['generation_log_sha256'] = digest(OUT / 'generation.log')
    save()
