#!/usr/bin/env python3
"""Finite paired benchmarks; preserve raw values and source/environment hashes."""
import argparse
import hashlib,json,platform,statistics,subprocess,tempfile
from pathlib import Path
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[4]
OUT=Path(__file__).resolve().parent
names=['fast-set','dual-segment-tree','double-ended-priority-queue','skew-heap','slope-trick','splay-slope-trick']
files=[ROOT/'benchmarks/specialized-structures.cpp',*[ROOT/f'blueberry/data-structure/{n}.hpp' for n in names],*sorted((ROOT/'.deps/ac-library/atcoder').glob('*'))]
def hashes():
 return {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in files if p.is_file()}
def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=OUT,
                        help='Output directory; refuses any existing measurement files')
    args = parser.parse_args(argv)
    output = args.output.resolve()
    protected = ['environment.json', 'samples.jsonl', 'summary.json', 'compile.log', 'failure.json']
    existing = [name for name in protected if (output / name).exists()]
    if existing:
        parser.error(f'Refusing to overwrite existing measurements in {output}: {", ".join(existing)}')
    if output.exists() and not output.is_dir():
        parser.error(f'Output path is not a directory: {output}')
    output.mkdir(parents=True, exist_ok=True)
    initial=hashes()
    meta={'timestamp_utc':datetime.now(timezone.utc).isoformat(),'platform':platform.platform(),'cpuinfo':Path('/proc/cpuinfo').read_text().split('processor\t: 1')[0],'compiler':subprocess.check_output(['g++','--version'],text=True),'flags':['-std=gnu++20','-O2','-DNDEBUG','-I.','-I.deps/ac-library'],'revision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'source_sha256':initial,'n':[4096,65536],'q':100000,'seed':20260919,'runs':5,'order':'Alternate candidate order each run; fresh process, no warmup','memory':'VmHWM including executable and generated input; KiB','timing':'Construction, operations and destruction; input generation excluded'}
    (output/'environment.json').write_text(json.dumps(meta,indent=2)+'\n')
    rows=[]
    with tempfile.TemporaryDirectory(prefix='specialized-bench-') as temp:
     binary=Path(temp)/'benchmark'
     command=['g++',*meta['flags'],'benchmarks/specialized-structures.cpp','-o',str(binary)]
     result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
     (output/'compile.log').write_text(' '.join(command)+'\n'+result.stdout+result.stderr)
     result.check_returncode()
     with (output/'samples.jsonl').open('w') as sample_stream:
      for group in ['fast','dual','depq','meld','slope']:
       sizes=[4096] if group=='meld' else meta['n']
       for n in sizes:
        for run in range(meta['runs']):
         for variant in (['blueberry','reference'] if run%2==0 else ['reference','blueberry']):
          command=[str(binary),group,variant,str(n)]
          result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
          if result.returncode:
           (output/'failure.json').write_text(json.dumps({'command':command,'stdout':result.stdout,'stderr':result.stderr,'returncode':result.returncode},indent=2))
          result.check_returncode()
          row=json.loads(result.stdout);row.update(group=group,variant=variant,n=n,run=run)
          rows.append(row);sample_stream.write(json.dumps(row)+'\n');sample_stream.flush()
        selected=[r for r in rows if r['group']==group and r['n']==n]
        assert len({r['checksum'] for r in selected})==1,(group,n,'checksum mismatch')
        assert len({r['input_hash'] for r in selected})==1,(group,n,'input mismatch')
    assert initial==hashes(),'Source changed during measurement'
    summary=[]
    for group,n,variant in sorted({(r['group'],r['n'],r['variant']) for r in rows}):
     selected=[r for r in rows if (r['group'],r['n'],r['variant'])==(group,n,variant)]
     summary.append({'group':group,'n':n,'variant':variant,'median_ms':statistics.median(r['milliseconds'] for r in selected),'min_ms':min(r['milliseconds'] for r in selected),'median_peak_rss_kib':statistics.median(r['peak_rss_kib'] for r in selected)})
    (output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps(summary,indent=2))


if __name__ == '__main__':
    main()
