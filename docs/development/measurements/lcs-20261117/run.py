import ctypes, json, os, pathlib, platform, subprocess, time, traceback
p=pathlib.Path(__file__).resolve().parent
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); v='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
acl=p.parent/'speed-snapshot-20261117' / '.deps' / 'ac-library'
env=os.environ.copy();env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH'];env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/v/x for x in ('ucrt','shared','um'))]));env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/v/x/'x64' for x in ('ucrt','um'))]))
k=ctypes.WinDLL('kernel32',use_last_error=True);k.GetCurrentProcess.restype=ctypes.c_void_p;k.SetProcessAffinityMask.argtypes=[ctypes.c_void_p,ctypes.c_size_t]
assert k.SetProcessAffinityMask(k.GetCurrentProcess(),1<<4)
d={'status':'running','platform':platform.platform(),'processor':platform.processor(),'affinity':'0x10','results':[]}
def save(): (p/'result.json').write_text(json.dumps(d,indent=2)+'\n')
def call(cmd):
  t=time.perf_counter();r=subprocess.run(cmd,cwd=p,env=env,capture_output=True,text=True,errors='replace',timeout=180)
  x={'cmd':list(map(str,cmd)),'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'seconds':time.perf_counter()-t};d['results'].append(x);save()
  if r.returncode:raise RuntimeError('command failed')
  return x
try:
  for compiler in ('clang','msvc'):
    exe=p/(compiler+'.exe')
    cmd=([str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-DNDEBUG','-I'+str(acl),str(p/'compare.cpp'),'-o',str(exe)] if compiler=='clang' else [str(vc/'bin/Hostx64/x64/cl.exe'),'/nologo','/std:c++20','/O2','/DNDEBUG','/EHsc','/I'+str(acl),str(p/'compare.cpp'),'/Fe:'+str(exe)])
    call(cmd);call([str(exe)])
    for kind in range(7):
      for sample in range(-1,7):
        results=[]
        for ver in ([0,1] if sample%2 else [1,0]):
          x=call([str(exe),str(ver),'100000',str(kind),'2']);x.update(compiler=compiler,kind=kind,sample=sample,version=ver);results.append(x['stdout'].split()[1]);save()
        if results[0]!=results[1]: raise RuntimeError('mismatching lengths')
  d['status']='complete';save()
except BaseException:
  d['status']='failed';d['traceback']=traceback.format_exc();save();raise
