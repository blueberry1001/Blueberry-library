import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
base = root / '.build/ordered-erase'
output = root / 'docs/development/measurements/ordered-iterator-20261117'
output.mkdir(parents=True, exist_ok=True)
env = dict(os.environ)
msvc = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.43.34808'
sdk = Path(os.environ['ProgramFiles(x86)']) / 'Windows Kits/10'
sdk_version = '10.0.22621.0'
env['PATH'] = str(msvc / 'bin/Hostx64/x64') + os.pathsep + env['PATH']
env['INCLUDE'] = os.pathsep.join(str(p) for p in [msvc / 'include', sdk / 'Include' / sdk_version / 'ucrt',
                          sdk / 'Include' / sdk_version / 'shared', sdk / 'Include' / sdk_version / 'um'])
env['LIB'] = os.pathsep.join(str(p) for p in [msvc / 'lib/x64', sdk / 'Lib' / sdk_version / 'ucrt/x64',
                                               sdk / 'Lib' / sdk_version / 'um/x64'])
cl = str(msvc / 'bin/Hostx64/x64/cl.exe')

digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
driver = root / 'benchmark/ordered-iterator-erase.cpp'
sources = {'baseline': base / 'baseline.hpp', 'candidate': root / 'blueberry/extra-fast/ordered-map.hpp'}
version = subprocess.run([cl], env=env, capture_output=True)
result = {'platform': platform.platform(), 'processor': platform.processor(), 'runs': 5, 'warmups': 1,
          'compiler': (version.stdout + version.stderr).decode('utf-8', errors='replace'), 'flags': ['/std:c++20', '/O2', '/DNDEBUG', '/EHsc'],
          'driver_sha256': digest(driver), 'sources': {}, 'samples': [], 'failed': []}
for variant, source in sources.items():
    include = base / variant / 'blueberry/extra-fast'
    include.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source, include / 'ordered-map.hpp')
    shutil.copyfile(source, output / (variant + '.hpp.snapshot'))
    result['sources'][variant] = digest(source)
    exe = base / (variant + '.exe')
    command = [cl, '/nologo', '/std:c++20', '/O2', '/DNDEBUG', '/EHsc', '/I' + str(base / variant),
               str(driver), '/Fe:' + str(exe), '/Fo:' + str(base / (variant + '.obj'))]
    proc = subprocess.run(command, cwd=base, env=env, capture_output=True)
    (output / (variant + '-compile.log')).write_bytes(proc.stdout + proc.stderr)
    if proc.returncode:
        result['failed'].append({'variant': variant, 'phase': 'compile', 'exit': proc.returncode})
        (output / 'results.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
        sys.exit(1)
    result['sources'][variant + '_executable'] = digest(exe)

for n in (64, 4096, 100000):
    for kind, seed in itertools.product(('set', 'map'), (0, 93841)):
        for mode in ('forward', 'alternate', 'reverse', 'random'):
            for rep in range(-1, 5):
                variants = ('baseline', 'candidate') if rep % 2 else ('candidate', 'baseline')
                pair = []
                for variant in variants:
                    proc = subprocess.run([str(base / (variant + '.exe')), str(n), mode, kind, str(seed)],
                                          capture_output=True, text=True)
                    if proc.returncode:
                        result['failed'].append({'n': n, 'kind': kind, 'mode': mode, 'seed': seed, 'rep': rep,
                                                 'variant': variant, 'stderr': proc.stderr, 'exit': proc.returncode})
                        continue
                    fields = proc.stdout.split()
                    sample = {'n': n, 'kind': kind, 'mode': mode, 'seed': seed, 'rep': rep, 'variant': variant,
                              'build_ms': float(fields[0]), 'erase_ms': float(fields[1]),
                              'digest': fields[2], 'size': int(fields[3])}
                    result['samples'].append(sample)
                    pair.append((sample['digest'], sample['size']))
                if len(pair) != 2 or pair[0] != pair[1]:
                    result['failed'].append({'n': n, 'kind': kind, 'mode': mode, 'seed': seed, 'rep': rep, 'checksums': pair})
    (output / 'results.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
if result['failed']:
    sys.exit(1)
