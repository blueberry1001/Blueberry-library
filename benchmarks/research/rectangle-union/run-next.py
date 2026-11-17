import ctypes, hashlib, json, os, platform, statistics, subprocess, sys
from pathlib import Path
root=Path.cwd(); work=root/'benchmarks/research/rectangle-union'
vc=Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=Path(r'C:\Program Files (x86)\Windows Kits\10'); version='10.0.22621.0'
compiler=Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy(); env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH']
env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
exe=work/'bench-next.exe'; cmd=[str(compiler),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG',str(work/'driver-next.cpp'),'-lpsapi','-o',str(exe)]
if '--build' in sys.argv:
  print(subprocess.check_output([str(compiler),'--version'],text=True).splitlines()[0],flush=True)
  print(subprocess.list2cmdline(cmd),flush=True)
  subprocess.run(cmd,env=env,check=True)
  subprocess.run([str(exe),'10','0','100','1'],check=True)
  sys.exit()
kernel=ctypes.windll.kernel32;kernel.GetCurrentProcess.restype=ctypes.c_void_p
kernel.GetProcessAffinityMask.argtypes=[ctypes.c_void_p,ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t)];kernel.SetProcessAffinityMask.argtypes=[ctypes.c_void_p,ctypes.c_size_t]
proc=kernel.GetCurrentProcess();mask=ctypes.c_size_t();sysmask=ctypes.c_size_t()
if not kernel.GetProcessAffinityMask(proc,ctypes.byref(mask),ctypes.byref(sysmask)): raise RuntimeError('GetProcessAffinityMask failed')
aff=mask.value & -mask.value
if not kernel.SetProcessAffinityMask(proc,aff): raise RuntimeError('SetProcessAffinityMask failed')
print('affinity',hex(aff),'platform',platform.platform(),'processor',platform.processor(),flush=True)
results=[]
for kind,n,repeat in [(0,100000,2),(0,500000,1),(1,100000,3),(2,100000,2),(3,100000,2),(4,100000,5),(5,100000,4),(0,50,5000),(0,8,50000),(0,3,100000),(0,2,100000),(0,1,200000),(0,0,200000)]:
  samples={v:[] for v in range(10)};hashes=set()
  for sample in range(10):
    for v in (list(range(10)) if sample%2==0 else list(range(9,-1,-1))):
      raw=subprocess.check_output([str(exe),str(v),str(kind),str(n),str(repeat)],text=True).split()
      ms=float(raw[0]);hashes.add((raw[1],raw[2]));row={'kind':kind,'n':n,'repeat':repeat,'sample':sample,'variant':v,'ms':ms,'hash':raw[1],'area':raw[2],'peak_bytes':int(raw[3])};results.append(row)
      if sample:samples[v].append(ms)
  if len(hashes)!=1: raise RuntimeError('checksum mismatch')
  print(kind,n,repeat,'medians',[statistics.median(samples[v]) for v in range(10)],'ranges',[(min(samples[v]),max(samples[v])) for v in range(10)],flush=True)
(work/'results-next-local.json').write_text(json.dumps({'command':cmd,'affinity':hex(aff),'hashes':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in work.glob('*.hpp')},'results':results},indent=2),encoding='utf-8')
