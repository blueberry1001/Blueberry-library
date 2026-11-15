import datetime
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
out = build / (sys.argv[1] if len(sys.argv) > 1 else 'pilot.json')
rounds = int(sys.argv[2]) if len(sys.argv) > 2 else 3
variants = sys.argv[3].split(',') if len(sys.argv) > 3 else ['base', 'four', 'radix']
profiles = sys.argv[4].split(',') if len(sys.argv) > 4 else ['release']
shapes = sys.argv[5].split(',') if len(sys.argv) > 5 else ['sparse', 'dense', 'zero', 'ties', 'disconnected', 'line', 'tiny', 'tiny64', 'tiny65', 'small128']
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
files = [build / 'benchmark.cpp', *(build / v / 'blueberry/graph/dijkstra.hpp' for v in variants)]
metadata = {'started': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'platform': platform.platform(),
            'machine': platform.machine(), 'cpu': Path('/proc/cpuinfo').read_text().split('model name\t: ')[1].split('\n')[0],
            'cpu_affinity': [cpu], 'seed': 20261115, 'rounds': rounds, 'shapes': shapes, 'variants': variants, 'profiles': profiles,
            'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
            'sha256': {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest() for p in files},
            'compilers': {}, 'builds': [], 'samples': [], 'failures': []}
def save(): out.write_text(json.dumps(metadata, indent=2) + '\n')
def run(args, **kw): return subprocess.run(args, capture_output=True, text=True, **kw)
for compiler in ['g++', 'clang++']:
    metadata['compilers'][compiler] = subprocess.check_output([compiler, '--version'], text=True).splitlines()[0]
    for profile in profiles:
        for variant in variants:
            exe = f'/tmp/blueberry-dijkstra-{compiler.replace("+", "p")}-{profile}-{variant}'
            flags = ['-std=gnu++20', '-O2', '-Wall', '-Wextra', '-Werror'] + (['-DNDEBUG'] if profile == 'release' else [])
            command = [compiler, *flags, '-I' + str(build / variant), '-I' + str(root), str(files[0]), '-o', exe]
            p = run(command)
            metadata['builds'].append({'command': command, 'returncode': p.returncode, 'stderr': p.stderr,
                                       'sha256': hashlib.sha256(Path(exe).read_bytes()).hexdigest() if p.returncode == 0 else None})
            save()
            if p.returncode: sys.exit(1)
            for shape in shapes:
                p = run([exe, shape])
                if p.returncode: metadata['failures'].append({'warmup': [compiler, profile, variant, shape], 'stderr': p.stderr}); save(); sys.exit(1)
        for shape in shapes:
            expected = None
            for repeat in range(rounds):
                order = variants[repeat % len(variants):] + variants[:repeat % len(variants)]
                for variant in order:
                    exe = f'/tmp/blueberry-dijkstra-{compiler.replace("+", "p")}-{profile}-{variant}'
                    mem = f'/tmp/blueberry-dijkstra-{os.getpid()}.rss'
                    start = time.perf_counter()
                    p = run(['/usr/bin/time', '-f', '%M', '-o', mem, exe, shape])
                    wall_ms = (time.perf_counter() - start) * 1000
                    if p.returncode: metadata['failures'].append({'case': [compiler, profile, variant, shape, repeat], 'stderr': p.stderr}); save(); sys.exit(1)
                    fields = p.stdout.split()
                    digest = fields[3]
                    if expected is None: expected = digest
                    if digest != expected: metadata['failures'].append({'case': [compiler, profile, variant, shape, repeat], 'digest': digest, 'expected': expected}); save(); sys.exit(1)
                    metadata['samples'].append({'compiler': compiler, 'profile': profile, 'variant': variant, 'shape': shape, 'run': repeat,
                                                'checksum': digest, 'build_ms': float(fields[4]), 'operation_ms': float(fields[5]),
                                                'wall_ms': wall_ms, 'max_rss_kib': int(Path(mem).read_text().strip())})
                    save()
                    print(compiler, profile, variant, shape, repeat, fields[5], flush=True)
metadata['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
save()
