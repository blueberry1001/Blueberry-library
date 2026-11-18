#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shlex
import shutil
import statistics
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
HEADER = 'blueberry/math/multipoint-evaluation.hpp'
FPS = 'blueberry/math/formal-power-series.hpp'
MARKER = '  polynomial_detail::check_size<Mint>(f.size(), x.size());\n'
INSERT = '''  if (f.size() == 1) return std::vector<Mint>(x.size(), f[0]);
  if (f.size() <= 32) {
    std::vector<Mint> result(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) {
      Mint value = 0;
      for (auto it = f.rbegin(); it != f.rend(); ++it) value = value * x[i] + *it;
      result[i] = value;
    }
    return result;
  }
'''
CASES = [(0, 0, 0), (0, 32, 0), (1, 1, 0), (8, 1, 0), (32, 1, 0), (33, 1, 0),
         (1, 100000, 0), (2, 100000, 0), (8, 100000, 0), (16, 100000, 0),
         (31, 100000, 0), (32, 100000, 0), (33, 100000, 0), (64, 100000, 0),
         (8, 100000, 1), (32, 100000, 2), (128, 1024, 0), (512, 512, 0), (2048, 2048, 0),
         (8192, 1, 0)]
CASES = [(1, 1, 0), (8, 1, 0), (32, 1, 0), (33, 1, 0), (64, 1, 0), (128, 1, 0),
         (1024, 1, 0), (8192, 1, 0), (100000, 1, 0), (33, 2, 0), (8192, 2, 0), (2048, 2048, 0)]

def digest(data):
    return hashlib.sha256(data).hexdigest()

def tree_digest(root):
    h = hashlib.sha256()
    for p in sorted(x for x in root.rglob('*') if x.is_file()):
        h.update(p.relative_to(root).as_posix().encode() + b'\0')
        h.update(bytes.fromhex(digest(p.read_bytes())))
    return h.hexdigest()

def run(cmd, **kwargs):
    return subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)

def stats(samples):
    median = statistics.median(samples)
    return {'median': median, 'min': min(samples), 'max': max(samples),
            'mad': statistics.median(abs(x - median) for x in samples),
            'stdev': statistics.stdev(samples) if len(samples) > 1 else None}

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--repo', type=Path, required=True)
    p.add_argument('--revision', required=True)
    p.add_argument('--acl', type=Path, required=True, help='ACL root containing atcoder/')
    p.add_argument('--cxx', default='g++')
    p.add_argument('--flags', default='-std=gnu++20 -O2 -DNDEBUG')
    p.add_argument('--repeats', type=int, default=7)
    p.add_argument('--warmups', type=int, default=1)
    p.add_argument('--seed', type=int, default=1729)
    p.add_argument('--cpu', type=int)
    p.add_argument('--case', action='append', type=int, help='zero-based case index; repeatable')
    a = p.parse_args()
    if a.repeats < 2 or a.warmups < 0 or not 0 <= a.seed < 2**32:
        p.error('invalid seed or repetition count')
    indices = a.case if a.case is not None else list(range(len(CASES)))
    if len(set(indices)) != len(indices) or any(i < 0 or i >= len(CASES) for i in indices):
        p.error('invalid case index')
    if a.cpu is not None and (not hasattr(os, 'sched_getaffinity') or a.cpu not in os.sched_getaffinity(0)):
        p.error('requested CPU is unavailable')
    acl = (a.acl / 'atcoder').resolve()
    if not acl.is_dir(): p.error('ACL root must contain atcoder/')
    repo = a.repo.resolve()
    commit = run(['git', '-C', str(repo), 'rev-parse', a.revision + '^{commit}']).stdout.decode().strip()
    base = run(['git', '-C', str(repo), 'show', commit + ':' + HEADER]).stdout
    fps = run(['git', '-C', str(repo), 'show', commit + ':' + FPS]).stdout
    marker = MARKER.encode()
    if base.count(marker) != 1: raise RuntimeError('header no longer matches candidate insertion point')
    base = base.replace(marker, marker + INSERT.encode())
    candidate = base.replace(b'  if (f.size() <= 32) {\n', b'  if (f.size() <= 32 || x.size() == 1) {\n')
    if base == candidate: raise RuntimeError('single-point insertion failed')
    study = Path(tempfile.mkdtemp(prefix='run-', dir=HERE))
    result_path = study / 'results.json'
    benchmark_source = (HERE / 'benchmark.cpp').read_bytes()
    oracle_source = (HERE / 'oracle.cpp').read_bytes()
    (study / 'benchmark.cpp').write_bytes(benchmark_source)
    (study / 'oracle.cpp').write_bytes(oracle_source)
    report = {'status': 'running', 'revision': commit, 'repo': str(repo), 'acl_source': str(acl),
              'input_hashes': {'base_header': digest(base), 'candidate_header': digest(candidate), 'fps': digest(fps),
                               'benchmark': digest(benchmark_source),
                               'oracle': digest(oracle_source), 'runner': digest(Path(__file__).read_bytes())},
              'environment': {'platform': platform.platform(), 'python': platform.python_version(), 'compiler': a.cxx,
                              'flags': shlex.split(a.flags), 'cpu': a.cpu},
              'seed': a.seed, 'warmups': a.warmups, 'repeats': a.repeats, 'cases': [CASES[i] for i in indices],
              'scope': 'header call including allocation; excludes input generation, validation and program start; RSS is process peak including input and oracle, KiB on Linux, -1 if unavailable',
              'compile': [], 'oracle': [], 'runs': [], 'summary': []}
    def save():
        result_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    save()
    try:
        shutil.copytree(acl, study / 'acl' / 'atcoder')
        report['input_hashes']['acl_snapshot'] = tree_digest(study / 'acl' / 'atcoder')
        report['input_hashes']['acl_source_after_copy'] = tree_digest(acl)
        if report['input_hashes']['acl_snapshot'] != report['input_hashes']['acl_source_after_copy']:
            raise RuntimeError('ACL changed while copied')
        report['environment']['compiler_version'] = run([a.cxx, '--version']).stdout.decode(errors='replace').splitlines()[0]
        binaries = {}
        for variant, content in [('baseline', base), ('candidate', candidate)]:
            root = study / variant
            (root / HEADER).parent.mkdir(parents=True)
            (root / HEADER).write_bytes(content)
            (root / FPS).write_bytes(fps)
            if digest((root / FPS).read_bytes()) != report['input_hashes']['fps']: raise RuntimeError('FPS mismatch')
            for kind in ('benchmark', 'oracle'):
                exe = root / kind
                cmd = [a.cxx, *shlex.split(a.flags), '-I', str(root), '-isystem', str(study / 'acl'),
                       str(study / (kind + '.cpp')), '-o', str(exe)]
                if kind == 'oracle': cmd.insert(1 + len(shlex.split(a.flags)), '-UNDEBUG')
                proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                report['compile'].append({'variant': variant, 'kind': kind, 'command': cmd, 'returncode': proc.returncode,
                                          'stdout': proc.stdout.decode(errors='replace'), 'stderr': proc.stderr.decode(errors='replace')})
                save()
                if proc.returncode: raise RuntimeError('compile failed')
                report['compile'][-1]['executable_sha256'] = digest(exe.read_bytes())
                binaries[variant, kind] = exe
        def pin():
            if a.cpu is not None: os.sched_setaffinity(0, {a.cpu})
        for variant in ('baseline', 'candidate'):
            for seed in (a.seed, (a.seed + 1) % 2**32, (a.seed + 97) % 2**32):
                proc = subprocess.run([str(binaries[variant, 'oracle']), str(seed)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=pin)
                report['oracle'].append({'variant': variant, 'seed': seed, 'returncode': proc.returncode,
                                         'stdout': proc.stdout.decode(errors='replace'), 'stderr': proc.stderr.decode(errors='replace')})
                save()
                if proc.returncode: raise RuntimeError('oracle failed')
        for ci in indices:
            n, m, mode = CASES[ci]
            expected = None
            rows = []
            for trial in range(a.warmups + a.repeats):
                variants = ('baseline', 'candidate') if (trial + ci) % 2 == 0 else ('candidate', 'baseline')
                for variant in variants:
                    cmd = [str(binaries[variant, 'benchmark']), str(n), str(m), str(a.seed), str(mode)]
                    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, preexec_fn=pin)
                    row = {'case': ci, 'n': n, 'm': m, 'mode': mode, 'trial': trial, 'warmup': trial < a.warmups,
                           'variant': variant, 'returncode': proc.returncode, 'stderr': proc.stderr.decode(errors='replace')}
                    report['runs'].append(row)
                    save()
                    if proc.returncode: raise RuntimeError('benchmark or oracle failed')
                    fields = proc.stdout.decode().split()
                    if len(fields) != 3: raise RuntimeError('invalid benchmark output')
                    row['ns'], row['checksum'], row['peak_rss'] = map(int, fields)
                    if expected is None: expected = row['checksum']
                    if expected != row['checksum']: raise RuntimeError('baseline and candidate differ')
                    rows.append(row)
                    save()
            row = {'case': ci, 'n': n, 'm': m, 'mode': mode}
            for variant in ('baseline', 'candidate'):
                samples = [r for r in rows if r['variant'] == variant and not r['warmup']]
                rss = [r['peak_rss'] for r in samples if r['peak_rss'] >= 0]
                row[variant] = {'ns': stats([r['ns'] for r in samples]), 'peak_rss': stats(rss) if rss else None}
            denominator = row['baseline']['ns']['median']
            row['candidate_over_baseline'] = row['candidate']['ns']['median'] / denominator if denominator else None
            report['summary'].append(row)
            print(json.dumps(row), flush=True)
            save()
        report['status'] = 'complete'
    except BaseException as exc:
        report['status'] = 'failed'
        report['error'] = f'{type(exc).__name__}: {exc}'
        raise
    finally:
        save()
        print('results:', result_path, flush=True)

if __name__ == '__main__':
    main()
