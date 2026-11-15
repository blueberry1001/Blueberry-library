import argparse
import hashlib
import json
import os
import platform
import random
import statistics
import subprocess
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
SOURCE = HERE / 'driver.cpp'
TRUSTED = HERE / 'trusted-input.hpp.txt'
SAFE = ROOT / 'blueberry/utility/fast-io.hpp'
MASK, PRIME = (1 << 64) - 1, 1099511628211


def sha(data):
    return hashlib.sha256(data).hexdigest()


def fixture(mode):
    limits = {'i32': (-(1 << 31), (1 << 31) - 1), 'u32': (0, (1 << 32) - 1),
              'i64': (-(1 << 63), (1 << 63) - 1), 'u64': (0, (1 << 64) - 1)}
    low, high = limits[mode]
    values = [low, high, 0, 1, high, low, 0, 17, high, low, 10, 42, 100000, 0, 5]
    rng = random.Random(20261116)
    values += [rng.randint(low, high) for _ in range(20)]
    data = bytearray(f'{len(values)}\n'.encode())
    expected = bytearray()
    offsets = []
    for i, value in enumerate(values):
        target = ((len(data) + 65535) // 65536) * 65536 + (-3, -2, -1, 0, 1, 2, 3)[i % 7]
        if target < len(data): target += 65536
        pad, blanks = target - len(data), b' \t\n\r\v\f'
        data.extend((blanks * ((pad + len(blanks) - 1) // len(blanks)))[:pad])
        zeros = '0' * (70000 if i == 4 else i % 4)
        sign = '-' if value < 0 or (low < 0 and value == 0 and i % 2 == 0) else ('+' if i % 3 == 0 else '')
        offsets.append(len(data) % 65536)
        data.extend((sign + zeros + str(abs(value))).encode('ascii'))
        if i + 1 < len(values): data.extend(b'\n')
        expected.extend(f'{value}\n'.encode())
    return bytes(data), bytes(expected), offsets


def make_case(path, shape):
    n, q = 1000000, 500000
    rng = random.Random(20261116)
    bits = ('01' * (n // 2)) if shape == 'varied' else ('1' * n if shape == 'maximum' else '0' * n)
    checksum = 14695981039346656037
    with path.open('w', encoding='ascii', newline='\n') as out:
        out.write(f'{n} {q}\n{bits}\n')
        for i in range(q):
            kind = rng.randrange(5)
            x = rng.randrange(n) if shape == 'varied' else (n - 1 if shape == 'maximum' else i % 10)
            checksum = ((checksum ^ kind) * PRIME) & MASK
            checksum = ((checksum ^ x) * PRIME) & MASK
            out.write(f'{kind} {x}' + ('\n' if i + 1 < q else ''))
    expected = f'{n} {q} {n} {bits.count("1")} {checksum}\n'.encode('ascii')
    return expected, {'input_sha256': sha(path.read_bytes()), 'expected_sha256': sha(expected), 'n': n, 'q': q}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=15)
    args = parser.parse_args()
    if args.output.exists() or args.rounds < 1:
        parser.error('choose a new output and positive round count')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    data = {'status': 'running', 'compiler': args.compiler, 'platform': platform.platform(),
            'processor': platform.processor(), 'timestamp_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
            'warmups': 1, 'rounds': args.rounds, 'seed': 20261116,
            'source_sha256': sha(SOURCE.read_bytes()), 'prototype_sha256': sha(TRUSTED.read_bytes()),
            'fastio_sha256': sha(SAFE.read_bytes()), 'runner_sha256': sha(Path(__file__).read_bytes()),
            'builds': [], 'fixtures': {}, 'validation': [], 'cases': {}, 'samples': [], 'failures': []}
    def save():
        args.output.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    save()
    try:
        cpuinfo = Path('/proc/cpuinfo').read_text()
        data['cpu_model'] = next((x.split(':', 1)[1].strip() for x in cpuinfo.splitlines() if x.startswith('model name')), None)
        flags = set(next((x.split(':', 1)[1].split() for x in cpuinfo.splitlines() if x.startswith('flags')), []))
        if not {'avx2', 'popcnt'}.issubset(flags): raise RuntimeError('AVX2/POPCNT not reported; measurements not run')
        try:
            cpus = sorted(os.sched_getaffinity(0))
            os.sched_setaffinity(0, {cpus[0]})
            data['cpu_affinity'] = sorted(os.sched_getaffinity(0))
        except (AttributeError, OSError, IndexError) as e:
            data['cpu_affinity'] = None
            data['affinity_error'] = str(e)
        v = subprocess.run([args.compiler, '--version'], capture_output=True, text=True, check=True)
        data['compiler_version'] = v.stdout.splitlines()[0]
        with tempfile.TemporaryDirectory(prefix='blueberry-trusted-input-') as folder:
            tmp = Path(folder)
            bins = {}
            for variant in ('safe', 'trusted'):
                exe = tmp / variant
                cmd = [args.compiler, '-std=gnu++23', '-O2', '-DNDEBUG', '-mavx2', '-mpopcnt', '-Wall', '-Wextra', '-Wshadow', '-Werror', '-I' + str(ROOT)]
                if variant == 'trusted': cmd.append('-DTRUSTED')
                cmd += [str(SOURCE), '-o', str(exe)]
                p = subprocess.run(cmd, capture_output=True, text=True)
                data['builds'].append({'variant': variant, 'command': cmd, 'exit': p.returncode, 'stdout': p.stdout, 'stderr': p.stderr,
                                       'binary_sha256': sha(exe.read_bytes()) if not p.returncode else None})
                save()
                if p.returncode: raise RuntimeError(f'{variant} compilation failed')
                bins[variant] = exe
            for mode in ('i32', 'u32', 'i64', 'u64'):
                input_data, expected, offsets = fixture(mode)
                path = tmp / (mode + '.in')
                path.write_bytes(input_data)
                data['fixtures'][mode] = {'input_sha256': sha(input_data), 'expected_sha256': sha(expected),
                                          'input_bytes': len(input_data), 'boundary_offsets': offsets}
                for variant in ('safe', 'trusted'):
                    for transport in ('file', 'pipe'):
                        if transport == 'pipe':
                            p = subprocess.run([str(bins[variant]), mode], input=input_data, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        else:
                            with path.open('rb') as source:
                                p = subprocess.run([str(bins[variant]), mode], stdin=source, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                        row = {'mode': mode, 'variant': variant, 'transport': transport, 'exit': p.returncode,
                               'output_sha256': sha(p.stdout), 'matches': p.stdout == expected, 'stderr': p.stderr.decode('utf-8', errors='replace')}
                        data['validation'].append(row)
                        save()
                        if p.returncode or not row['matches']: raise RuntimeError(f'valid-input mismatch: {mode}/{variant}/{transport}')
            for shape in ('short', 'varied', 'maximum'):
                path = tmp / (shape + '.in')
                expected, data['cases'][shape] = make_case(path, shape)
                save()
                for repeat in range(-1, args.rounds):
                    order = ('safe', 'trusted') if repeat % 2 == 0 else ('trusted', 'safe')
                    for variant in order:
                        with path.open('rb') as source:
                            begin = time.perf_counter()
                            p = subprocess.run([str(bins[variant])], stdin=source, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                            wall = (time.perf_counter() - begin) * 1000
                        if p.returncode or p.stdout != expected:
                            raise RuntimeError(f'{shape}/{variant} failed or output mismatch: {p.returncode} {p.stderr!r}')
                        metric = json.loads(p.stderr.decode('ascii'))
                        if not metric.get('peak_rss_bytes'): raise RuntimeError('peak RSS not measured')
                        if repeat >= 0:
                            data['samples'].append({'shape': shape, 'variant': variant, 'round': repeat,
                                                    'wall_ms': wall, 'output_sha256': sha(p.stdout), **metric})
                        save()
            data['summary'] = {}
            for shape in ('short', 'varied', 'maximum'):
                for variant in ('safe', 'trusted'):
                    rows = [r for r in data['samples'] if r['shape'] == shape and r['variant'] == variant]
                    data['summary'][f'{shape}/{variant}'] = {key: statistics.median(r[key] for r in rows) for key in
                        ('wall_ms', 'prefix_ms', 'queries_ms', 'parse_ms', 'post_parse_ms', 'main_ms', 'peak_rss_bytes')}
        data['status'] = 'passed'
    except Exception as e:
        data['status'] = 'failed'
        data['failures'].append({'type': type(e).__name__, 'message': str(e)})
        save()
        raise
    save()
    print(json.dumps(data['summary'], indent=2))


if __name__ == '__main__':
    main()
