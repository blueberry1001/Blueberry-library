import hashlib
import json
import os
import platform
import subprocess
import sys
import time
from pathlib import Path

root = Path.cwd()
build = root / '.build/dijkstra-candidates'
whole = json.loads((build / 'whole.json').read_text())
output = build / (sys.argv[2] if len(sys.argv) > 2 else 'cross-final.json')
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
shapes = ['sparse', 'zero', 'line', 'disconnected']
variants = sys.argv[1].split(',') if len(sys.argv) > 1 else ['base-fast', 'radix-fast', 'target-fast', 'targetcsr-fast', 'fastest']
result = {'platform': platform.platform(), 'cpu_affinity': [cpu], 'revision': whole['revision'], 'rounds': 5,
          'inputs': whole['inputs'], 'builds': [], 'samples': [], 'failures': []}
def save(): output.write_text(json.dumps(result, indent=2) + '\n')
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
for shape in shapes:
    path = Path('/tmp/blueberry-dijkstra-lc-' + shape + '.in')
    if not path.exists(): subprocess.run([sys.executable, str(build / 'whole.py'), '--generate', shape], check=True)
    if sha(path) != whole['inputs'][shape]['sha256']: raise ValueError(('input changed', shape))
for compiler in ['g++', 'clang++']:
    for variant in variants:
        exe = f'/tmp/blueberry-cross-{compiler.replace("+", "p")}-{variant}'
        if variant == 'fastest':
            source = root / '.build/dijkstra-fastest/submission-415573.cpp'
            args = []
        else:
            name, io = variant.split('-')
            directory = {'base': 'base', 'radix': 'published', 'target': 'published', 'targetcsr': 'target-csr'}[name]
            source = build / 'io-final.cpp'
            args = ['-I' + str(build / directory), '-I' + str(root)]
            if io == 'fast': args += ['-DFAST_IO']
            if name in ('target', 'targetcsr'): args += ['-DTARGET_ONLY']
            if name == 'targetcsr': args += ['-DTARGET_PROTOTYPE']
        cmd = [compiler, '-std=gnu++23', '-O3', '-march=native', '-DNDEBUG', *args, str(source), '-o', exe]
        p = subprocess.run(cmd, capture_output=True, text=True)
        result['builds'].append({'compiler': compiler, 'variant': variant, 'command': cmd,
                                 'compiler_version': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0],
                                 'source_sha256': sha(source), 'returncode': p.returncode, 'stderr': p.stderr,
                                 'binary_sha256': sha(exe) if p.returncode == 0 else None})
        save()
        if p.returncode: sys.exit(1)
    for shape in shapes:
        path = Path('/tmp/blueberry-dijkstra-lc-' + shape + '.in')
        for repeat in range(5):
            order = variants[repeat % len(variants):] + variants[:repeat % len(variants)]
            for variant in order:
                exe = f'/tmp/blueberry-cross-{compiler.replace("+", "p")}-{variant}'
                answer = Path(f'/tmp/blueberry-cross-{compiler.replace("+", "p")}-{shape}-{variant}.out')
                memory = Path(f'/tmp/blueberry-cross-{os.getpid()}.rss')
                begin = time.perf_counter()
                with path.open('rb') as inp, answer.open('wb') as out:
                    p = subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(memory), exe], stdin=inp, stdout=out, stderr=subprocess.PIPE)
                wall_ms = (time.perf_counter() - begin) * 1000
                if p.returncode:
                    result['failures'].append({'case': [compiler, variant, shape, repeat], 'returncode': p.returncode,
                                               'stderr': p.stderr.decode(errors='replace')}); save(); sys.exit(1)
                result['samples'].append({'compiler': compiler, 'variant': variant, 'shape': shape, 'run': repeat,
                                          'wall_ms': wall_ms, 'max_rss_kib': int(memory.read_text().strip()), 'output_sha256': sha(answer)})
                save()
                print(compiler, variant, shape, repeat, round(wall_ms, 3), flush=True)
for shape in shapes:
    costs = {}
    with Path('/tmp/blueberry-dijkstra-lc-' + shape + '.in').open() as f:
        n, m, start, target = map(int, f.readline().split())
        for line in f:
            u, v, w = map(int, line.split())
            costs[u * n + v] = w
    for compiler in ['g++', 'clang++']:
        for variant in variants:
            path = Path(f'/tmp/blueberry-cross-{compiler.replace("+", "p")}-{shape}-{variant}.out')
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
                        visited.add(nxt); v = nxt
                    if v != target or total != answer or f.read().strip(): raise ValueError((shape, variant, total, answer))
                if str(answer) != whole['inputs'][shape]['answer']: raise ValueError((shape, variant, answer))
    save()
result['verified'] = True
save()
