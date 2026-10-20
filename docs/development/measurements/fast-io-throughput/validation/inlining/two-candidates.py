import hashlib,itertools,json,os,platform,random,statistics,subprocess,tempfile,time
from pathlib import Path
root=Path.cwd();out=root/'.build/fast-io-throughput/inlining-review'
source=(root/'blueberry/utility/fast-io.hpp').read_text()
old_begin=source.index('    if constexpr (!Interactive && sizeof(U) >= 4 && std::endian::native == std::endian::little) {')
old_end=source.index('    while (static_cast<unsigned>(c - \'0\') <= 9) {',old_begin)
original=source[old_begin:old_end]
block4='''    if constexpr (!Interactive && sizeof(U) >= 4 && sizeof(U) <= 8 && std::endian::native == std::endian::little) {
      const U block_quotient = negative ? negative_limit / 10000 : positive_limit / 10000;
      const U block_remainder = negative ? negative_limit % 10000 : positive_limit % 10000;
      while (end_ - pos_ >= 3 && static_cast<unsigned>(buffer_[pos_] - '0') <= 9) {
        std::uint32_t block;
        std::memcpy(&block, buffer_.data() + pos_ - 1, sizeof block);
        constexpr std::uint32_t high = 0xf0f0f0f0u;
        if ((block & high) != 0x30303030u || ((block + 0x06060606u) & high) != 0x30303030u) break;
        block &= 0x0f0f0f0fu;
        block = (block * 10 + (block >> 8)) & 0x00ff00ffu;
        const auto digits = static_cast<unsigned>((block * 100 + (block >> 16)) & 0xffffu);
        if (result > block_quotient || (result == block_quotient && digits > block_remainder)) {
          discard(c); return false;
        }
        result = U(result * 10000 + digits);
        pos_ += 3;
        c = next();
      }
    } else '''
changed4=source[:old_begin]+block4+original.lstrip()+source[old_end:]
changed_inline=source.replace('  bool read(T& value) {','  __attribute__((always_inline)) bool read(T& value) {',1)
variants={'baseline':root/'.build/fast-io-throughput/baseline','current':root,'always-inline':out/'always-inline','word4':out/'word4'}
for name,text in [('always-inline',changed_inline),('word4',changed4)]:
 header=variants[name]/'blueberry/utility/fast-io.hpp';header.parent.mkdir(parents=True,exist_ok=True);header.write_text(text)
flags=['g++','-std=gnu++20','-O2','-DNDEBUG','-DIO_FACADE=0'];acl='/mnt/c/Users/kubar/Documents/GitHub/Blueberry-library/.deps/ac-library'
binaries={}
with (out/'candidate-compile.log').open('w') as log:
 for name,include in variants.items():
  binary=out/(name+'.bin');cmd=['taskset','-c','4',*flags,'-I'+str(include),'-I'+str(root),'-isystem',acl,str(root/'benchmark/fast-io-throughput.cpp'),'-o',str(binary)]
  log.write(json.dumps(cmd)+'\n');log.flush();subprocess.run(cmd,stdout=log,stderr=log,check=True);binaries[name]=binary
sha=lambda data:hashlib.sha256(data).hexdigest()
cache=Path('/mnt/c/Users/kubar/Documents/GitHub/Blueberry-library/.verify-helper/cache/4eadb574dcb743b83b4a231fcab69002/test')
env={'timestamp_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'platform':platform.platform(),'compiler':subprocess.check_output(['g++','--version'],text=True),'flags':flags,'cpu':2,'repeats':5,'warmups':1,'source_sha256':{'benchmark':sha((root/'benchmark/fast-io-throughput.cpp').read_bytes()),'current_header':sha(source.encode()),'always_inline_header':sha(changed_inline.encode()),'word4_header':sha(changed4.encode())},'experiment':'Separate hypothesis screen: original header and runner unchanged. Other root make check activity may overlap; do not combine with final baseline measurements. Input copied to /tmp exactly as final runner.'}
with tempfile.TemporaryDirectory(prefix='blueberry-io-inline-') as directory:
 temp=Path(directory);cases={}
 for name in ['digit_random_00','max_random_00']:
  data=(cache/(name+'.in')).read_bytes();expected=(cache/(name+'.out')).read_bytes();checksum=f'{sum(map(int,expected.split()))%(1<<64)}\n'.encode();path=temp/(name+'.in');path.write_bytes(data)
  cases[name]={'data':data,'output':expected,'checksum':checksum,'path':path}
 env['cases']={n:{'input_sha256':sha(c['data']),'output_sha256':sha(c['output']),'checksum':c['checksum'].decode().strip()} for n,c in cases.items()}
 (out/'candidate-environment.json').write_text(json.dumps(env,indent=2)+'\n')
 os.sched_setaffinity(0,{2});rng=random.Random(20261020);configs=list(itertools.product(variants,cases,['file','pipe']));rows=[]
 # Full output oracle catches conversion errors independently before timing input-only.
 for variant,name in itertools.product(variants,cases):
  with cases[name]['path'].open('rb') as src: result=subprocess.run([str(binaries[variant]),'i64','end-to-end','1000000'],stdin=src,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True)
  assert result.stdout==cases[name]['output'],(variant,name,'full-output mismatch')
 with (out/'candidate-samples.jsonl').open('w') as log:
  for run in range(-1,5):
   rng.shuffle(configs)
   for variant,name,transport in configs:
    case=cases[name];cmd=[str(binaries[variant]),'i64','input','1000000']
    if transport=='file':
     with case['path'].open('rb') as src: result=subprocess.run(cmd,stdin=src,stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True)
    else: result=subprocess.run(cmd,input=case['data'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,check=True)
    assert result.stdout==case['checksum']
    row={'variant':variant,'case':name,'transport':transport,'run':run,'warmup':run<0,'byte_match':True,**json.loads(result.stderr)}
    log.write(json.dumps(row)+'\n');log.flush()
    if run>=0:rows.append(row)
 summary=[]
 for variant,name,transport in sorted(configs):
  group=[r['operation_ms'] for r in rows if (r['variant'],r['case'],r['transport'])==(variant,name,transport)]
  summary.append({'variant':variant,'case':name,'transport':transport,'median':statistics.median(group),'min':min(group),'max':max(group)})
 (out/'candidate-summary.json').write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
