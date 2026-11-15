import argparse
import ctypes
import gc
import hashlib
import heapq
import json
import os
import platform
import random
import statistics
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
BUILD = ROOT / '.build/dijkstra-csr-end-to-end'
DRIVER = HERE / 'csr-end-to-end.cpp'
HEADER = ROOT / 'blueberry/graph/dijkstra.hpp'
IO = ROOT / 'blueberry/utility/fast-io.hpp'
SHAPES = ('random', 'zero', 'line', 'dag', 'disconnected')
VARIANTS = ('stream-nested', 'buffered-nested', 'buffered-csr')
SEED = 20261115
N, EXTRA = 100000, 400000


def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def prepare():
    BUILD.mkdir(parents=True, exist_ok=True)
    rows = {}
    for shape in SHAPES:
        rng = random.Random(SEED)
        edges = []
        limit = 1000 if shape == 'disconnected' else N
        for i in range(1, limit):
            edges.append((i - 1, i, 0 if shape == 'zero' else 1))
        if shape != 'line':
            for _ in range(EXTRA):
                if shape == 'disconnected' and rng.randrange(2):
                    a, b = rng.randrange(1000, N), rng.randrange(1000, N)
                else:
                    a, b = rng.randrange(limit), rng.randrange(limit)
                if shape == 'dag':
                    if a > b:
                        a, b = b, a
                    if a == b:
                        continue
                cost = 0 if shape == 'zero' else rng.randrange(1_000_000_000)
                edges.append((a, b, cost))
        path = BUILD / f'{shape}.in'
        with path.open('w', encoding='ascii', newline='\n') as out:
            out.write(f'{N} {len(edges)} 0 {N - 1}\n')
            out.writelines(f'{a} {b} {c}\n' for a, b, c in edges)
        rows[shape] = {'vertices': N, 'edges': len(edges), 'source': 0, 'target': N - 1,
                       'path': path.name, 'sha256': sha(path)}
        del edges
        gc.collect()
    (BUILD / 'inputs.json').write_text(json.dumps({'seed': SEED, 'shapes': rows}, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(rows, indent=2))


def check_path(input_path, output_path):
    with input_path.open(encoding='ascii') as source:
        n, m, start, target = map(int, source.readline().split())
        graph = [[] for _ in range(n)]
        for line in source:
            a, b, cost = map(int, line.split())
            graph[a].append((b, cost))
    dist = [None] * n
    dist[start] = 0
    pending = [(0, start)]
    while pending:
        cost, v = heapq.heappop(pending)
        if dist[v] != cost:
            continue
        if v == target:
            break
        for u, weight in graph[v]:
            nxt = cost + weight
            if dist[u] is None or nxt < dist[u]:
                dist[u] = nxt
                heapq.heappush(pending, (nxt, u))
    with output_path.open(encoding='ascii') as answer:
        first = list(map(int, answer.readline().split()))
        if first == [-1]:
            if dist[target] is not None or answer.read().strip():
                raise ValueError('incorrect unreachable output')
            return {'distance': None, 'edges': 0}
        distance, count = first
        current, total, visited = start, 0, {start}
        for _ in range(count):
            a, b = map(int, answer.readline().split())
            matches = [cost for vertex, cost in graph[current] if vertex == b]
            if a != current or not matches or b in visited:
                raise ValueError('invalid path edge')
            total += min(matches)
            current = b
            visited.add(b)
        if current != target or total != distance or distance != dist[target] or answer.read().strip():
            raise ValueError('incorrect distance or output')
        return {'distance': distance, 'edges': count}


def affinity():
    if os.name != 'nt':
        return None
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.GetCurrentProcess.restype = ctypes.c_void_p
    kernel.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
    kernel.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    process, system = ctypes.c_size_t(), ctypes.c_size_t()
    handle = kernel.GetCurrentProcess()
    if kernel.GetProcessAffinityMask(handle, ctypes.byref(process), ctypes.byref(system)) and process.value:
        mask = process.value & -process.value
        return hex(mask) if kernel.SetProcessAffinityMask(handle, mask) else None
    return None


def measure(args):
    inputs = json.loads((BUILD / 'inputs.json').read_text(encoding='utf-8'))
    bins = {v: BUILD / f'{args.compiler}-{v}.exe' for v in VARIANTS}
    output = HERE / args.output
    result = {'status': 'running', 'compiler': args.compiler, 'builds': [],
              'platform': platform.platform(), 'processor': platform.processor(), 'affinity_mask': affinity(),
              'rounds': args.rounds, 'warmups': 1, 'seed': SEED,
              'source_sha256': sha(DRIVER), 'header_sha256': sha(HEADER), 'io_sha256': sha(IO),
              'executables': {}, 'inputs': inputs['shapes'],
              'samples': [], 'verification': {}, 'failures': []}
    def save():
        output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    save()
    try:
        vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
        sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
        clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
        env = os.environ.copy()
        env['INCLUDE'] = ';'.join(str(p) for p in (vc / 'include', sdk / 'Include/10.0.22621.0/ucrt', sdk / 'Include/10.0.22621.0/shared', sdk / 'Include/10.0.22621.0/um'))
        env['LIB'] = ';'.join(str(p) for p in (vc / 'lib/x64', sdk / 'Lib/10.0.22621.0/ucrt/x64', sdk / 'Lib/10.0.22621.0/um/x64'))
        env['PATH'] = str(vc / 'bin/Hostx64/x64') + ';' + env.get('PATH', '')
        compiler = vc / 'bin/Hostx64/x64/cl.exe' if args.compiler == 'msvc' else clang
        ver = subprocess.run([str(compiler), '/Bv' if args.compiler == 'msvc' else '--version'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace', env=env)
        result['compiler_version'] = ver.stdout.splitlines()[0] if ver.stdout else None
        for variant in VARIANTS:
            define = {'stream-nested': 'STREAM_NESTED', 'buffered-csr': 'CSR_GRAPH'}.get(variant)
            if args.compiler == 'msvc':
                command = [str(compiler), '/nologo', '/std:c++20', '/EHsc', '/O2', '/W4', '/WX', '/DNDEBUG', '/I.']
                if define:
                    command.append('/D' + define)
                command += [str(DRIVER), '/Fo' + str(BUILD / f'{args.compiler}-{variant}.obj'), '/Fe' + str(bins[variant])]
            else:
                command = [str(compiler), '--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std=gnu++20', '-O2', '-Wall', '-Wextra', '-Wshadow', '-Werror', '-DNDEBUG', '-I.']
                if define:
                    command.append('-D' + define)
                command += [str(DRIVER), '-o', str(bins[variant])]
            p = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
            result['builds'].append({'variant': variant, 'command': command, 'exit': p.returncode, 'output': p.stdout})
            save()
            if p.returncode:
                raise RuntimeError(f'{variant} compilation failed')
            result['executables'][variant] = sha(bins[variant])
        if sha(HEADER) != result['header_sha256'] or sha(DRIVER) != result['source_sha256'] or sha(IO) != result['io_sha256']:
            raise ValueError('source changed during compilation')
        for shape in SHAPES:
            input_path = BUILD / inputs['shapes'][shape]['path']
            if sha(input_path) != inputs['shapes'][shape]['sha256']:
                raise ValueError(f'input hash changed: {shape}')
            expected_hash = None
            for round_num in range(-1, args.rounds):
                order = VARIANTS[round_num % len(VARIANTS):] + VARIANTS[:round_num % len(VARIANTS)]
                for variant in order:
                    answer = BUILD / f'{shape}-{variant}.out'
                    start = time.perf_counter()
                    with input_path.open('rb') as source, answer.open('wb') as out:
                        p = subprocess.run([str(bins[variant])], stdin=source, stdout=out, stderr=subprocess.PIPE)
                    wall = (time.perf_counter() - start) * 1000
                    if p.returncode:
                        raise RuntimeError(f'{shape} {variant} exit {p.returncode}: {p.stderr.decode(errors="replace")}')
                    row = json.loads(p.stderr.decode('ascii'))
                    if not row['peak_working_set_bytes']:
                        raise RuntimeError(f'{shape} {variant}: missing peak memory')
                    answer_hash = sha(answer)
                    if expected_hash is None:
                        expected_hash = answer_hash
                    if answer_hash != expected_hash:
                        raise ValueError(f'{shape} {variant}: output differs from other variants')
                    if round_num >= 0:
                        result['samples'].append({'shape': shape, 'variant': variant, 'round': round_num,
                                                  'wall_ms': wall, 'output_sha256': answer_hash, **row})
                    save()
            result['verification'][shape] = check_path(input_path, BUILD / f'{shape}-stream-nested.out')
            save()
            gc.collect()
        result['summary'] = {}
        for shape in SHAPES:
            result['summary'][shape] = {}
            for variant in VARIANTS:
                rows = [r for r in result['samples'] if r['shape'] == shape and r['variant'] == variant]
                fields = ('wall_ms', 'parse_ms', 'build_ms', 'parse_build_ms', 'search_ms', 'output_ms', 'peak_working_set_bytes')
                result['summary'][shape][variant] = {field: (statistics.median(r[field] for r in rows) if rows[0][field] is not None else None) for field in fields}
        result['status'] = 'passed'
    except Exception as exc:
        result['status'] = 'failed'
        result['failures'].append({'type': type(exc).__name__, 'message': str(exc)})
        save()
        raise
    save()
    print(json.dumps(result['summary'], indent=2))


parser = argparse.ArgumentParser()
parser.add_argument('--prepare', action='store_true')
parser.add_argument('--compiler', choices=('msvc', 'clang'))
parser.add_argument('--rounds', type=int, default=5)
parser.add_argument('--output', default='csr-end-to-end.json')
args = parser.parse_args()
if args.prepare:
    prepare()
else:
    if not args.compiler or args.rounds < 1:
        parser.error('--compiler and a positive --rounds are required')
    measure(args)
