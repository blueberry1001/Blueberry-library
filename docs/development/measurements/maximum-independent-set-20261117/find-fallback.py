import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import time

here = Path(__file__).resolve().parent
build = here / 'build'
build.mkdir(exist_ok=True)
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
msvc = vc / 'bin/Hostx64/x64/cl.exe'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH', '')
env['INCLUDE'] = os.pathsep.join(map(str, [vc / 'include', *(sdk / 'Include' / version / p for p in ('ucrt', 'shared', 'um'))]))
env['LIB'] = os.pathsep.join(map(str, [vc / 'lib/x64', *(sdk / 'Lib' / version / p / 'x64' for p in ('ucrt', 'um'))]))
variants = ['baseline', 'components', 'components-budget17', 'components-budget18', 'components-budget20', 'components-popcnt-cached', 'nyaan']
compilers = ['msvc', 'clang']
digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
out = {'status':'running','system':platform.platform(),'processor':platform.processor(),'note':'Unpinned; other sessions may run concurrently. Timing includes construction of solver adjacency and result, excludes input generation and result validation. Nyaan includes conversion to adjacency lists; all variants must have the same answer size. Peak working set is process-wide.', 'runs':5,'warmups':1,'sources':{v:digest(here / (v + '.hpp')) for v in variants},'driver_sha256':digest(here/'bench.cpp'),'test_sha256':digest(here/'test.cpp'),'tests':[],'builds':[],'samples':[]}
result_path = here / 'results-find-fallback.json'
def save(): result_path.write_text(json.dumps(out, indent=2)+'\n', encoding='utf-8')
def compile_one(compiler, variant, src, release):
    folder = build / (compiler + '-' + variant + ('-bench' if release else '-test'))
    folder.mkdir(exist_ok=True)
    if variant != 'nyaan': shutil.copyfile(here / (variant + '.hpp'), folder / 'candidate.hpp')
    exe = folder / 'run.exe'
    if compiler == 'msvc':
        cmd = [str(msvc), '/nologo', '/std:c++20', '/EHsc', '/O2', '/W3', '/I'+str(folder), '/I'+str(here)]
        if release: cmd += ['/DNDEBUG']
        if variant == 'nyaan': cmd += ['/DNYAAN']
        cmd += [str(src), '/Fo'+str(folder/'run.obj'), '/Fe'+str(exe), '/link', 'Psapi.lib']
    else:
        cmd = [str(clang), '--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std=gnu++20', '-O2', '-Wall', '-Wextra', '-I'+str(folder), '-I'+str(here)]
        if release: cmd += ['-DNDEBUG']
        if variant == 'nyaan': cmd += ['-DNYAAN']
        cmd += [str(src), '-o', str(exe), '-lpsapi']
    proc = subprocess.run(cmd, cwd=folder, env=env, capture_output=True, text=True, errors='replace')
    out['builds'].append({'compiler':compiler,'variant':variant,'release':release,'command':cmd,'exit':proc.returncode,'output':proc.stdout+proc.stderr})
    save()
    if proc.returncode: raise RuntimeError(f'compile failed: {compiler} {variant} {src}')
    return exe
try:
    exe = compile_one('clang', 'components-budget18', here/'find-fallback.cpp', False)
    proc = subprocess.run([str(exe)], env=env, capture_output=True, text=True, errors='replace', timeout=120)
    out['tests'].append({'compiler':'clang','exit':proc.returncode,'output':proc.stdout+proc.stderr})
    if proc.returncode: raise RuntimeError('no fallback candidate or failed test')
    print(proc.stdout)
    out['status']='complete'
except BaseException as exc:
    out['status']='failed';out['error']=repr(exc);save();raise
save()
