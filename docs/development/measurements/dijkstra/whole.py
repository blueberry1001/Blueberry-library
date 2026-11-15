import datetime
import hashlib
import json
import os
import platform
import random
import subprocess
import sys
import time
from pathlib import Path

root = Path.cwd()
build = root / '.build/dijkstra-candidates'
output = build / 'whole.json'
variants = {'base': build / 'base', 'radix': build / 'radix-hybrid'}
reference = root / '.build/dijkstra-fastest/submission-415573.cpp'
shapes = ['sparse', 'zero', 'line', 'disconnected']
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
results = {'started': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'platform': platform.platform(),
           'cpu_affinity': [cpu],
           'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
           'input_seed': 20261115, 'rounds': 5, 'builds': [], 'inputs': {}, 'samples': [], 'failures': []}
def save(): output.write_text(json.dumps(results, indent=2) + '\n')
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def make_input(shape):
    path = Path('/tmp/blueberry-dijkstra-lc-' + shape + '.in')
    n, m = (500000, 499999) if shape == 'line' else (200000, 500000)
    rng = random.Random(20261115)
    with path.open('w') as f:
        f.write(f'{n} {m} 0 {n-1}\n')
        if shape == 'line':
            for i in range(1, n): f.write(f'{i-1} {i} 1\n')
        else:
            seen = set()
            if shape != 'disconnected':
                for i in range(1, n):
                    f.write(f'{i-1} {i} {0 if shape == "zero" else 1}\n')
                    seen.add((i - 1) * n + i)
            while len(seen) < m:
                lo, hi = (0, 1000) if shape == 'disconnected' and len(seen) < m // 2 else (1000, n) if shape == 'disconnected' else (0, n)
                u, v = rng.randrange(lo, hi), rng.randrange(lo, hi)
                key = u * n + v
                if u == v or key in seen: continue
                seen.add(key)
                f.write(f'{u} {v} {0 if shape == "zero" else rng.randrange(1000000001)}\n')
    return path
if len(sys.argv) == 3 and sys.argv[1] == '--generate':
    make_input(sys.argv[2]); sys.exit(0)
inputs = {}
for shape in shapes:
    subprocess.run([sys.executable, __file__, '--generate', shape], check=True)
    path = Path('/tmp/blueberry-dijkstra-lc-' + shape + '.in')
    n, m = (500000, 499999) if shape == 'line' else (200000, 500000)
    inputs[shape] = path
    results['inputs'][shape] = {'n': n, 'm': m, 'sha256': sha(path)}
    save()
for compiler in ['g++', 'clang++']:
    for variant in ['base', 'radix', 'fastest']:
        exe = f'/tmp/blueberry-lc-{compiler.replace("+", "p")}-{variant}'
        source = reference if variant == 'fastest' else root / 'verify/graph/shortest-path.test.cpp'
        flags = ['-std=gnu++23', '-O3', '-march=native', '-DNDEBUG']
        command = [compiler, *flags]
        if variant != 'fastest': command += ['-I' + str(variants[variant]), '-I' + str(root)]
        command += [str(source), '-o', exe]
        p = subprocess.run(command, capture_output=True, text=True)
        results['builds'].append({'compiler': compiler, 'variant': variant, 'command': command,
                                  'compiler_version': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0],
                                  'source_sha256': sha(source), 'returncode': p.returncode, 'stderr': p.stderr,
                                  'binary_sha256': sha(exe) if p.returncode == 0 else None})
        save()
        if p.returncode: sys.exit(1)
    for shape in shapes:
        for repeat in range(5):
            order = ['base', 'radix', 'fastest']
            order = order[repeat % 3:] + order[:repeat % 3]
            for variant in order:
                exe = f'/tmp/blueberry-lc-{compiler.replace("+", "p")}-{variant}'
                answer = Path(f'/tmp/blueberry-lc-{compiler.replace("+", "p")}-{shape}-{variant}.out')
                memory = Path(f'/tmp/blueberry-lc-{os.getpid()}.rss')
                begin = time.perf_counter()
                with inputs[shape].open('rb') as inp, answer.open('wb') as out:
                    p = subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(memory), exe], stdin=inp, stdout=out, stderr=subprocess.PIPE)
                wall_ms = (time.perf_counter() - begin) * 1000
                if p.returncode:
                    results['failures'].append({'case': [compiler, variant, shape, repeat], 'returncode': p.returncode,
                                                'stderr': p.stderr.decode(errors='replace')}); save(); sys.exit(1)
                results['samples'].append({'compiler': compiler, 'variant': variant, 'shape': shape, 'run': repeat,
                                           'wall_ms': wall_ms, 'max_rss_kib': int(memory.read_text().strip()), 'output_sha256': sha(answer)})
                save()
                print(compiler, variant, shape, repeat, round(wall_ms, 3), flush=True)
for shape in shapes:
    costs = {}
    with inputs[shape].open() as f:
        n, m, start, target = map(int, f.readline().split())
        for line in f:
            u, v, w = map(int, line.split())
            key = u * n + v
            if key not in costs or w < costs[key]: costs[key] = w
    expected = None
    for compiler in ['g++', 'clang++']:
        for variant in ['base', 'radix', 'fastest']:
            path = Path(f'/tmp/blueberry-lc-{compiler.replace("+", "p")}-{shape}-{variant}.out')
            with path.open() as f:
                first = list(map(int, f.readline().split()))
                if first == [-1]: answer = None
                else:
                    answer, count = first
                    v, total, visited = start, 0, {start}
                    for i in range(count):
                        u, nxt = map(int, f.readline().split())
                        if u != v or u * n + nxt not in costs or nxt in visited: raise ValueError((shape, variant, i))
                        total += costs[u * n + nxt]
                        visited.add(nxt)
                        v = nxt
                    if v != target or total != answer or f.read().strip(): raise ValueError((shape, variant, total, answer))
                key = str(answer)
                if expected is None: expected = key
                if key != expected: raise ValueError((shape, variant, key, expected))
    results['inputs'][shape]['answer'] = expected
    if shape == 'line' and expected != str(n - 1): raise ValueError((shape, expected))
    if shape == 'zero' and expected != '0': raise ValueError((shape, expected))
    if shape == 'disconnected' and expected != 'None': raise ValueError((shape, expected))
    save()
results['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
save()
