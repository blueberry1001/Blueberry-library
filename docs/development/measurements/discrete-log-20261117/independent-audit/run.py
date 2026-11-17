import os,pathlib,subprocess,json,traceback
p=pathlib.Path(__file__).resolve().parent
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); v='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
acl=p.parent/'speed-snapshot-20261117'/'.deps'/'ac-library'
env=os.environ.copy();env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH'];env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/v/x for x in ('ucrt','shared','um'))]));env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/v/x/'x64' for x in ('ucrt','um'))]))
cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2','-I'+str(acl),str(p/'test.cpp'),'-o',str(p/'test.exe')]
out=[]
try:
 for c in (cmd,[str(p/'test.exe')]):
  r=subprocess.run(c,cwd=p,env=env,capture_output=True,text=True,errors='replace',timeout=180);out.append({'cmd':c,'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr});(p/'result.json').write_text(json.dumps(out,indent=2))
  if r.returncode:raise RuntimeError('failed')
except BaseException:
 (p/'error.log').write_text(traceback.format_exc());raise
