import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

root = Path.cwd()
here = root / 'docs/development/measurements/polynomial-product-skew-20261117'
work = root / '.build/polynomial-product-skew-20261117'
work.mkdir(parents=True, exist_ok=True)
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
msvc = vc / 'bin/Hostx64/x64/cl.exe'
env = os.environ.copy()
env['PATH'] = str(vc / 'bin/Hostx64/x64') + os.pathsep + env['PATH']
env['INCLUDE'] = os.pathsep.join(map(str, [vc / 'include', *(sdk / 'Include' / version / p for p in ('ucrt', 'shared', 'um'))]))
env['LIB'] = os.pathsep.join(map(str, [vc / 'lib/x64', *(sdk / 'Lib' / version / p / 'x64' for p in ('ucrt', 'um'))]))
sources = ['blueberry/math/polynomial-product.hpp', 'tests/random/polynomial-product.cpp']
data = {'source_sha256':{s:hashlib.sha256((root / s).read_bytes()).hexdigest() for s in sources}, 'commands':[]}
output = here / 'validation.json'
def execute(cmd):
    p = subprocess.run(list(map(str, cmd)), cwd=work, env=env, capture_output=True, text=True)
    data['commands'].append({'command':list(map(str, cmd)), 'exit_code':p.returncode, 'stdout':p.stdout, 'stderr':p.stderr})
    output.write_text(json.dumps(data, indent=2)+'\n', encoding='utf-8')
    if p.returncode:
        print(p.stdout, p.stderr, flush=True)
        sys.exit(p.returncode or 1)
source = root / 'tests/random/polynomial-product.cpp'
for compiler, std in [('clang', 'gnu++20'), ('clang', 'gnu++23'), ('msvc', 'c++20')]:
    for release in (False, True):
        name = compiler + '-' + std + ('-release' if release else '-debug')
        exe = work / (name + '.exe')
        if compiler == 'clang':
            cmd = [clang, '--target=x86_64-pc-windows-msvc', '-fuse-ld=link', '-std='+std, '-O2', '-Wall', '-Wextra', '-Wshadow', '-Werror', '-isystem', root, source, '-o', exe]
            if release: cmd.insert(5, '-DNDEBUG')
        else:
            cmd = [msvc, '/nologo', '/std:'+std, '/EHsc', '/O2', '/I'+str(root), '/Fe:'+str(exe), '/Fo:'+str(work/(name+'.obj')), source]
            if release: cmd.insert(5, '/DNDEBUG')
        execute(cmd)
        for seed in (1, 17, 731): execute([exe, str(seed)])
        print(name, 'passed', flush=True)
