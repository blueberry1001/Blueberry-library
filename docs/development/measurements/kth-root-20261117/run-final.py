import ctypes
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess

here = Path(__file__).resolve().parent
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
msvc = vc / 'bin/Hostx64/x64/cl.exe'
clang = Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(msvc.parent) + os.pathsep + env.get('PATH', '')
env['INCLUDE'] = os.pathsep.join(map(str, [vc/'include', *(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str, [vc/'lib/x64', *(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
def cpu_times():
  idle=ctypes.c_ulonglong();kernel=ctypes.c_ulonglong();user=ctypes.c_ulonglong()
  if not ctypes.windll.kernel32.GetSystemTimes(ctypes.byref(idle),ctypes.byref(kernel),ctypes.byref(user)): return None
  return idle.value,kernel.value,user.value
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out={'status':'running','platform':platform.platform(),'count':200000,'warmups':1,'runs':5,'baseline_sha256':sha(here/'baseline.hpp'),'candidate_sha256':sha(here/'candidate.hpp'),'nyaan_sha256':sha(here/'nyaan.hpp'),'driver_sha256':sha(here/'final-benchmark.cpp'),'variants':['baseline','candidate','nyaan'],'warning':'CPU not pinned; other sessions may be active. Windows total CPU includes the measured program.','rows':[]}
def save(): (here/'final-results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for compiler in ['msvc','clang']:
    exe=here/(compiler+'-final.exe')
    if compiler=='msvc': cmd=[str(msvc),'/nologo','/std:c++20','/EHsc','/O2','/DNDEBUG',str(here/'final-benchmark.cpp'),'/Fo'+str(here/'final-msvc.obj'),'/Fe'+str(exe)]
    else: cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG',str(here/'final-benchmark.cpp'),'-o',str(exe)]
    p=subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
    row={'compiler':compiler,'compile':cmd,'compile_exit':p.returncode,'compile_output':p.stdout+p.stderr,'samples':[]};out['rows'].append(row);save()
    if p.returncode: raise RuntimeError('compile failed '+compiler)
    variants=range(2 if compiler=='msvc' else 3)
    for shape in range(11):
      checksums=set()
      for rep in range(-1,5):
        order=list(variants);order=order[rep%len(order):]+order[:rep%len(order)]
        for variant in order:
          before=cpu_times();p=subprocess.run([str(exe),str(variant),str(shape),str(out['count'])],env=env,capture_output=True,text=True,errors='replace',timeout=120);after=cpu_times()
          if p.returncode: raise RuntimeError('run failed '+p.stdout+p.stderr)
          v,s,ms,checksum=p.stdout.strip().split()
          total=None if before is None or after is None else after[1]+after[2]-before[1]-before[2]
          raw=None if not total else 100*(1-(after[0]-before[0])/total)
          load=raw if raw is not None and 0 <= raw <= 100 else None
          row['samples'].append({'shape':shape,'rep':rep,'variant':variant,'ms':float(ms),'checksum':checksum,'system_cpu_pct':load,'system_cpu_raw_pct':raw});checksums.add(checksum)
        if len(checksums)!=1: raise RuntimeError('checksum mismatch '+str(checksums))
        save()
      print(compiler,'shape',shape,'done',flush=True)
  out['status']='complete'
except BaseException as error:
  out['status']='failed';out['error']=repr(error);save();raise
save()
for row in out['rows']:
  print(row['compiler'])
  for shape in range(11):
    variants=range(2 if row['compiler']=='msvc' else 3)
    print(shape,' '.join(f"{statistics.median(s['ms'] for s in row['samples'] if s['rep']>=0 and s['shape']==shape and s['variant']==v):.3f}" for v in variants))
