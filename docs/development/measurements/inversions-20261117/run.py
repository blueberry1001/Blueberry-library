import hashlib,json,platform,statistics,subprocess
from pathlib import Path
root=Path('.build/inversions-experiment')
modes=['baseline','guarded','shortguard']
cases=[(100000,1,'random'),(100000,100,'random'),(100000,1000,'random'),(100000,10000,'random'),(100000,50000,'random'),(100000,100000,'random'),(20000,200000,'random'),(100000,1000,'short'),(100000,10000,'short'),(100000,1000,'mixed'),(100000,1000,'duplicate'),(100000,100000,'short'),(100000,100000,'duplicate')]
rows=[]
for n,q,shape in cases:
    for run in range(6):
        for mode in (modes if run%2==0 else list(reversed(modes))):
            proc=subprocess.run([str(root/'bench.exe'),mode,str(n),str(q),shape],capture_output=True,text=True)
            row={'n':n,'q':q,'shape':shape,'run':run,'mode':mode,'returncode':proc.returncode,'stdout':proc.stdout,'stderr':proc.stderr}
            if proc.returncode==0: row.update(json.loads(proc.stdout))
            rows.append(row)
            (root/'results.json').write_text(json.dumps({'rows':rows},indent=2))
            print(n,q,shape,run,mode,proc.stdout.strip(),flush=True)
            if proc.returncode: raise RuntimeError(row)
    assert len({x['checksum'] for x in rows if (x['n'],x['q'],x['shape'])==(n,q,shape)})==1
summary=[]
for n,q,shape in cases:
    for mode in modes:
        samples=[r['ms'] for r in rows if (r['n'],r['q'],r['shape'],r['mode'])==(n,q,shape,mode) and r['run']>0]
        summary.append(dict(n=n,q=q,shape=shape,mode=mode,median_ms=statistics.median(samples),min_ms=min(samples)))
files=[str(root/(m+'.hpp')) for m in ['baseline','width','blocks','both','guarded','shortguard']]+[str(root/'bench.cpp')]
result={'platform':platform.platform(),'seed':20261117,'warmup':1,'repeats':5,'flags':'--target=x86_64-pc-windows-msvc -fuse-ld=link -std=gnu++20 -O2 -DNDEBUG','timing':'entire function; input generation and checksum outside','hashes':{f:hashlib.sha256(Path(f).read_bytes()).hexdigest() for f in files},'summary':summary,'rows':rows}
(root/'results.json').write_text(json.dumps(result,indent=2))
