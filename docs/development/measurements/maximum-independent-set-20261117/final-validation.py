import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess

here = Path(__file__).resolve().parent
build = here/'build'/'final-validation'
build.mkdir(parents=True,exist_ok=True)
shutil.copyfile(here/'final.hpp', build/'candidate.hpp')
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
msvc = vc/'bin/Hostx64/x64/cl.exe'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH','')
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
sha = lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out = {'status':'running','platform':platform.platform(),'header_sha256':sha(here/'final.hpp'),'test_sha256':sha(here/'test.cpp'),'windows_test_sha256':sha(here/'windows-include-test.cpp'),'instrumented_sha256':sha(here/'instrumented.hpp'),'fallback_test_sha256':sha(here/'fallback-test.cpp'),'cases':[]}
def save(): (here/'validation-final.json').write_text(json.dumps(out,indent=2)+'\n',encoding='utf-8')
try:
    for compiler,standard in [('msvc','c++20'),('msvc','c++latest'),('clang','gnu++20'),('clang','gnu++23')]:
        for filename in ['test.cpp','windows-include-test.cpp','fallback-test.cpp']:
            name = compiler+'-'+standard.replace('+','p')+'-'+filename.removesuffix('.cpp')
            exe = build/(name+'.exe')
            if compiler == 'msvc':
                cmd = [str(msvc),'/nologo','/std:'+standard,'/EHsc','/O2','/W3','/WX','/I'+str(build),'/I'+str(here),str(here/filename),'/Fo'+str(build/(name+'.obj')),'/Fe'+str(exe)]
            else:
                cmd = [str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std='+standard,'-O2','-Wall','-Wextra','-Wshadow','-Werror','-I'+str(build),'-I'+str(here),str(here/filename),'-o',str(exe)]
            p = subprocess.run(cmd, cwd=build, env=env, capture_output=True, text=True, errors='replace')
            row = {'compiler':compiler,'standard':standard,'source':filename,'command':cmd,'compile_exit':p.returncode,'compile_output':p.stdout+p.stderr,'runs':[]}
            out['cases'].append(row);save()
            if p.returncode: raise RuntimeError(f'compile failed: {name}')
            for seed in ((1,17,20261117) if filename == 'test.cpp' else (None,)):
                args = [str(exe)] + ([] if seed is None else [str(seed)])
                p = subprocess.run(args, cwd=build, env=env, capture_output=True, text=True, errors='replace',timeout=120)
                row['runs'].append({'args':args,'seed':seed,'exit':p.returncode,'output':p.stdout+p.stderr});save()
                if p.returncode: raise RuntimeError(f'test failed: {name} {seed}')
            print(name,'passed',flush=True)
    out['status'] = 'complete'
except BaseException as exc:
    out['status']='failed';out['error']=repr(exc);save();raise
save()
