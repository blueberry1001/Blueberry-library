import os, pathlib, subprocess
root=pathlib.Path.cwd(); build=root/'.build/hld-path-study'
source=(build/'baseline/blueberry/graph/heavy-light-decomposition.hpp').read_text(encoding='utf-8')
old='''    if (depth_[head_[u]] >= depth_[head_[v]]) {
      invoke(f, in_[head_[u]], in_[u] + 1, true);
      path_query_impl(parent_[head_[u]], v, f, edge);
    } else {
      const int head = head_[v];
      path_query_impl(u, parent_[head], f, edge);
      invoke(f, in_[head], in_[v] + 1, false);
    }'''
new='''    while (depth_[head_[u]] >= depth_[head_[v]]) {
      invoke(f, in_[head_[u]], in_[u] + 1, true);
      u = parent_[head_[u]];
      if (head_[u] == head_[v]) {
        if (in_[u] <= in_[v]) {
          const int left = in_[u] + static_cast<int>(edge);
          if (left <= in_[v]) invoke(f, left, in_[v] + 1, false);
        } else {
          const int left = in_[v] + static_cast<int>(edge);
          if (left <= in_[u]) invoke(f, left, in_[u] + 1, true);
        }
        return;
      }
    }
    const int head = head_[v];
    path_query_impl(u, parent_[head], f, edge);
    invoke(f, in_[head], in_[v] + 1, false);'''
assert source.count(old)==1
dest=build/'candidate2/blueberry/graph'; dest.mkdir(parents=True,exist_ok=True)
(dest/'heavy-light-decomposition.hpp').write_text(source.replace(old,new),encoding='utf-8',newline='\n')
vc=pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk=pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10'); ver='10.0.22621.0'
clang=pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env=os.environ.copy(); env['PATH']=str(vc/'bin/Hostx64/x64')+os.pathsep+env.get('PATH','')
env['INCLUDE']=os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/ver/p for p in ('ucrt','shared','um'))]))
env['LIB']=os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/ver/p/'x64' for p in ('ucrt','um'))]))
for compiler in ('msvc','clang'):
 for test,src in [('driver',build/'driver.cpp'),('random',root/'tests/tree_random.cpp')]:
  out=build/'candidate2'/(compiler+'-'+test+'.exe')
  if compiler=='msvc': cmd=[str(vc/'bin/Hostx64/x64/cl.exe'),'/nologo','/std:c++20','/EHsc','/O2',*(['/DNDEBUG'] if test=='driver' else []),'/I'+str(build/'candidate2'),'/I'+str(root),str(src),'/Fo'+str(out.with_suffix('.obj')),'/Fe'+str(out)]
  else: cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2',*(['-DNDEBUG'] if test=='driver' else []),'-I'+str(build/'candidate2'),'-I'+str(root),str(src),'-o',str(out)]
  p=subprocess.run(cmd,cwd=build,env=env,capture_output=True,text=True,errors='replace')
  with (build/'compile-v2.log').open('a',encoding='utf-8') as f: f.write(str(cmd)+'\n'+p.stdout+p.stderr+'exit '+str(p.returncode)+'\n')
  if p.returncode: raise RuntimeError('compile failed')
  if test=='random': subprocess.run([str(out)],cwd=build,env=env,check=True)
