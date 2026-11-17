import ctypes, hashlib, json, os, pathlib, platform, subprocess, time
root=pathlib.Path.cwd(); src=root/'docs/development/measurements/bm-20261117'; build=root/'.build/bm-final'; build.mkdir(parents=True,exist_ok=True)
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); ver='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy(); env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH']; env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/ver/p for p in ('ucrt','shared','um'))])); env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/ver/p/'x64' for p in ('ucrt','um'))]))
k=ctypes.WinDLL('kernel32',use_last_error=True); k.GetCurrentProcess.restype=ctypes.c_void_p; k.GetProcessAffinityMask.argtypes=[ctypes.c_void_p,ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t)]; k.SetProcessAffinityMask.argtypes=[ctypes.c_void_p,ctypes.c_size_t]
proc=k.GetCurrentProcess(); old=ctypes.c_size_t(); system=ctypes.c_size_t(); assert k.GetProcessAffinityMask(proc,ctypes.byref(old),ctypes.byref(system)); mask=old.value & -old.value; assert k.SetProcessAffinityMask(proc,mask)
data={'platform':platform.platform(),'processor':platform.processor(),'affinity':hex(mask),'runs':5,'warmups':1,'builds':[],'counts':[],'samples':[],'status':'running','hashes':{}}
for f in [src/'baseline.hpp.txt',src/'candidate.hpp.txt',src/'driver.cpp',src/'run.py',root/'blueberry/math/linear-recurrence.hpp']: data['hashes'][str(f.relative_to(root))]=hashlib.sha256(f.read_bytes()).hexdigest()
def save(): (src/'results.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
save()
try:
 for mode in ('assert','release'):
  exe=build/(mode+'.exe'); cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-Wall','-Wextra','-Wshadow','-Werror',*(['-DNDEBUG'] if mode=='release' else []),'-iquote',str(root),'-isystem',str(root/'.deps/ac-library'),str(src/'driver.cpp'),'-o',str(exe)]
  p=subprocess.run(cmd,env=env,cwd=build,capture_output=True,text=True,errors='replace'); data['builds'].append({'cmd':cmd,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr}); save()
  if p.returncode: raise RuntimeError('build failed')
  if mode=='assert':
   p=subprocess.run([str(exe)],env=env,cwd=build,capture_output=True,text=True,errors='replace',timeout=120); data['builds'].append({'run':mode,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr}); save()
   if p.returncode: raise RuntimeError('correctness failed')
 exe=build/'release.exe'; data['hashes']['release.exe']=hashlib.sha256(exe.read_bytes()).hexdigest()
 for n,reps in [(16,10000),(128,1000),(1024,40),(4096,4),(10000,1)]:
  for kind in range(5):
   for variant in range(3):
    cmd=[str(exe),str(variant),str(n),str(kind),'1','count']; p=subprocess.run(cmd,env=env,cwd=build,capture_output=True,text=True,errors='replace',timeout=120); data['counts'].append({'n':n,'kind':kind,'variant':variant,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr}); save()
    if p.returncode: raise RuntimeError('count failed')
   expected=None
   for sample in range(-1,5):
    for variant in ([0,1,2] if sample%2==0 else [2,1,0]):
     cmd=[str(exe),str(variant),str(n),str(kind),str(reps)]; start=time.perf_counter(); p=subprocess.Popen(cmd,env=env,cwd=build,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True); child=ctypes.c_size_t(); sy=ctypes.c_size_t(); ok=bool(k.GetProcessAffinityMask(ctypes.c_void_p(p._handle),ctypes.byref(child),ctypes.byref(sy))); stdout,stderr=p.communicate(timeout=180)
     row={'n':n,'reps':reps,'kind':kind,'sample':sample,'variant':variant,'exit':p.returncode,'stdout':stdout,'stderr':stderr,'wall_seconds':time.perf_counter()-start,'child_mask':hex(child.value) if ok else None};data['samples'].append(row);save()
     if p.returncode or not ok or child.value!=mask: raise RuntimeError('run or affinity failed')
     result=stdout.split()[1]
     if expected is None: expected=result
     if result!=expected: raise RuntimeError('result mismatch')
 data['status']='complete';save()
except BaseException as exc:
 data['status']='failed';data['error']=repr(exc);save();raise
