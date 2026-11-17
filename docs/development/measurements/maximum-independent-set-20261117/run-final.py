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
variants = ['baseline', 'packed', 'final', 'nyaan']
compilers = ['msvc', 'clang']
digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
out = {'status':'running','system':platform.platform(),'processor':platform.processor(),'note':'Unpinned; other sessions may run concurrently. Timing includes construction of solver adjacency and result, excludes input generation and result validation. Nyaan includes conversion to adjacency lists; all variants must have the same answer size. Peak working set is process-wide.', 'runs':5,'warmups':1,'sources':{v:digest(here / (v + '.hpp')) for v in variants},'driver_sha256':digest(here/'bench.cpp'),'test_sha256':digest(here/'test.cpp'),'tests':[],'builds':[],'samples':[]}
result_path = here / 'results-final.json'
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
    for compiler in compilers:
        for variant in ['final']:
            exe = compile_one(compiler, variant, here/'test.cpp', False)
            for seed in (1, 17, 20261117):
                proc = subprocess.run([str(exe),str(seed)], env=env, capture_output=True, text=True, errors='replace', timeout=120)
                out['tests'].append({'compiler':compiler,'variant':variant,'seed':seed,'exit':proc.returncode,'output':proc.stdout+proc.stderr})
                save()
                if proc.returncode: raise RuntimeError(f'test failed {compiler} {variant} {seed}')
    exes = {(compiler, variant):compile_one(compiler,variant,here/'bench.cpp',True) for compiler in compilers for variant in variants}
    for compiler in compilers:
        for n, count, shapes in [(20,50,range(12)),(30,12,range(12)),(40,4,range(12)),(40,200,(2,4)),(40,40,(7,)),(40,8,(11,))]:
            for shape in shapes:
                for rep in range(-1,5):
                    shift = (rep+1) % len(variants)
                    checksum = None
                    for variant in variants[shift:] + variants[:shift]:
                        cmd = [str(exes[compiler,variant]),str(n),str(shape),str(count)]
                        try:
                            proc = subprocess.run(cmd, env=env, capture_output=True, text=True, errors='replace', timeout=60)
                        except subprocess.TimeoutExpired as exc:
                            out['samples'].append({'compiler':compiler,'variant':variant,'n':n,'count':count,'shape':shape,'rep':rep,'status':'timeout','timeout':60})
                            save()
                            raise RuntimeError(f'timed out {compiler} {variant} {n} {shape}') from exc
                        if proc.returncode: raise RuntimeError(f'run failed: {cmd} {proc.returncode} {proc.stdout} {proc.stderr}')
                        fields = proc.stdout.split()
                        if len(fields) != 3: raise RuntimeError(f'bad output: {cmd}: {proc.stdout}')
                        if checksum is None: checksum = fields[0]
                        if fields[0] != checksum: raise RuntimeError(f'wrong checksum {compiler} {variant} {n} {shape} {fields[0]} {checksum}')
                        out['samples'].append({'compiler':compiler,'variant':variant,'n':n,'count':count,'shape':shape,'rep':rep,'checksum':fields[0],'milliseconds':float(fields[1]),'peak_bytes':int(fields[2])})
                    save()
                rows = [x for x in out['samples'] if x['compiler']==compiler and x['n']==n and x['count']==count and x['shape']==shape and x['rep']>=0]
                print(compiler,n,count,shape,*[f"{v}={statistics.median(x['milliseconds'] for x in rows if x['variant']==v):.3f}" for v in variants],flush=True)
    out['status'] = 'complete'
except BaseException as exc:
    out['status']='failed';out['error']=repr(exc);save();raise
save()
