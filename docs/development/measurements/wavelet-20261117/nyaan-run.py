import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess

root=Path.cwd(); record=root/'docs/development/measurements/wavelet-20261117'
build=root/'.build/wavelet-nyaan-20261117'; build.mkdir(parents=True,exist_ok=True)
vc=Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=Path(r'C:\Program Files (x86)\Windows Kits\10'); version='10.0.22621.0'
link=vc/'bin/Hostx64/x64'
clang=Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy();env['PATH']=str(link)+os.pathsep+env.get('PATH','')
env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
shutil.copyfile(record/'nyaan-driver.cpp.txt',build/'driver.cpp')
shutil.copyfile(record/'nyaan-raw.hpp.txt',build/'nyaan.hpp')
inputs={'baseline':record/'baseline.hpp.txt','current':record/'final-prerank.hpp.txt','nyaan':record/'nyaan-raw.hpp.txt'}
flags=['--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG','-mbmi2','-mpopcnt','-Wno-unknown-attributes']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out={'status':'running','platform':platform.platform(),'cpu':platform.processor(),'compiler':str(clang),'flags':flags,'n':200000,'q':100000,'warmups':1,'runs':5,'sha256':{k:sha(v) for k,v in inputs.items()},'samples':[]}
def save(): (record/'nyaan-results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for name,path in inputs.items():
    folder=build/name;folder.mkdir(parents=True,exist_ok=True)
    if name!='nyaan': shutil.copyfile(path,folder/'wavelet-matrix.hpp')
    cmd=[str(clang),*flags,'-I'+str(folder),'-I'+str(build)]
    if name=='nyaan': cmd+=['-DNYAAN']
    cmd += [str(build/'driver.cpp'),'-o',str(folder/'driver.exe')]
    p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
    if p.returncode: raise RuntimeError(str((cmd,p.stdout,p.stderr)))
  for shape in ['random','duplicates','sorted']:
    for iteration in range(-1,5):
      names=list(inputs);offset=(iteration+1)%len(names);checksum=None
      for name in names[offset:]+names[:offset]:
        p=subprocess.run([str(build/name/'driver.exe'),'200000','100000',shape],capture_output=True,text=True,errors='replace')
        if p.returncode: raise RuntimeError(str((name,shape,p.stdout,p.stderr)))
        fields=p.stdout.split()
        if checksum is None: checksum=fields[0]
        if checksum != fields[0]: raise RuntimeError(str((name,shape,checksum,fields)))
        if iteration>=0:
          out['samples'].append({'shape':shape,'iteration':iteration,'variant':name,'checksum':checksum,'build_ms':float(fields[1]),'kth_ms':float(fields[2]),'range_ms':float(fields[3]),'narrow_ms':float(fields[4])})
    save()
    for name in inputs:
      rows=[x for x in out['samples'] if x['shape']==shape and x['variant']==name]
      print(shape,name,*[round(statistics.median(x[k] for x in rows),3) for k in ('build_ms','kth_ms','range_ms','narrow_ms')],flush=True)
  out['status']='complete'
except BaseException as exc:
  out['status']='failed';out['error']=repr(exc);save();raise
save()
