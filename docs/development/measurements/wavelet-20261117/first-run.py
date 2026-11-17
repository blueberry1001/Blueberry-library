import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import time

root = Path.cwd()
here = root / '.build/wavelet-investigation'
here.mkdir(parents=True, exist_ok=True)
record = root / 'docs/development/measurements/wavelet-20261117'
shutil.copyfile(record / 'first-driver.cpp.txt', here / 'driver.cpp')
base = (record / 'baseline.hpp.txt').read_text()
start = base.index('    // Reuse the partition buffer for sorted positions:')
end = base.index('    height_ = std::bit_width', start)
constructor = '''    bool encoded = false;
    if (std::is_sorted(a.begin(), a.end())) {
      int distinct = 0;
      for (int i = 0; i < n_; ++i) {
        if (distinct == 0 || values_[distinct - 1] != a[i]) values_[distinct++] = a[i];
        current[i] = distinct - 1;
      }
      values_.resize(distinct);
      encoded = true;
    }
    if (!encoded) {
      std::iota(next.begin(), next.end(), 0);
      using U = std::make_unsigned_t<T>;
      U base = 0, remaining = 0;
      int passes = 0;
      if (n_ >= 64) {
        const auto [minimum, maximum] = std::minmax_element(a.begin(), a.end());
        base = static_cast<U>(*minimum);
        remaining = U(static_cast<U>(*maximum) - base);
        if constexpr (sizeof(T) <= 8) {
          if (static_cast<std::uint64_t>(remaining) <= std::min<std::uint64_t>(1u << 20, std::uint64_t(n_) * 2)) {
            std::vector<int> mapping(static_cast<std::size_t>(remaining) + 1, -1);
            for (const auto& value : a) mapping[U(static_cast<U>(value) - base)] = 0;
            int distinct = 0;
            for (int& value : mapping) if (value == 0) value = distinct++;
            for (int i = 0; i < n_; ++i) { current[i] = mapping[U(static_cast<U>(a[i]) - base)]; values_[current[i]] = a[i]; }
            values_.resize(distinct);
            encoded = true;
          }
        }
        for (U width = remaining; width; width = U(width >> 8)) ++passes;
        if constexpr (sizeof(T) <= 8) {
          int sampled = std::min(n_, 256), extreme = 0;
          for (int i = 0; i < sampled; ++i) { const T& value = a[std::size_t(i) * std::size_t(n_) / sampled]; extreme += value == *minimum || value == *maximum; }
          if (passes > 6 && extreme * 2 >= sampled) passes = n_;
        }
      }
      if (!encoded) {
        if (n_ < 64 || n_ < 128LL * passes) {
          std::sort(next.begin(), next.end(), [&](int i, int j) { return a[i] < a[j]; });
        } else {
          for (int shift = 0; remaining; shift += 8, remaining = U(remaining >> 8)) {
            std::array<int, 256> offsets{};
            for (int p : next) ++offsets[(U(static_cast<U>(a[p]) - base) >> shift) & 255];
            int offset = 0;
            for (int& count : offsets) { const int size = count; count = offset; offset += size; }
            for (int p : next) current[offsets[(U(static_cast<U>(a[p]) - base) >> shift) & 255]++] = p;
            current.swap(next);
          }
        }
        int distinct = 0;
        for (int p : next) {
          if (distinct == 0 || values_[distinct - 1] != a[p]) values_[distinct++] = a[p];
          current[p] = distinct - 1;
        }
        values_.resize(distinct);
      }
    }
'''
build = base[:start] + constructor + base[end:]
query = base.replace('int count_less(int l, int r, int code) const {', 'int count_less(int l, int r, int code, int first = 0) const {').replace('for (int h = 0; h < height_; ++h) {\n      const auto& level = levels_[h];\n      const int ones_l = level.rank(l), ones_r = level.rank(r);\n      if ((code >>', 'for (int h = first; h < height_; ++h) {\n      const auto& level = levels_[h];\n      const int ones_l = level.rank(l), ones_r = level.rank(r);\n      if ((code >>', 1)
old = '    return count_less(l, r, lower(upper)) - count_less(l, r, lower(lower_value));'
new = '''    const int lo = lower(lower_value), hi = lower(upper);
    if (lo == hi) return 0;
    if (hi == static_cast<int>(values_.size())) return r - l - count_less(l, r, lo);
    int first = 0;
    while (first < height_ && ((lo ^ hi) >> (height_ - 1 - first)) == 0) {
      const auto& level = levels_[first];
      const int ones_l = level.rank(l), ones_r = level.rank(r);
      if ((lo >> (height_ - 1 - first)) & 1) { l = level.zeros + ones_l; r = level.zeros + ones_r; }
      else { l -= ones_l; r -= ones_r; }
      ++first;
    }
    return count_less(l, r, hi, first) - count_less(l, r, lo, first);'''
assert old in query
query = query.replace(old,new)
combined = build.replace(base[start:end], constructor) if False else query[:query.index('    // Reuse the partition buffer for sorted positions:')] + constructor + query[query.index('    height_ = std::bit_width'):]
variants = {'baseline':base, 'build':build, 'query':query, 'combined':combined}
vc = Path(r'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808')
sdk = Path(r'C:\Program Files (x86)\Windows Kits\10')
version = '10.0.22621.0'
compiler = vc / 'bin/Hostx64/x64/cl.exe'
env = os.environ.copy()
env['PATH'] = str(compiler.parent) + os.pathsep + env.get('PATH','')
env['INCLUDE'] = os.pathsep.join(map(str,[vc/'include', *(sdk/'Include'/version/p for p in ('ucrt','shared','um'))]))
env['LIB'] = os.pathsep.join(map(str,[vc/'lib/x64', *(sdk/'Lib'/version/p/'x64' for p in ('ucrt','um'))]))
out = {'status':'running','platform':platform.platform(),'cpu':platform.processor(),'compiler':str(compiler),'flags':['/std:c++20','/O2','/DNDEBUG'],'samples':[],'sha256':{}}
def save(): (here/'results.json').write_text(json.dumps(out,indent=2)+'\n')
try:
  for name, text in variants.items():
    folder = here/name
    folder.mkdir(parents=True,exist_ok=True)
    header = folder/'wavelet-matrix.hpp'
    header.write_text(text,newline='\n')
    out['sha256'][name] = hashlib.sha256(header.read_bytes()).hexdigest()
    cmd = [str(compiler),'/nologo','/std:c++20','/EHsc','/O2','/DNDEBUG','/I'+str(folder),str(here/'driver.cpp'),'/Fo'+str(folder/'driver.obj'),'/Fe'+str(folder/'driver.exe')]
    p = subprocess.run(cmd,env=env,capture_output=True,text=True,errors='replace')
    if p.returncode: raise RuntimeError(str((cmd,p.stdout,p.stderr)))
  for shape in ['full','duplicates','narrow','sorted','extreme','biased']:
    for iteration in range(-1,5):
      names=list(variants)
      shift=(iteration+1)%len(names)
      checksum=None
      for name in names[shift:]+names[:shift]:
        p=subprocess.run([str(here/name/'driver.exe'),'200000','100000',shape],capture_output=True,text=True,errors='replace')
        if p.returncode: raise RuntimeError(str((name,shape,p.stdout,p.stderr)))
        fields=p.stdout.split()
        if checksum is None: checksum=fields[0]
        if checksum != fields[0]: raise RuntimeError(str((name,shape,checksum,fields)))
        if iteration>=0:
          out['samples'].append({'shape':shape,'iteration':iteration,'variant':name,'checksum':checksum,'build_ms':float(fields[1]),'kth_ms':float(fields[2]),'range_ms':float(fields[3]),'equal_ms':float(fields[4])})
    save()
    for name in variants:
      rows=[x for x in out['samples'] if x['shape']==shape and x['variant']==name]
      print(shape,name,*[round(statistics.median(x[k] for x in rows),3) for k in ('build_ms','kth_ms','range_ms','equal_ms')],flush=True)
  out['status']='complete'
except BaseException as exc:
  out['status']='failed'; out['error']=repr(exc); save(); raise
save()
