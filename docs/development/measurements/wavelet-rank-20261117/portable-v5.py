import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess

root=Path.cwd(); here=root/'.build/wavelet-rank-study'; build=here/'portable'; build.mkdir(parents=True,exist_ok=True)
vc=Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=Path(r'C:\Program Files (x86)\Windows Kits\10'); version='10.0.22621.0'
msvc=vc/'bin/Hostx64/x64/cl.exe'
clang=Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy();env['PATH']=str(msvc.parent)+os.pathsep+env.get('PATH','')
env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
inputs={name:here/name/'wavelet-matrix.hpp' for name in ['current','hybrid','hybrid-eq']}
compilers={'msvc':(msvc,['/nologo','/std:c++20','/EHsc','/O2','/DNDEBUG']),'clang':(clang,['--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG'])}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out={'status':'running','platform':platform.platform(),'cpu':platform.processor(),'n':200000,'q':100000,'warmups':1,'runs':5,'source_hashes':{k:sha(v) for k,v in inputs.items()},'driver_hash':sha(here/'portable-driver.cpp'),'samples':[],'compilers':{k:{'path':str(v[0]),'flags':v[1]} for k,v in compilers.items()},'notes':'Not CPU-pinned; other sessions may be active. Peak working set includes input, queries, build temporaries, runtime and binary.'}
def save(): (here/'portable-results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for name,path in inputs.items():
    folder=build/name;folder.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,folder/'wavelet-matrix.hpp')
    for comp,(compiler,flags) in compilers.items():
      obj=folder/(comp+'.obj');exe=folder/(comp+'.exe')
      if comp=='msvc': cmd=[str(compiler),*flags,'/I'+str(folder),str(here/'portable-driver.cpp'),'/Fo'+str(obj),'/Fe'+str(exe)]
      else: cmd=[str(compiler),*flags,'-I'+str(folder),str(here/'portable-driver.cpp'),'-o',str(exe)]
      p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
      if p.returncode: raise RuntimeError(str((cmd,p.stdout,p.stderr)))
  for comp in compilers:
    for shape in ['full','duplicates','narrow','sorted','extreme','biased','dense','sparse-dense','sparse-256','sparse-4096','min-dense']:
      for iteration in range(-1,5):
        names=list(inputs);shift=(iteration+1)%len(names);checksum=None
        for name in names[shift:]+names[:shift]:
          p=subprocess.run([str(build/name/(comp+'.exe')),'200000','100000',shape],env=env,capture_output=True,text=True,errors='replace')
          if p.returncode: raise RuntimeError(str((comp,name,shape,p.stdout,p.stderr)))
          f=p.stdout.split()
          if checksum is None: checksum=f[0]
          if checksum!=f[0]: raise RuntimeError(str((comp,name,shape,checksum,f)))
          if iteration>=0: out['samples'].append({'compiler':comp,'shape':shape,'iteration':iteration,'variant':name,'checksum':checksum,'build_ms':float(f[1]),'kth_ms':float(f[2]),'range_ms':float(f[3]),'equal_ms':float(f[4]),'narrow_ms':float(f[5]),'peak_working_set':int(f[6])})
      save()
      for name in inputs:
        rows=[x for x in out['samples'] if x['compiler']==comp and x['shape']==shape and x['variant']==name]
        print(comp,shape,name,*[round(statistics.median(x[k] for x in rows),3) for k in ('build_ms','kth_ms','range_ms','equal_ms','narrow_ms','peak_working_set')],flush=True)
  out['status']='complete'
except BaseException as exc:
  out['status']='failed';out['error']=repr(exc);save();raise
save()
