#!/usr/bin/env python3
"""Prepare identical LC-shaped inputs and compile local/public implementations.

Run from the repository root after fetch_lc_submissions.py has saved the four
problem sources and metadata under .build/operation-structures/{fastest-sources,fastest}.
Foreign sources stay in the ignored research directory.
"""
import hashlib
import json
import pathlib
import random
import subprocess

BASE = pathlib.Path('.build/operation-structures')
OUTPUT = pathlib.Path('benchmark/results/operation-structures')
OUTPUT.mkdir(parents=True, exist_ok=True)
FLAGS = ['-std=gnu++23', '-O2', '-DNDEBUG', '-march=native', '-I.', '-isystem', '.deps/ac-library']
PROBLEMS = {
    'range_chmin_chmax_add_range_sum': 'segment-tree-beats',
    'set_xor_min': 'binary-trie',
    'queue_operate_all_composite': 'aggregate-queue',
    'deque_operate_all_composite': 'aggregate-deque',
}
rows = []
for problem, driver in PROBLEMS.items():
    sources = [('blueberry', pathlib.Path('verify/data-structure/' + driver + '.test.cpp'))]
    sources += [(path.stem.rsplit('-', 1)[1], path)
                for path in sorted((BASE / 'fastest-sources').glob(problem + '-*.cpp'))]
    for variant, source in sources:
        binary = BASE / (problem + '-' + variant)
        result = subprocess.run(['g++', *FLAGS, str(source), '-o', str(binary)], capture_output=True, text=True)
        (BASE / (problem + '-' + variant + '-compile.log')).write_text(result.stdout + result.stderr)
        rows.append(dict(problem=problem, variant=variant, binary=str(binary), source=str(source),
                         source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                         compile_returncode=result.returncode))
        print(problem, variant, result.returncode, flush=True)
    rng = random.Random(20260917)
    q = 100000
    modulus = 998244353
    if driver == 'segment-tree-beats':
        n = 100000
        lines = [f'{n} {q}', ' '.join(str(rng.randrange(-10**8, 10**8)) for _ in range(n))]
        for i in range(q):
            left, right = sorted([rng.randrange(n + 1), rng.randrange(n + 1)])
            if left == right:
                if right < n:
                    right += 1
                else:
                    left -= 1
            kind = i % 4
            line = f'{kind} {left} {right}'
            if kind != 3:
                bound = 10**6 if kind == 2 else 10**8
                line += f' {rng.randrange(-bound, bound)}'
            lines.append(line)
    elif driver == 'binary-trie':
        lines = [str(q)]
        for i in range(q):
            # Keep 0 present, so every min-XOR query is legal.
            kind = 0 if i < 10000 else i % 3
            key = 0 if i == 0 else rng.randrange(1, 1 << 30)
            lines.append(f'{kind} {key}')
    else:
        lines = [str(q)]
        size = 0
        for i in range(q):
            if driver == 'aggregate-queue':
                kind = 0 if i < 10000 or size == 0 else rng.randrange(3)
                if kind == 0:
                    lines.append(f'0 {rng.randrange(1, modulus)} {rng.randrange(modulus)}'); size += 1
                elif kind == 1:
                    lines.append('1'); size -= 1
                else:
                    lines.append(f'2 {rng.randrange(modulus)}')
            else:
                kind = rng.randrange(2) if i < 10000 or size == 0 else rng.randrange(5)
                if kind < 2:
                    lines.append(f'{kind} {rng.randrange(1, modulus)} {rng.randrange(modulus)}'); size += 1
                elif kind < 4:
                    lines.append(str(kind)); size -= 1
                else:
                    lines.append(f'4 {rng.randrange(modulus)}')
    (BASE / (problem + '.in')).write_text('\n'.join(lines) + '\n')

metadata = json.loads((BASE / 'fastest/submissions.json').read_text())
(OUTPUT / 'fastest-metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
(BASE / 'prepared.json').write_text(json.dumps(dict(flags=FLAGS, rows=rows, seed=20260917, q=100000,
    header_sha256={driver: hashlib.sha256(pathlib.Path('blueberry/data-structure/' + driver + '.hpp').read_bytes()).hexdigest()
                   for driver in PROBLEMS.values()}), indent=2) + '\n')
