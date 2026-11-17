import json, os, pathlib, shutil, subprocess
root = pathlib.Path.cwd()
build = root / '.build/hld-path-study'
source = root / 'blueberry/graph/heavy-light-decomposition.hpp'
vc = pathlib.Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = pathlib.Path(r'C:\Program Files (x86)\Windows Kits\10')
ver = '10.0.22621.0'
clang = pathlib.Path(r'C:\Program Files\Unity\Hub\Editor\6000.5.2f1\Editor\Data\PlaybackEngines\AndroidPlayer\NDK\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe')
env = os.environ.copy()
env['PATH'] = str(vc / 'bin/Hostx64/x64') + os.pathsep + env.get('PATH','')
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include',*(sdk/'Include'/ver/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64',*(sdk/'Lib'/ver/p/'x64' for p in ('ucrt','um'))]))
header = source.read_text(encoding='utf-8')
start = header.index('    if (head_[u] == head_[v]) {',header.index('void path_query_impl'))
end = header.index('\n  }\n\n  void check_vertex',start)
candidate = '''    int pending[32], count = 0;
    while (head_[u] != head_[v]) {
      if (depth_[head_[u]] >= depth_[head_[v]]) {
        invoke(f, in_[head_[u]], in_[u] + 1, true);
        u = parent_[head_[u]];
      } else {
        pending[count++] = v;
        v = parent_[head_[v]];
      }
    }
    if (in_[u] <= in_[v]) {
      const int left = in_[u] + static_cast<int>(edge);
      if (left <= in_[v]) invoke(f, left, in_[v] + 1, false);
    } else {
      const int left = in_[v] + static_cast<int>(edge);
      if (left <= in_[u]) invoke(f, left, in_[u] + 1, true);
    }
    while (count) {
      v = pending[--count];
      invoke(f, in_[head_[v]], in_[v] + 1, false);
    }'''
candidate = header[:start] + candidate + header[end:]
records=[]
for variant, content in [('baseline',header),('candidate',candidate)]:
  dest=build/variant/'blueberry/graph'
  dest.mkdir(parents=True,exist_ok=True)
  (dest/'heavy-light-decomposition.hpp').write_text(content,encoding='utf-8',newline='\n')
  for compiler in ['msvc','clang']:
    for test,src in [('driver',build/'driver.cpp'),('random',root/'tests/tree_random.cpp')]:
      out=build/variant/(compiler+'-'+test+'.exe')
      if compiler=='msvc':
        cmd=[str(vc/'bin/Hostx64/x64/cl.exe'),'/nologo','/std:c++20','/EHsc','/O2',*(['/DNDEBUG'] if test=='driver' else []),'/I'+str(build/variant),'/I'+str(root),str(src),'/Fo'+str(out.with_suffix('.obj')),'/Fe'+str(out)]
      else:
        cmd=[str(clang),'--target=x86_64-pc-windows-msvc','-fuse-ld=link','-std=gnu++20','-O2',*(['-DNDEBUG'] if test=='driver' else []),'-I'+str(build/variant),'-I'+str(root),str(src),'-o',str(out)]
      p=subprocess.run(cmd,cwd=build,env=env,capture_output=True,text=True,errors='replace')
      records.append({'variant':variant,'compiler':compiler,'test':test,'cmd':cmd,'exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr})
      (build/'compile.json').write_text(json.dumps(records,indent=2)+'\n',encoding='utf-8')
      if p.returncode: raise RuntimeError('compile failed '+str(cmd))
      if test=='random':
        p=subprocess.run([str(out)],cwd=build,env=env,capture_output=True,text=True,errors='replace')
        records.append({'variant':variant,'compiler':compiler,'test':'random-run','exit':p.returncode,'stdout':p.stdout,'stderr':p.stderr})
        (build/'compile.json').write_text(json.dumps(records,indent=2)+'\n',encoding='utf-8')
        if p.returncode: raise RuntimeError('random failed '+str(out))
