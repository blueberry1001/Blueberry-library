import ctypes
import hashlib
import json
import os
import platform
import statistics
import subprocess
from pathlib import Path

root = Path('.build/inversions-experiment')
output = Path('docs/development/measurements/inversions-20261117/affinity-rerun.json')
kernel = ctypes.WinDLL('kernel32', use_last_error=True)
size_t = ctypes.c_size_t
kernel.GetCurrentProcess.restype = ctypes.c_void_p
kernel.GetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.POINTER(size_t), ctypes.POINTER(size_t)]
kernel.GetProcessAffinityMask.restype = ctypes.c_int
kernel.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, size_t]
kernel.SetProcessAffinityMask.restype = ctypes.c_int

def affinity(handle):
    process, system = size_t(), size_t()
    if not kernel.GetProcessAffinityMask(handle, ctypes.byref(process), ctypes.byref(system)):
        raise ctypes.WinError(ctypes.get_last_error())
    return process.value, system.value

handle = kernel.GetCurrentProcess()
allowed, system = affinity(handle)
chosen = allowed & -allowed
if not chosen or not kernel.SetProcessAffinityMask(handle, chosen):
    raise ctypes.WinError(ctypes.get_last_error())
rows = []
record = {'platform': platform.platform(), 'processor': os.environ.get('PROCESSOR_IDENTIFIER'),
          'parent_initial_mask': hex(allowed), 'system_mask': hex(system), 'chosen_mask': hex(chosen),
          'seed': 20261117, 'n': 20000, 'q': 200000, 'shape': 'random', 'warmup': 3, 'repeats': 20,
          'timing': 'entire function, excludes input generation and checksum',
          'flags': '--target=x86_64-pc-windows-msvc -fuse-ld=link -std=gnu++20 -O2 -DNDEBUG',
          'sha256': {f: hashlib.sha256((root / f).read_bytes()).hexdigest() for f in ['bench.exe','bench.cpp','baseline.hpp','shortguard.hpp']},
          'rows': rows}
for run in range(23):
    for mode in (['baseline', 'shortguard'] if run % 2 == 0 else ['shortguard', 'baseline']):
        proc = subprocess.Popen([str(root / 'bench.exe'), mode, '20000', '200000', 'random'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        child_mask, child_system = affinity(int(proc._handle))
        stdout, stderr = proc.communicate()
        row = {'run': run, 'warmup': run < 3, 'mode': mode, 'returncode': proc.returncode,
               'child_mask': hex(child_mask), 'child_system_mask': hex(child_system), 'stdout': stdout, 'stderr': stderr}
        if proc.returncode == 0:
            row.update(json.loads(stdout))
        rows.append(row)
        output.write_text(json.dumps(record, indent=2) + '\n')
        print(run, mode, row.get('ms'), hex(child_mask), flush=True)
        if proc.returncode or child_mask != chosen:
            raise RuntimeError(row)
assert len({r['checksum'] for r in rows}) == 1
record['summary'] = []
for mode in ('baseline', 'shortguard'):
    samples = [r['ms'] for r in rows if not r['warmup'] and r['mode'] == mode]
    record['summary'].append({'mode': mode, 'median_ms': statistics.median(samples), 'min_ms': min(samples), 'max_ms': max(samples)})
output.write_text(json.dumps(record, indent=2) + '\n')
