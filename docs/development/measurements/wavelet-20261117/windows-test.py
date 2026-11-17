import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess

root=Path.cwd()
record=root/'docs/development/measurements/wavelet-20261117'
build=root/'.build/wavelet-investigation-test'
build.mkdir(parents=True,exist_ok=True)
vc=Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=Path(r'C:\Program Files (x86)\Windows Kits\10'); version='10.0.22621.0'
msvc=vc/'bin/Hostx64/x64/cl.exe'
clang=Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy()
env['PATH']=str(msvc.parent)+os.pathsep+env.get('PATH','')
env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
src=root/'tests/random/wavelet-matrix.cpp'; header=root/'blueberry/data-structure/wavelet-matrix.hpp'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out={'status':'running','platform':platform.platform(),'source_sha256':sha(src),'header_sha256':sha(header),'rows':[]}
def save(): (record/'windows-test-results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for compiler,standard in [('msvc','c++20'),('msvc','c++latest'),('clang','gnu++20'),('clang','gnu++23')]:
    name=compiler+'-'+standard.replace('+','p'); exe=build/(name+'.exe')
    if compiler=='msvc':
      cmd=[str(msvc),'/nologo','/std:'+standard,'/EHsc','/O2','/I'+str(root),str(src),'/Fo'+str(build/(name+'.obj')),'/Fe'+str(exe)]
    else:
      cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std='+standard,'-O2','-Wall','-Wextra','-Wshadow','-Werror','-I'+str(root),str(src),'-o',str(exe)]
    p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
    row={'compiler':compiler,'standard':standard,'command':cmd,'compile_exit':p.returncode,'compile_output':p.stdout+p.stderr,'runs':[]}; out['rows'].append(row); save()
    if p.returncode: raise RuntimeError('compile failed: '+name)
    for seed in [1,2,17]:
      p=subprocess.run([str(exe),str(seed)],env=env,capture_output=True,text=True,errors='replace')
      row['runs'].append({'seed':seed,'exit':p.returncode,'output':p.stdout+p.stderr});save()
      if p.returncode: raise RuntimeError('test failed: '+name+' seed '+str(seed))
    print(name,'passed',flush=True)
  out['status']='complete'
except BaseException as exc:
  out['status']='failed';out['error']=repr(exc);save();raise
save()
