#!/usr/bin/env python3
"""Reproducible same-contract stream/global-fast/direct-fast I/O comparison."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
PROFILES = {
    'gcc-release': ['g++', '-std=gnu++20', '-O2', '-DNDEBUG'],
    'gcc-assert': ['g++', '-std=gnu++20', '-O2', '-D_GLIBCXX_ASSERTIONS'],
    'clang-release': ['clang++', '-std=gnu++20', '-O2', '-DNDEBUG'],
}
BACKENDS = {'streams': 0, 'contest-fast': 1, 'direct-fast': 2}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--acl-root', type=Path, default=ROOT / '.deps/ac-library')
    parser.add_argument('--count', type=int, default=1000000)
    parser.add_argument('--float-count', type=int, default=250000)
    parser.add_argument('--repeats', type=int, default=5)
    parser.add_argument('--profile', choices=list(PROFILES), action='append')
    parser.add_argument('--revision', help='Explicit git revision for Windows-created worktrees run from WSL')
    parser.add_argument('--cpu', type=int, help='Pin to this allowed logical CPU')
    parser.add_argument('--baseline-header', type=Path, help='Add interleaved contest-fast-before backend using this header snapshot')
    args = parser.parse_args()
    if args.repeats < 3 or args.count <= 0 or args.float_count <= 0:
        parser.error('positive counts and at least 3 repeats are required')
    args.output.mkdir(parents=True, exist_ok=False)
    cpu = args.cpu if args.cpu is not None else min(os.sched_getaffinity(0))
    if cpu not in os.sched_getaffinity(0): parser.error('CPU is not in the allowed affinity set')
    os.sched_setaffinity(0, {cpu})
    profiles = {p: PROFILES[p] for p in args.profile or PROFILES}
    tracked = [ROOT / p for p in ['benchmark/contest-fast-io.cpp',
               'benchmark/contest-fast-io.py', 'blueberry/utility/fast-io.hpp',
               'blueberry/utility/contest-fast-io.hpp', 'blueberry/utility/contest-template.hpp',
               'blueberry/utility/internal/contest-template-base.hpp']]
    if args.baseline_header:
        args.baseline_header = args.baseline_header.resolve()
        tracked.append(args.baseline_header)
    hashes = {str(p.relative_to(ROOT)): sha(p.read_bytes()) for p in tracked}
    env = {'timestamp_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
           'git_revision': args.revision or subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
           'platform': platform.platform(), 'cpuinfo': Path('/proc/cpuinfo').read_text().split('\n\n')[0],
           'cpu_affinity': cpu, 'profiles': profiles, 'repeats': args.repeats, 'warmups': 1,
           'count': args.count, 'float_count': args.float_count, 'source_sha256': hashes,
           'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in {p[0] for p in profiles.values()}},
           'contract': 'Valid signed 64-bit operands within +/-10^18; sums fit. Same bytes and order; fixed float precision=30. Input checksum modulo 2^64.',
           'timing': 'construction_ms: Backend initialization; operation_ms: parse/checksum or format/write/flush; process_ms includes startup, initialization, data generation, destruction and time wrapper. Peak RSS is whole-process.',
           'ordering': 'Seed 20261020 shuffle each round; one pinned process at a time; file and pipe stdin; stdout regular cached /tmp file.',
           'baseline_header': str(args.baseline_header) if args.baseline_header else None}
    save(args.output / 'environment.json', env)
    rows = []
    try:
        with tempfile.TemporaryDirectory(prefix='blueberry-contest-fast-io-') as directory:
            temp = Path(directory)
            backends = dict(BACKENDS)
            if args.baseline_header:
                backends['contest-fast-before'] = 1
                overlay = temp / 'baseline/blueberry/utility/contest-fast-io.hpp'
                overlay.parent.mkdir(parents=True)
                overlay.write_bytes(args.baseline_header.read_bytes())
            rng = random.Random(20261020)
            input_data = bytearray(f'{args.count}\n'.encode())
            output_data = bytearray()
            checksum = 0
            for _ in range(args.count):
                a = rng.randrange(-10**18, 10**18 + 1)
                b = rng.randrange(-10**18, 10**18 + 1)
                input_data += f'{a} {b}\n'.encode()
                output_data += f'{a+b}\n'.encode()
                checksum = (checksum+a+b) % (1 << 64)
            input_data = bytes(input_data)
            expected = {'input': f'{checksum}\n'.encode(), 'end-to-end': bytes(output_data),
                        'output': b''.join(f'{i*1000000007 % (2*10**18)-10**18}\n'.encode() for i in range(args.count)),
                        'float-output': b''.join(f'{(i%100000-50000)/16:.30f}\n'.encode() for i in range(args.float_count))}
            (temp / 'input').write_bytes(input_data)
            env['input'] = {'sha256': sha(input_data), 'bytes': len(input_data), 'seed': 20261020}
            env['expected'] = {m: {'sha256': sha(data), 'bytes': len(data)} for m, data in expected.items()}
            save(args.output / 'environment.json', env)
            binaries = {}
            with (args.output / 'compile.log').open('w') as log:
                for profile, flags in profiles.items():
                    for backend, code in backends.items():
                        binary = temp / f'{profile}-{backend}'
                        extra = ['-I'+str(temp / 'baseline')] if backend == 'contest-fast-before' else []
                        cmd = [*flags, '-DIO_BACKEND='+str(code), *extra, '-I'+str(ROOT), '-isystem', str(args.acl_root),
                               str(ROOT / 'benchmark/contest-fast-io.cpp'), '-o', str(binary)]
                        log.write(json.dumps(cmd)+'\n'); log.flush()
                        subprocess.run(cmd, stdout=log, stderr=log, check=True)
                        binaries[profile, backend] = binary
            configs = [(p, b, m, t) for p, b, m in itertools.product(profiles, backends, expected)
                       for t in (['file', 'pipe'] if m in ['input', 'end-to-end'] else ['none'])]
            with (args.output / 'samples.jsonl').open('w') as log:
                for run in range(-1, args.repeats):
                    rng.shuffle(configs)
                    for profile, backend, mode, transport in configs:
                        dest_path, rss = temp / 'output', temp / 'rss'
                        count = args.float_count if mode == 'float-output' else args.count
                        cmd = ['/usr/bin/time', '-f', '%M', '-o', str(rss), str(binaries[profile, backend]), mode, str(count)]
                        with dest_path.open('wb') as dest:
                            begin = time.perf_counter()
                            if transport == 'file':
                                with (temp / 'input').open('rb') as source:
                                    result = subprocess.run(cmd, stdin=source, stdout=dest, stderr=subprocess.PIPE, timeout=60)
                            else:
                                result = subprocess.run(cmd, input=input_data if transport == 'pipe' else b'', stdout=dest, stderr=subprocess.PIPE, timeout=60)
                            process_ms = (time.perf_counter()-begin)*1000
                        row = {'profile': profile, 'backend': backend, 'mode': mode, 'transport': transport,
                               'run': run, 'warmup': run < 0, 'returncode': result.returncode}
                        if result.returncode or dest_path.read_bytes() != expected[mode]:
                            save(args.output / 'failure.json', {**row, 'stderr': result.stderr.decode(errors='replace'),
                                 'actual_sha256': sha(dest_path.read_bytes()), 'expected_sha256': sha(expected[mode])})
                            raise RuntimeError('Execution/output mismatch; see failure.json')
                        row.update(json.loads(result.stderr), process_ms=process_ms, peak_rss_kib=int(rss.read_text()), output_byte_match=True)
                        log.write(json.dumps(row)+'\n'); log.flush()
                        if run >= 0: rows.append(row)
                    print('Finished round', run, flush=True)
        if hashes != {str(p.relative_to(ROOT)): sha(p.read_bytes()) for p in tracked}:
            raise RuntimeError('Sources changed during benchmark; results not comparable')
        summary = []
        for config in sorted(configs):
            group = [r for r in rows if tuple(r[k] for k in ['profile', 'backend', 'mode', 'transport']) == config]
            row = {k: group[0][k] for k in ['profile', 'backend', 'mode', 'transport']}
            for field in ['construction_ms', 'operation_ms', 'process_ms', 'peak_rss_kib']:
                row[field] = {'median': statistics.median(r[field] for r in group),
                              'min': min(r[field] for r in group), 'max': max(r[field] for r in group)}
            summary.append(row)
        save(args.output / 'summary.json', summary)
    except Exception as error:
        save(args.output / 'run-error.json', {'type': type(error).__name__, 'message': str(error)})
        raise


if __name__ == '__main__':
    main()
