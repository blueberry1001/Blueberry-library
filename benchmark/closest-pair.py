#!/usr/bin/env python3
"""Bounded immutable closest-pair comparison; source /tmp/blueberry-setup/env.sh first."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import shutil
import statistics
import subprocess
import time
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'benchmark/closest-pair.cpp'
RUNNER = Path(__file__).resolve()
BASE = ROOT / '.verification/closest-pair/benchmark'
HEADERS = ['blueberry/geometry/closest-pair.hpp', 'blueberry/utility/fast-io.hpp']
VARIANTS = ['blueberry', 'vectors', 'sweep']
CELLS = [('random', 20000), ('random', 100000), ('duplicates', 100000),
         ('grid', 100000), ('nearline', 100000), ('extreme', 100000)]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save(path, data):
    Path(path).write_text(json.dumps(data, indent=2) + '\n')


def now():
    return datetime.now(timezone.utc).isoformat()


def command(argv, input_path=None, timeout=180):
    start = time.perf_counter_ns()
    try:
        with open(input_path if input_path else os.devnull, 'rb') as stream:
            r = subprocess.run([str(x) for x in argv], stdin=stream, capture_output=True,
                               text=True, timeout=timeout)
        return dict(command=[str(x) for x in argv], returncode=r.returncode,
                    stdout=r.stdout, stderr=r.stderr, process_ns=time.perf_counter_ns() - start)
    except subprocess.TimeoutExpired as e:
        return dict(command=[str(x) for x in argv], returncode=None, timeout=timeout,
                    stdout=(e.stdout or b'').decode(), stderr=(e.stderr or b'').decode(),
                    process_ns=time.perf_counter_ns() - start)


def execute(argv, out, *, input_path=None, io=False):
    record = command(argv, input_path)
    try:
        if record['returncode'] != 0:
            raise RuntimeError('nonzero exit or timeout')
        data = json.loads(record['stderr'] if io else record['stdout'])
        required = ['parse_ns', 'solve_ns', 'format_ns', 'end_to_end_ns', 'cases'] if io else [
            'input_digest', 'n', 'pair', 'full_ns', 'prepare_ns', 'search_ns', 'rss_kib']
        if not isinstance(data, dict) or not all(k in data for k in required):
            raise RuntimeError('missing JSON fields')
        if any(v is not None and (not isinstance(v, int) or v < 0)
               for k, v in data.items() if k.endswith('_ns')):
            raise RuntimeError('invalid duration')
        if not io and (not isinstance(data['pair'], list) or len(data['pair']) != 2):
            raise RuntimeError('invalid pair')
        if io:
            data['output_sha256'] = hashlib.sha256(record['stdout'].encode()).hexdigest()
        return record, data
    except Exception as e:
        record['failure'] = repr(e)
        save(out / ('failure-' + str(time.time_ns()) + '.json'), record)
        raise


def environment():
    return dict(time=now(), platform=platform.platform(),
                cpu=Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
                cpu_quota=Path('/sys/fs/cgroup/cpu.max').read_text().strip(),
                affinity=sorted(os.sched_getaffinity(0)),
                process_snapshot=command(['ps', '-eo', 'pid,pcpu,comm'])['stdout'],
                caveat='Shared host, unpinned; coordinated quiet window, ranges are sample ranges, not confidence intervals.')


def prepare(args):
    out = BASE / args.experiment
    if out.exists():
        raise RuntimeError('Refusing to overwrite experiment')
    out.mkdir(parents=True)
    snap = out / 'snapshot'
    snap.mkdir()
    hashes = {str(SOURCE.relative_to(ROOT)): sha(SOURCE), **{p: sha(ROOT / p) for p in HEADERS}}
    if hashes[HEADERS[0]] != args.production_sha256:
        raise RuntimeError('Production hash differs from requested freeze')
    for name, expected in hashes.items():
        target = snap / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / name, target)
        assert sha(target) == expected
    shutil.copy2(RUNNER, out / 'runner.py')
    meta = dict(created_at=now(), git_head=command(['git', '-C', ROOT, 'rev-parse', 'HEAD'])['stdout'].strip(),
                source_sha256=hashes, runner_sha256=sha(RUNNER), cells=CELLS, measured_repeats=3,
                warmup_repeats=1, entries=[], inputs={}, environment=environment())
    for compiler in ['g++', 'clang++']:
        for mode in ['release', 'assert']:
            for alloc in ([False, True] if compiler == 'g++' else [False]):
                name = compiler.replace('+', 'p') + '-' + mode + ('-allocation' if alloc else '')
                binary = out / name
                flags = ['-std=gnu++20', '-O2', '-Wall', '-Wextra', '-I', str(snap)]
                if mode == 'release': flags.append('-DNDEBUG')
                if alloc: flags += ['-DCP_ALLOCATIONS', '-Wno-mismatched-new-delete']
                rec = command([compiler, *flags, snap / SOURCE.relative_to(ROOT), '-o', binary])
                entry = dict(compiler=compiler, mode=mode, allocation=alloc, binary=str(binary.relative_to(ROOT)),
                             compiler_version=command([compiler, '--version'])['stdout'], flags=flags, compile=rec)
                meta['entries'].append(entry)
                save(out / 'compile.json', meta['entries'])
                if rec['returncode'] != 0 or rec['stderr']:
                    raise RuntimeError('Failed or warning-producing compilation; evidence retained')
                entry['binary_sha256'] = sha(binary)
                entry['oracle_check'] = command([binary, 'check'])
                if mode == 'assert':
                    entry['negative_checks'] = [command([binary, 'invalid-singleton', v]) for v in VARIANTS]
                save(out / 'compile.json', meta['entries'])
                if entry['oracle_check']['returncode'] != 0 or any(
                        r['returncode'] != -6 for r in entry.get('negative_checks', [])):
                    raise RuntimeError('Correctness/negative checks failed')
    checks = []
    identities = {}
    for e in meta['entries']:
        for kind, _ in CELLS:
            for variant in VARIANTS:
                rec, data = execute([ROOT / e['binary'], 'sample', variant, kind, 127], out)
                row = dict(compiler=e['compiler'], mode=e['mode'], allocation=e['allocation'],
                           variant=variant, workload=kind, data=data)
                checks.append(row)
                save(out / 'small-checks.json', checks)
                identity = [data['input_digest'], data['pair'], data['n']]
                if identity != identities.setdefault(kind, identity):
                    save(out / 'small-check-failure.json', dict(rows=checks, rejected=rec))
                    raise RuntimeError('Small identity mismatch')
    binary = ROOT / meta['entries'][0]['binary']
    for kind, size in [('random', 100000), ('tiny', 10000)]:
        path = out / (kind + '.in')
        rec = command([binary, 'generate', kind, size])
        if rec['returncode'] != 0:
            save(out / 'generation-failure.json', rec)
            raise RuntimeError('I/O input generation failed')
        path.write_text(rec['stdout'])
        meta['inputs'][kind] = dict(path=str(path.relative_to(ROOT)), sha256=sha(path), bytes=path.stat().st_size)
    for name, expected in hashes.items():
        if sha(ROOT / name) != expected or sha(snap / name) != expected:
            raise RuntimeError('Live/snapshot source changed during preparation')
    if sha(RUNNER) != meta['runner_sha256']:
        raise RuntimeError('Runner changed during preparation')
    meta['small_checks_sha256'] = sha(out / 'small-checks.json')
    save(out / 'prepared.json', meta)
    print(f'Prepared {len(meta["entries"])} binaries, {len(checks)} small profiles; timings held.')


def checked(args):
    out = BASE / args.experiment
    meta = json.loads((out / 'prepared.json').read_text())
    for name, expected in meta['source_sha256'].items():
        assert sha(ROOT / name) == expected, f'Live source changed: {name}'
        assert sha(out / 'snapshot' / name) == expected, f'Snapshot changed: {name}'
    assert sha(RUNNER) == meta['runner_sha256'] == sha(out / 'runner.py')
    assert sha(out / 'small-checks.json') == meta['small_checks_sha256']
    for e in meta['entries']: assert sha(ROOT / e['binary']) == e['binary_sha256']
    for item in meta['inputs'].values(): assert sha(ROOT / item['path']) == item['sha256']
    return out, meta


def measure(args):
    out, meta = checked(args)
    if (out / 'raw.jsonl').exists(): raise RuntimeError('Refusing to overwrite raw attempts')
    save(out / 'timing-environment.json', environment())
    identities = {}
    started = time.monotonic()
    with (out / 'raw.jsonl').open('x') as stream:
        for e in [x for x in meta['entries'] if not x['allocation']]:
            for rep in range(4):
                order = VARIANTS[rep % 3:] + VARIANTS[:rep % 3]
                for kind, n in CELLS:
                    for variant in order:
                        rec, data = execute([ROOT / e['binary'], 'sample', variant, kind, n], out)
                        row = dict(family='kernel', compiler=e['compiler'], mode=e['mode'], rep=rep,
                                   warmup=rep == 0, variant=variant, workload=kind, n=n, data=data)
                        stream.write(json.dumps(row) + '\n'); stream.flush()
                        key = (kind, n)
                        identity = [data['input_digest'], data['pair']]
                        if identity != identities.setdefault(key, identity):
                            save(out / 'identity-failure.json', dict(rejected=rec, row=row))
                            raise RuntimeError('Large exact-pair identity mismatch')
                for kind, item in meta['inputs'].items():
                    for method in (['fast', 'iostream'] if rep % 2 == 0 else ['iostream', 'fast']):
                        rec, data = execute([ROOT / e['binary'], 'io', method], out,
                                            input_path=ROOT / item['path'], io=True)
                        row = dict(family='io', compiler=e['compiler'], mode=e['mode'], rep=rep,
                                   warmup=rep == 0, method=method, workload=kind, data=data)
                        stream.write(json.dumps(row) + '\n'); stream.flush()
                        key = ('io', kind)
                        if data['output_sha256'] != identities.setdefault(key, data['output_sha256']):
                            save(out / 'io-identity-failure.json', dict(rejected=rec, row=row))
                            raise RuntimeError('I/O result mismatch')
    save(out / 'timing-completed.json', dict(time=now(), elapsed_seconds=time.monotonic() - started,
         raw_sha256=sha(out / 'raw.jsonl'), kernel_profiles=288, io_profiles=64,
         kernel_warmups=72, io_warmups=16, identities={str(k): v for k, v in identities.items()}))
    print('Timing finished: 288 kernel +64 I/O profiles, including88 warmups.')


def diagnostics(args):
    out, meta = checked(args)
    if (out / 'diagnostics.jsonl').exists(): raise RuntimeError('Refusing to overwrite diagnostics')
    measured = [json.loads(x) for x in (out / 'raw.jsonl').read_text().splitlines()]
    identities = {(r['workload'], r['n']): [r['data']['input_digest'], r['data']['pair']]
                  for r in measured if r['family'] == 'kernel'}
    with (out / 'diagnostics.jsonl').open('x') as stream:
        for e in [x for x in meta['entries'] if x['compiler'] == 'g++']:
            for kind in ['random', 'duplicates', 'grid']:
                for variant in VARIANTS:
                    # Tiny shell parent avoids inherited Python high-water RSS being mistaken for the algorithm.
                    argv = ['/bin/sh', '-c', '"$@"; result=$?; exit "$result"', 'closest-pair',
                            str(ROOT / e['binary']), 'diagnostic', variant, kind, '100000']
                    rec, data = execute(argv, out)
                    row = dict(compiler=e['compiler'], mode=e['mode'], allocation=e['allocation'],
                               workload=kind, n=100000, variant=variant, data=data)
                    stream.write(json.dumps(row) + '\n'); stream.flush()
                    if [data['input_digest'], data['pair']] != identities[(kind, 100000)] or (
                            e['allocation'] and not all(k in data for k in ['allocation_calls', 'allocation_bytes'])):
                        save(out / 'diagnostic-identity-failure.json', dict(rejected=rec, row=row))
                        raise RuntimeError('Diagnostic identity/schema mismatch')
    print('Separate diagnostics finished:18 allocation profiles +18 uninstrumented RSS profiles.')


def summarize(args):
    out, _ = checked(args)
    rows = [json.loads(x) for x in (out / 'raw.jsonl').read_text().splitlines()]
    groups = {}
    for row in rows:
        if row['warmup']: continue
        key = (row['family'], row['compiler'], row['mode'], row['workload'], row.get('n'), row.get('variant', row.get('method')))
        groups.setdefault(key, []).append(row['data'])
    summary = []
    for key, data in groups.items():
        metrics = {}
        for metric in data[0]:
            if not metric.endswith('_ns'): continue
            values = [x[metric] for x in data]
            metrics[metric] = None if values[0] is None else dict(median=statistics.median(values), min=min(values), max=max(values))
        summary.append(dict(family=key[0], compiler=key[1], mode=key[2], workload=key[3], n=key[4], variant=key[5], samples=len(data), metrics=metrics))
    save(out / 'summary.json', summary)
    print(f'Summarized {len(summary)} groups without pooling compilers, modes, workloads or stages.')


if __name__ == '__main__':
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['prepare', 'measure', 'diagnostics', 'summarize'])
    parser.add_argument('--experiment', default='initial')
    parser.add_argument('--production-sha256')
    options = parser.parse_args()
    globals()[options.action](options)
