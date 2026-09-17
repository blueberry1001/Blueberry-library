#!/usr/bin/env python3
"""Compile local Fastest sources and generate fixed inputs; run from repo root.

First use scripts/fetch_lc_submissions.py with the three problem IDs below,
--limit 3 --analyze-top 2 --save-source-dir .build/legacy-treap/fastest-sources
--output .build/legacy-treap/fastest.
"""
import hashlib
import json
import pathlib
import random
import subprocess

BASE = pathlib.Path('.build/legacy-treap')
OUTPUT = pathlib.Path('benchmark/results/legacy-treap')
OUTPUT.mkdir(parents=True, exist_ok=True)
metadata = json.loads((BASE / 'fastest/submissions.json').read_text())
(OUTPUT / 'fastest-metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
FLAGS = ['-std=gnu++23', '-O2', '-DNDEBUG', '-march=native', '-I.', '-isystem', '.deps/ac-library']
PROBLEMS = {
    'dynamic_sequence_range_affine_range_sum': 'implicit-treap-affine',
    'double_ended_priority_queue': 'ordered-multiset-priority-queue',
    'point_set_range_composite': 'ordered-multiset-composite',
}
rows = []
for problem, driver in PROBLEMS.items():
    sources = [('blueberry', pathlib.Path('verify/data-structure/' + driver + '.test.cpp'))]
    sources += [(p.stem.rsplit('-', 1)[1], p)
                for p in sorted((BASE / 'fastest-sources').glob(problem + '-*.cpp'))]
    for variant, source in sources:
        binary = BASE / (problem + '-' + variant)
        result = subprocess.run(['g++', *FLAGS, str(source), '-o', str(binary)],
                                capture_output=True, text=True)
        (BASE / (problem + '-' + variant + '-compile.log')).write_text(result.stdout + result.stderr)
        rows.append(dict(problem=problem, variant=variant, source=str(source),
                         sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                         binary=str(binary), compile_returncode=result.returncode))
        print(problem, variant, result.returncode, flush=True)

    rng = random.Random(20260917)
    n = q = size = 50000
    modulus = 998244353
    lines = [f'{n} {q}']
    if problem == 'point_set_range_composite':
        lines += [f'{rng.randrange(1, modulus)} {rng.randrange(modulus)}' for _ in range(n)]
    else:
        bound = modulus if problem.startswith('dynamic') else 10000
        lines += [' '.join(str(rng.randrange(bound)) for _ in range(n))]
    for i in range(q):
        if problem.startswith('dynamic'):
            kind = i % 5
            if kind == 0:
                lines.append(f'0 {rng.randrange(size + 1)} {rng.randrange(modulus)}')
                size += 1
            elif kind == 1:
                lines.append(f'1 {rng.randrange(size)}')
                size -= 1
            else:
                left, right = sorted([rng.randrange(size + 1), rng.randrange(size + 1)])
                row = f'{kind} {left} {right}'
                if kind == 3:
                    row += f' {rng.randrange(modulus)} {rng.randrange(modulus)}'
                lines.append(row)
        elif problem.startswith('double'):
            kind = i % 3
            if kind == 0:
                lines.append(f'0 {rng.randrange(10000)}')
                size += 1
            else:
                lines.append(str(kind))
                size -= 1
        elif i % 2 == 0:
            lines.append(f'0 {rng.randrange(n)} {rng.randrange(1, modulus)} {rng.randrange(modulus)}')
        else:
            left, right = sorted([rng.randrange(n + 1), rng.randrange(n + 1)])
            lines.append(f'1 {left} {right} {rng.randrange(modulus)}')
    (BASE / (problem + '.in')).write_text('\n'.join(lines) + '\n')

(BASE / 'fastest-prepared.json').write_text(json.dumps(
    dict(flags=FLAGS, rows=rows, seed=20260917, n=50000, q=50000), indent=2) + '\n')
