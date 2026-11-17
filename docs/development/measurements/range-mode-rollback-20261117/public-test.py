import json
import os
import pathlib
import subprocess

root = pathlib.Path.cwd()
out = root / 'docs/development/measurements/range-mode-rollback-20261117'
build = root / '.build/range-mode-rollback-20261117'
vc = pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); version = '10.0.22621.0'
clang = pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy(); env['PATH'] = str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH']
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
results = []
def run(command):
  p = subprocess.run(command, env=env, capture_output=True, text=True, errors='replace', timeout=180)
  results.append({'command':list(map(str,command)), 'exit':p.returncode, 'stdout':p.stdout, 'stderr':p.stderr})
  (out/'public-test.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf-8')
  if p.returncode: raise RuntimeError('test failed')
for standard in ('gnu++20','gnu++23'):
  for release in (False,True):
    exe = build/(standard+('-release' if release else '-assert')+'.exe')
    command = [str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std='+standard,'-O2','-Wall','-Wextra','-Wshadow','-Werror',*(['-DNDEBUG'] if release else []),'-iquote',str(root),'-isystem',str(root/'.deps/ac-library'),str(root/'tests/random/sequence-batch-four.cpp'),'-o',str(exe)]
    run(command)
    for seed in ('1','17','731'): run([str(exe),seed])
