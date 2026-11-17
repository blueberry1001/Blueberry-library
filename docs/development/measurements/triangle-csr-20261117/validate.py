import hashlib, json, os, pathlib, subprocess
root=pathlib.Path.cwd(); src=root/'docs/development/measurements/triangle-csr-20261117'; build=root/'.build/triangle-csr-study'; build.mkdir(parents=True,exist_ok=True)
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808'); sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); ver='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy(); env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH']; env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/ver/p for p in ('ucrt','shared','um'))])); env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/ver/p/'x64' for p in ('ucrt','um'))]))
test=root/'tests/random/graph-batch-two.cpp'; header=root/'blueberry/graph/triangle-enumeration.hpp'
data={'hashes':{str(x.relative_to(root)):hashlib.sha256(x.read_bytes()).hexdigest() for x in (test,header)},'results':[],'status':'running'}
def save(): (src/'validation.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
save()
try:
 for mode in ('gnu++20','gnu++23','msvc20'):
  exe=build/(mode+'.exe')
  if mode=='msvc20': cmd=[str(vc/'bin/Hostx64/x64/cl.exe'),'/nologo','/EHsc','/std:c++20','/O2','/I'+str(root),str(test),'/Fe:'+str(exe)]
  else: cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std='+mode,'-O2','-I'+str(root),str(test),'-o',str(exe)]
  p=subprocess.run(cmd,env=env,cwd=build,capture_output=True,text=True,errors='replace'); data['results'].append({'mode':mode,'cmd':cmd,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr}); save()
  if p.returncode: raise RuntimeError('build failed')
  p=subprocess.run([str(exe)],env=env,cwd=build,capture_output=True,text=True,errors='replace',timeout=120); log=src/(mode+'.log'); log.write_text(p.stdout+p.stderr,encoding='utf-8'); data['results'].append({'mode':mode,'run':str(exe),'exit':p.returncode,'log':log.name,'log_sha256':hashlib.sha256(log.read_bytes()).hexdigest()}); save()
  if p.returncode: raise RuntimeError('test failed')
 data['status']='complete';save()
except BaseException as exc:
 data['status']='failed';data['error']=repr(exc);save();raise
