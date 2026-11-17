import ctypes
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys
import time

root = Path.cwd()
here = root / 'docs/development/measurements/polynomial-product-skew-20261117'
work = root / '.build/polynomial-product-skew-20261117'
work.mkdir(parents=True, exist_ok=True)
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
compiler = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(vc / 'bin/Hostx64/x64') + os.pathsep + env['PATH']
env['INCLUDE'] = os.pathsep.join(map(str, [vc / 'include', *(sdk / 'Include' / version / p for p in ('ucrt', 'shared', 'um'))]))
env['LIB'] = os.pathsep.join(map(str, [vc / 'lib/x64', *(sdk / 'Lib' / version / p / 'x64' for p in ('ucrt', 'um'))]))
exe = work / 'public-bench.exe'
names = ['baseline-original.hpp.txt', 'public-bench.cpp', 'public-run.py']
hashes = {n: hashlib.sha256((here / n).read_bytes()).hexdigest() for n in names}
hashes['blueberry/math/polynomial-product.hpp'] = hashlib.sha256((root / 'blueberry/math/polynomial-product.hpp').read_bytes()).hexdigest()
if '--build' in sys.argv:
    cmd = [str(compiler), '--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std=gnu++20', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Wshadow', '-Werror', '-isystem', str(root), str(here/'public-bench.cpp'), '-lpsapi', '-o', str(exe)]
    ver = subprocess.run([str(compiler), '--version'], capture_output=True, text=True)
    r = subprocess.run(cmd, env=env, capture_output=True, text=True)
    data = {'compiler':ver.stdout, 'command':cmd, 'source_sha256':hashes, 'exit_code':r.returncode, 'stdout':r.stdout, 'stderr':r.stderr}
    if not r.returncode:
        c = subprocess.run([str(exe), 'check'], capture_output=True, text=True)
        data['check'] = {'exit_code':c.returncode, 'stdout':c.stdout, 'stderr':c.stderr}
    (here/'public-build.json').write_text(json.dumps(data, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(data, indent=2), flush=True)
    if r.returncode or data.get('check', {}).get('exit_code') != 0: sys.exit(1)
    sys.exit()
kernel = ctypes.windll.kernel32
kernel.GetCurrentProcess.restype = ctypes.c_void_p
kernel.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
kernel.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
proc = kernel.GetCurrentProcess(); mask = ctypes.c_size_t(); system = ctypes.c_size_t()
if not kernel.GetProcessAffinityMask(proc, ctypes.byref(mask), ctypes.byref(system)): raise OSError('GetProcessAffinityMask')
one = mask.value & -mask.value
if not kernel.SetProcessAffinityMask(proc, one): raise OSError('SetProcessAffinityMask')
cases = [('empty', 100000), ('single64', 8000), ('two8', 5000), ('uniform_linear8192', 2), ('uniform_len16_1024', 3), ('skew_first', 2), ('skew_last', 2), ('many_constant_skew', 2), ('constants_only', 5), ('small_skew', 1000), ('zero_constant_skew', 2), ('mixed_constants', 200), ('ratio8', 15), ('ratio4', 15), ('ratio1point5', 30), ('four_skew', 5000), ('eight_skew', 4000), ('half_constant', 150), ('few_constants', 100), ('mostly_constants', 150)]
data = {'environment':{'platform':platform.platform(), 'cpu':platform.processor(), 'compiler':'Clang 18 MSVC ABI', 'flags':'-std=gnu++20 -O2 -DNDEBUG', 'affinity':hex(one), 'original_affinity':hex(mask.value), 'warmups':1, 'runs':5, 'seed':20261117}, 'source_sha256':hashes, 'cases':[]}
for category, (name, count) in enumerate(cases):
    entry = {'category':category, 'name':name, 'count':count, 'samples':[]}; expected = None
    data['cases'].append(entry)
    for sample in range(6):
        for mode in ([0,4] if sample % 2 == 0 else [4,0]):
            cmd = [str(exe), str(mode), str(category), str(count)]
            started = time.time(); r = subprocess.run(cmd, capture_output=True, text=True); ended = time.time()
            row = {'sample':sample, 'mode':mode, 'start_unix':started, 'end_unix':ended, 'exit_code':r.returncode, 'stdout':r.stdout, 'stderr':r.stderr}
            entry['samples'].append(row)
            (here/'public-results.json').write_text(json.dumps(data, indent=2)+'\n', encoding='utf-8')
            if r.returncode: raise RuntimeError(f'run failed {cmd}: {r.stderr}')
            parts = r.stdout.split()
            if len(parts) != 3 or parts[2] == 'missing': raise RuntimeError('missing timing or memory: '+r.stdout)
            row.update({'ms':float(parts[0]), 'checksum':parts[1], 'peak_working_set':int(parts[2])})
            if expected is None: expected = parts[1]
            if expected != parts[1]: raise RuntimeError('checksum mismatch')
            print(name, sample, mode, parts[0], flush=True)
    entry['summaries'] = {str(mode):{'median_ms':statistics.median(x['ms'] for x in entry['samples'] if x['mode']==mode and x['sample']), 'min_ms':min(x['ms'] for x in entry['samples'] if x['mode']==mode and x['sample'])} for mode in (0, 4)}
    (here/'public-results.json').write_text(json.dumps(data, indent=2)+'\n', encoding='utf-8')
    print('summary', name, entry['summaries'], flush=True)
