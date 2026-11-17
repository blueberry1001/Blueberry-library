import ctypes, hashlib, json, os, pathlib, platform, statistics, subprocess, time
root=pathlib.Path.cwd(); build=root/'.build/hld-path-study'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
kernel=ctypes.windll.kernel32
kernel.GetCurrentProcess.restype=ctypes.c_void_p
proc=kernel.GetCurrentProcess()
old=ctypes.c_size_t(); system=ctypes.c_size_t()
if not kernel.GetProcessAffinityMask(ctypes.c_void_p(proc),ctypes.byref(old),ctypes.byref(system)): raise RuntimeError('affinity query failed')
mask=old.value & -old.value
if not kernel.SetProcessAffinityMask(ctypes.c_void_p(proc),ctypes.c_size_t(mask)): raise RuntimeError('affinity set failed')
rows=[]
meta={'timestamp_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'platform':platform.platform(),'cpu':platform.processor(),'parent_affinity':hex(old.value),'test_affinity':hex(mask),'q':300000,'n':200000,'seed':1729,'warmups':2,'runs':7,'driver_sha256':sha(build/'driver.cpp'),'prepare_sha256':sha(build/'prepare.py'),'runner_sha256':sha(pathlib.Path(__file__)),'headers':{v:sha(build/v/'blueberry/graph/heavy-light-decomposition.hpp') for v in ('baseline','candidate2')},'rows':rows}
out=build/'results-v2.json'
def save(): out.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')
try:
  for compiler in ('msvc','clang'):
    for shape in ('path','star','balanced','random'):
      for edge in (0,1):
        previous=None
        for k in range(-2,7):
          for variant in (('baseline','candidate2') if k%2 else ('candidate2','baseline')):
            cmd=[str(build/variant/(compiler+'-driver.exe')),'200000','300000',shape,str(edge)]
            p=subprocess.run(cmd,capture_output=True,text=True,errors='replace',timeout=120)
            if p.returncode: raise RuntimeError('process failure '+str(cmd)+p.stderr)
            fields=p.stdout.split()
            if len(fields)!=3: raise RuntimeError('bad output '+p.stdout)
            pair=tuple(fields[:2]); ms=float(fields[2])
            if previous is not None and pair!=previous: raise RuntimeError('checksum mismatch')
            previous=pair
            rows.append({'compiler':compiler,'shape':shape,'edge':edge,'run':k+1,'variant':variant,'hash':fields[0],'segments':int(fields[1]),'ms':ms,'exit':p.returncode})
            save()
        for variant in ('baseline','candidate2'):
          vals=[x['ms'] for x in rows if x['compiler']==compiler and x['shape']==shape and x['edge']==edge and x['variant']==variant and x['run']>0]
          print(compiler,shape,edge,variant,round(statistics.median(vals),3),flush=True)
  meta['status']='complete'; save()
except BaseException as e:
  meta['status']='failed'; meta['error']=repr(e); save(); raise
finally:
  kernel.SetProcessAffinityMask(ctypes.c_void_p(proc),ctypes.c_size_t(old.value))
