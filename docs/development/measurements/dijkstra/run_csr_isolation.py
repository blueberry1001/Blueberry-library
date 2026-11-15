import ctypes
import hashlib
import json
import os
import platform
import statistics
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[4]
source = Path(__file__).with_name('csr-isolation.cpp')
clang = len(sys.argv) > 2 and sys.argv[2] == 'clang'
exe = root / ('.build/dijkstra-csr-isolation/csr-clang.exe' if clang else '.build/dijkstra-csr-isolation/csr.exe')
header = root / 'blueberry/graph/dijkstra.hpp'
output = Path(__file__).with_name(sys.argv[1] if len(sys.argv) > 1 else 'csr-isolation-msvc.json')
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
result = {'status': 'running', 'compiler': 'Clang 18.0.3 Windows MSVC ABI' if clang else 'MSVC 14.43.34808',
          'flags': (['--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std=gnu++20', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Wshadow', '-Werror'] if clang else ['/std:c++20', '/EHsc', '/W4', '/WX', '/O2', '/DNDEBUG']),
          'platform': platform.platform(), 'processor': platform.processor(),
          'source_sha256': sha(source), 'header_sha256': sha(header),
          'executable_sha256': sha(exe), 'rounds': 8, 'samples': [], 'failures': []}
def save():
    output.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')

if os.name == 'nt':
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.GetCurrentProcess.restype = ctypes.c_void_p
    kernel.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t)]
    kernel.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    process, system = ctypes.c_size_t(), ctypes.c_size_t()
    handle = kernel.GetCurrentProcess()
    if kernel.GetProcessAffinityMask(handle, ctypes.byref(process), ctypes.byref(system)) and process.value:
        mask = process.value & -process.value
        result['affinity_mask'] = hex(mask) if kernel.SetProcessAffinityMask(handle, mask) else None
    else:
        result['affinity_mask'] = None
save()
for shape in ['random', 'zero', 'line', 'dag']:
    p = subprocess.run([str(exe), shape], capture_output=True, text=True)
    if p.returncode:
        result['status'] = 'failed'
        result['failures'].append({'shape': shape, 'returncode': p.returncode, 'stderr': p.stderr, 'stdout': p.stdout})
        save()
        sys.exit(p.returncode)
    for line in p.stdout.splitlines():
        s, round_num, csr, edges, build, search = line.split()
        result['samples'].append({'shape': s, 'round': int(round_num), 'csr': bool(int(csr)),
                                  'edges': int(edges), 'build_ms': float(build), 'search_ms': float(search)})
    save()
result['summary'] = {}
for shape in ['random', 'zero', 'line', 'dag']:
    result['summary'][shape] = {}
    for csr in [False, True]:
        rows = [s for s in result['samples'] if s['shape'] == shape and s['csr'] == csr]
        result['summary'][shape]['csr' if csr else 'nested'] = {
            'edges': rows[0]['edges'],
            'build_median_ms': statistics.median(s['build_ms'] for s in rows),
            'search_median_ms': statistics.median(s['search_ms'] for s in rows),
            'total_median_ms': statistics.median(s['build_ms'] + s['search_ms'] for s in rows)}
result['status'] = 'passed'
save()
print(json.dumps(result['summary'], indent=2))
