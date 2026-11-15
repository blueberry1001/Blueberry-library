import ctypes
import hashlib
import json
import os
import platform
import shutil
import statistics
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
BUILD = ROOT / '.build/dijkstra-short-parse'
INPUTS = ROOT / '.build/dijkstra-csr-end-to-end'
TEST = ROOT / 'tests/random/fast-io.cpp'
DRIVER = HERE / 'csr-end-to-end.cpp'
GRAPH = ROOT / 'blueberry/graph/dijkstra.hpp'
BASE = HERE / 'parse-baseline.hpp.txt'
CANDIDATE = HERE / 'parse-short-limit-candidate.hpp.txt'
OUTPUT = HERE / (sys.argv[1] if len(sys.argv) > 1 else 'parse-short-windows.json')
SHAPES = ('random', 'zero', 'line', 'dag', 'disconnected')
COMPILERS = ('msvc', 'clang')
VARIANTS = ('baseline', 'candidate')
SEEDS = (1, 2, 17)
ROUNDS = 5


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def windows_env():
    vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
    sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
    clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
    env = os.environ.copy()
    env['INCLUDE'] = ';'.join(str(p) for p in (vc / 'include', sdk / 'Include/10.0.22621.0/ucrt', sdk / 'Include/10.0.22621.0/shared', sdk / 'Include/10.0.22621.0/um'))
    env['LIB'] = ';'.join(str(p) for p in (vc / 'lib/x64', sdk / 'Lib/10.0.22621.0/ucrt/x64', sdk / 'Lib/10.0.22621.0/um/x64'))
    env['PATH'] = str(vc / 'bin/Hostx64/x64') + ';' + env.get('PATH', '')
    return env, {'msvc': vc / 'bin/Hostx64/x64/cl.exe', 'clang': clang}


def affinity():
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


BUILD.mkdir(parents=True, exist_ok=True)
manifest = json.loads((INPUTS / 'inputs.json').read_text(encoding='utf-8'))
reference = json.loads((HERE / 'csr-end-to-end-msvc-current.json').read_text(encoding='utf-8'))
result = {'status': 'running', 'platform': platform.platform(), 'processor': platform.processor(),
          'affinity_mask': affinity(), 'rounds': ROUNDS, 'warmups': 1, 'seeds': SEEDS,
          'candidate_sha256': sha(CANDIDATE), 'baseline_sha256': sha(BASE), 'graph_sha256': sha(GRAPH),
          'test_sha256': sha(TEST), 'driver_sha256': sha(DRIVER), 'runner_sha256': sha(Path(__file__)),
          'input_manifest': manifest, 'builds': [], 'validation': [], 'samples': [], 'failures': []}
def save():
    OUTPUT.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
def command(compiler, source, overlay, output, define=None):
    if compiler == 'msvc':
        cmd = [str(paths[compiler]), '/nologo', '/std:c++20', '/EHsc', '/O2', '/W4', '/WX', '/wd4146', '/D_CRT_SECURE_NO_WARNINGS', '/I' + str(overlay), '/I' + str(ROOT)]
        if define:
            cmd.append('/D' + define)
            cmd.append('/DNDEBUG')
        return cmd + [str(source), '/Fo' + str(output.with_suffix('.obj')), '/Fe' + str(output)]
    cmd = [str(paths[compiler]), '--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std=gnu++20', '-O2', '-Wall', '-Wextra', '-Wshadow', '-Werror', '-Wno-constant-conversion', '-U__SIZEOF_INT128__', '-D_CRT_SECURE_NO_WARNINGS', '-I' + str(overlay), '-I' + str(ROOT)]
    if define:
        cmd.append('-D' + define)
        cmd.append('-DNDEBUG')
    return cmd + [str(source), '-o', str(output)]
def build(compiler, variant, purpose, source, define=None):
    overlay = BUILD / variant
    exe = BUILD / f'{compiler}-{variant}-{purpose}.exe'
    cmd = command(compiler, source, overlay, exe, define)
    p = subprocess.run(cmd, cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
    result['builds'].append({'compiler': compiler, 'variant': variant, 'purpose': purpose, 'command': cmd, 'exit': p.returncode, 'output': p.stdout, 'exe_sha256': sha(exe) if not p.returncode else None})
    save()
    if p.returncode:
        raise RuntimeError(f'{compiler} {variant} {purpose} compilation failed')
    return exe

env, paths = windows_env()
save()
try:
    for variant, header in [('baseline', BASE), ('candidate', CANDIDATE)]:
        dest = BUILD / variant / 'blueberry/utility/fast-io.hpp'
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(header, dest)
        if sha(dest) != sha(header):
            raise ValueError('overlay copy mismatch')
    for compiler in COMPILERS:
        for variant in VARIANTS:
            exe = build(compiler, variant, 'correctness', TEST)
            for seed in SEEDS:
                p = subprocess.run([str(exe), str(seed)], cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
                result['validation'].append({'compiler': compiler, 'variant': variant, 'seed': seed, 'exit': p.returncode, 'output': p.stdout})
                save()
                if p.returncode:
                    raise RuntimeError(f'{compiler} {variant} correctness seed {seed} failed')
                print('validated', compiler, variant, seed, flush=True)
    bins = {}
    for compiler in COMPILERS:
        for variant in VARIANTS:
            bins[(compiler, variant)] = build(compiler, variant, 'csr', DRIVER, 'CSR_GRAPH')
    for compiler in COMPILERS:
        for shape in SHAPES:
            path = INPUTS / manifest['shapes'][shape]['path']
            if sha(path) != manifest['shapes'][shape]['sha256']:
                raise ValueError(f'{shape} input hash changed')
            expected = next(x['output_sha256'] for x in reference['samples'] if x['shape'] == shape and x['variant'] == 'buffered-csr')
            for repeat in range(-1, ROUNDS):
                order = VARIANTS if repeat % 2 == 0 else tuple(reversed(VARIANTS))
                for variant in order:
                    out_path = BUILD / f'{shape}-{compiler}-{variant}.out'
                    begin = time.perf_counter()
                    with path.open('rb') as data, out_path.open('wb') as out:
                        p = subprocess.run([str(bins[(compiler, variant)])], stdin=data, stdout=out, stderr=subprocess.PIPE)
                    wall = (time.perf_counter() - begin) * 1000
                    if p.returncode:
                        raise RuntimeError(f'{compiler} {variant} {shape}: {p.returncode} {p.stderr.decode(errors="replace")}')
                    metrics = json.loads(p.stderr.decode('ascii'))
                    output_hash = sha(out_path)
                    if output_hash != expected:
                        raise ValueError(f'{compiler} {variant} {shape}: output differs from independent-oracle-checked reference')
                    if repeat >= 0:
                        result['samples'].append({'compiler': compiler, 'variant': variant, 'shape': shape, 'round': repeat, 'wall_ms': wall, 'output_sha256': output_hash, **metrics})
                    save()
            print('measured', compiler, shape, flush=True)
    result['summary'] = {}
    for compiler in COMPILERS:
        for shape in SHAPES:
            for variant in VARIANTS:
                rows = [r for r in result['samples'] if r['compiler'] == compiler and r['shape'] == shape and r['variant'] == variant]
                fields = ('wall_ms', 'parse_ms', 'build_ms', 'parse_build_ms', 'search_ms', 'output_ms', 'peak_working_set_bytes')
                result['summary'][f'{compiler}/{shape}/{variant}'] = {key: statistics.median(r[key] for r in rows) for key in fields}
    result['status'] = 'passed'
except Exception as e:
    result['status'] = 'failed'
    result['failures'].append({'type': type(e).__name__, 'message': str(e)})
    save()
    raise
save()
print(json.dumps(result['summary'], indent=2))
