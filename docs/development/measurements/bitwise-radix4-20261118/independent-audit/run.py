import json,os,pathlib,subprocess,traceback
p=pathlib.Path(__file__).resolve().parent
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808');sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10');v='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy();env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env['PATH'];env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/v/x for x in ('ucrt','shared','um'))]));env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/v/x/'x64' for x in ('ucrt','um'))]))
d={'status':'running','results':[]}
def save(): (p/'result.json').write_text(json.dumps(d,indent=2)+'\n')
def call(cmd):
 r=subprocess.run(cmd,cwd=p,env=env,capture_output=True,text=True,errors='replace',timeout=180);d['results'].append({'cmd':list(map(str,cmd)),'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr});save()
 if r.returncode:raise RuntimeError('command failed')
try:
 for cc in ('clang','msvc'):
  for std in ('20','23'):
   exe=p/('audit-'+cc+std+'.exe');src=p/'test.cpp'
   cmd=([str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++'+std,'-O2','-Wall','-Wextra','-Wshadow','-Werror',str(src),'-o',str(exe)] if cc=='clang' else [str(vc/'bin/Hostx64/x64/cl.exe'),'/nologo','/std:c++'+('latest' if std=='23' else '20'),'/O2','/EHsc',str(src),'/Fe:'+str(exe)])
   call(cmd);call([str(exe)])
 d['status']='complete';save()
except BaseException:
 d['status']='failed';d['traceback']=traceback.format_exc();save();raise
