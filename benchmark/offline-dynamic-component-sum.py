#!/usr/bin/env python3
"""Bounded CSR versus nested-bucket comparison; source env.sh before preparation."""
import argparse
import difflib
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'benchmark/offline-dynamic-component-sum.cpp'
HEADER = 'blueberry/graph/offline-dynamic-component-sum.hpp'
DEPENDENCIES = [HEADER, 'blueberry/data-structure/rollback-union-find.hpp', 'blueberry/utility/fast-io.hpp']
BASE = ROOT / '.verification/offline-dynamic-connectivity/performance'
CELLS = [('long',300000,300000), ('churn',300000,300000), ('updates',300000,300000), ('cyclic',4096,300000)]
VARIANTS = ['public','buckets']


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def save(path, data): Path(path).write_text(json.dumps(data, indent=2)+'\n')

def command(argv, input_path=None):
    start = time.monotonic()
    try:
        with open(input_path or os.devnull, 'rb') as stream:
            r = subprocess.run([str(x) for x in argv], stdin=stream, capture_output=True, text=True, timeout=180)
        return dict(command=[str(x) for x in argv], returncode=r.returncode, stdout=r.stdout, stderr=r.stderr, elapsed_seconds=time.monotonic()-start)
    except subprocess.TimeoutExpired as e:
        return dict(command=[str(x) for x in argv], returncode=None, stdout=(e.stdout or b'').decode(), stderr=(e.stderr or b'').decode(), timeout=180)


def execute(argv, out, *, input_path=None, io=False, fields=()):
    r = command(argv, input_path)
    try:
        assert r['returncode'] == 0, 'nonzero exit or timeout'
        data = json.loads(r['stderr'] if io else r['stdout'])
        assert isinstance(data, dict) and all(k in data for k in fields), 'JSON schema'
        assert all(isinstance(v,int) and v>=0 for k,v in data.items() if k.endswith('_ns')), 'negative/noninteger duration'
        if io: data['output_sha256'] = hashlib.sha256(r['stdout'].encode()).hexdigest()
        return r,data
    except Exception as e:
        r['failure'] = repr(e); save(out/('failure-'+str(time.time_ns())+'.json'),r); raise


def alternate(source):
    # Deliberately narrow transform; unchanged interval/event loops preserve order.
    start = source.index('    std::vector<std::size_t> offsets(2 * base + 1);')
    end = source.index('    RollbackUnionFind uf(size());',start)
    result = source[:start] + '''    std::vector<std::vector<int>> buckets(2 * base);
    visit_events([&](int node, int event) { buckets[node].push_back(event); });

''' + source[end:]
    old = '''      for (std::size_t pos = offsets[node]; pos < offsets[node + 1]; ++pos) {
        const int event = events[pos];'''
    assert result.count(old)==1
    result = result.replace(old, '      for (const int event : buckets[node]) {')
    assert result.count('OfflineDynamicComponentSum')==2
    return result.replace('OfflineDynamicComponentSum','OfflineDynamicComponentSumBuckets')


def environment():
    return dict(platform=platform.platform(), cpu=Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
                cpu_quota=Path('/sys/fs/cgroup/cpu.max').read_text().strip(),
                processes=command(['ps','-eo','pid,pcpu,comm'])['stdout'],
                caveat='Shared host, unpinned. Sample ranges are not confidence intervals.')


def prepare(args):
    out=BASE/args.experiment
    if out.exists(): raise RuntimeError('Refusing existing experiment')
    out.mkdir(parents=True); snap=out/'snapshot';snap.mkdir()
    hashes={str(SOURCE.relative_to(ROOT)):sha(SOURCE), **{p:sha(ROOT/p) for p in DEPENDENCIES}}
    assert hashes[HEADER]==args.production_sha256, 'Requested frozen header differs'
    for name,expected in hashes.items():
        target=snap/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/name,target);assert sha(target)==expected
    before=(snap/HEADER).read_text();after=alternate(before);(snap/'control.hpp').write_text(after)
    (out/'control.diff').write_text(''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='production',tofile='nested-buckets')))
    shutil.copy2(__file__,out/'runner.py')
    meta=dict(head=command(['git','-C',ROOT,'rev-parse','HEAD'])['stdout'].strip(),source_sha256=hashes,
              runner_sha256=sha(__file__),control_sha256=sha(snap/'control.hpp'),cells=CELLS,entries=[],large_checks=[],environment=environment())
    for compiler in ['g++','clang++']:
        for mode in ['release','assert']:
            for allocation in ([False,True] if compiler=='g++' and mode=='release' else [False]):
                binary=out/(compiler.replace('+','p')+'-'+mode+('-allocation' if allocation else ''))
                flags=['-std=gnu++20','-O2','-Wall','-Wextra','-I',str(snap)]
                if mode=='release':flags.append('-DNDEBUG')
                if allocation:flags+=['-DODC_ALLOCATIONS','-Wno-mismatched-new-delete']
                r=command([compiler,*flags,snap/SOURCE.relative_to(ROOT),'-o',binary])
                e=dict(compiler=compiler,mode=mode,allocation=allocation,binary=str(binary.relative_to(ROOT)),flags=flags,
                       compiler_version=command([compiler,'--version'])['stdout'],compile=r)
                meta['entries'].append(e);save(out/'compile.json',meta['entries'])
                assert r['returncode']==0 and not r['stderr'], 'Compile failed or warned; log retained'
                e['sha256']=sha(binary);e['check']=command([binary,'check']);save(out/'compile.json',meta['entries'])
                assert e['check']['returncode']==0, 'BFS/type/snapshot smoke failed'
    for kind,n,q in CELLS:
        r,d=execute([ROOT/meta['entries'][0]['binary'],'compare',kind,n,q],out,fields=['input_hash','output_hash','queries'])
        meta['large_checks'].append(dict(workload=kind,n=n,q=q,data=d))
        save(out/'large-checks.json',meta['large_checks'])
    checks=[];identities={}
    for e in meta['entries']:
        for kind,_,_ in CELLS:
            for v in VARIANTS:
                r,d=execute([ROOT/e['binary'],'sample',v,kind,32,300],out,fields=['input_hash','output_hash','queries','constructor_ns','registration_ns','solve_ns','full_ns'])
                checks.append(dict(compiler=e['compiler'],mode=e['mode'],allocation=e['allocation'],variant=v,workload=kind,data=d));save(out/'small-checks.json',checks)
                identity=[d['input_hash'],d['output_hash'],d['queries']]
                if identity!=identities.setdefault(kind,identity):save(out/'small-mismatch.json',dict(rejected=r,rows=checks));raise RuntimeError('Small cross-binary mismatch')
    r=command([ROOT/meta['entries'][0]['binary'],'generate','churn',300000,300000]);assert r['returncode']==0
    (out/'churn.in').write_text(r['stdout']);meta['io_input']=dict(path=str((out/'churn.in').relative_to(ROOT)),sha256=sha(out/'churn.in'))
    meta['small_checks_sha256']=sha(out/'small-checks.json')
    for name,h in hashes.items():assert sha(ROOT/name)==h==sha(snap/name), 'Source changed during preparation'
    assert sha(__file__)==meta['runner_sha256'], 'Runner changed during preparation'
    save(out/'prepared.json',meta)
    print(f'Prepared{len(meta["entries"])}clean binaries,217BFS cases each plusnondefaulttype,4largefull-vector equalities,{len(checks)}small profiles. No performance run.')


def checked(args):
    out=BASE/args.experiment;meta=json.loads((out/'prepared.json').read_text())
    for name,h in meta['source_sha256'].items():assert sha(ROOT/name)==h==sha(out/'snapshot'/name),name
    assert sha(__file__)==meta['runner_sha256']==sha(out/'runner.py')
    assert sha(out/'snapshot/control.hpp')==meta['control_sha256']
    assert sha(out/'small-checks.json')==meta['small_checks_sha256']
    assert sha(ROOT/meta['io_input']['path'])==meta['io_input']['sha256']
    for e in meta['entries']:assert sha(ROOT/e['binary'])==e['sha256']
    return out,meta


def measure(args):
    out,meta=checked(args)
    if (out/'raw.jsonl').exists():raise RuntimeError('Refusing existing raw samples')
    save(out/'timing-environment.json',environment());start=time.monotonic()
    identities={r['workload']:[r['data']['input_hash'],r['data']['output_hash'],r['data']['queries']] for r in meta['large_checks']}
    io_hash=None
    with (out/'raw.jsonl').open('x') as f:
        for e in [x for x in meta['entries'] if not x['allocation']]:
            for rep in range(4):
                for kind,n,q in CELLS:
                    for v in VARIANTS[rep%2:]+VARIANTS[:rep%2]:
                        r,d=execute([ROOT/e['binary'],'sample',v,kind,n,q],out,fields=['input_hash','output_hash','queries','constructor_ns','registration_ns','solve_ns','full_ns'])
                        row=dict(family='kernel',compiler=e['compiler'],mode=e['mode'],rep=rep,warmup=rep==0,variant=v,workload=kind,n=n,q=q,data=d)
                        f.write(json.dumps(row)+'\n');f.flush()
                        if [d['input_hash'],d['output_hash'],d['queries']]!=identities[kind]:save(out/'identity-failure.json',dict(row=row,rejected=r));raise RuntimeError('Exact-output preflight identity differs')
                if e['mode']=='release':
                    for method in (['fast','iostream'] if rep%2==0 else ['iostream','fast']):
                        r,d=execute([ROOT/e['binary'],'io',method],out,input_path=ROOT/meta['io_input']['path'],io=True,
                                    fields=['parse_ns','algorithm_ns','format_ns','end_to_end_ns','input_hash','output_hash'])
                        row=dict(family='io',compiler=e['compiler'],mode=e['mode'],rep=rep,warmup=rep==0,method=method,workload='churn',data=d)
                        f.write(json.dumps(row)+'\n');f.flush()
                        if io_hash is None:io_hash=d['output_sha256']
                        if io_hash!=d['output_sha256'] or [d['input_hash'],d['output_hash']]!=identities['churn'][:2]:save(out/'io-failure.json',dict(row=row,rejected=r));raise RuntimeError('I/O answers differ')
    save(out/'timing-completed.json',dict(elapsed_seconds=time.monotonic()-start,kernel_profiles=128,io_profiles=16,warmups=36,raw_sha256=sha(out/'raw.jsonl')))
    print('Timing complete:128kernel+16release-I/O profiles,36warmups included.')


def diagnostics(args):
    out,meta=checked(args)
    with (out/'diagnostics.jsonl').open('x') as f:
        for e in [x for x in meta['entries'] if x['compiler']=='g++' and x['mode']=='release']:
            for kind,n,q in CELLS:
                expected=next(x['data'] for x in meta['large_checks'] if x['workload']==kind)
                for v in VARIANTS:
                    argv=['/bin/sh','-c','"$@"; result=$?; exit "$result"','odc',ROOT/e['binary'],'sample',v,kind,n,q]
                    r,d=execute(argv,out,fields=['input_hash','output_hash','queries','rss_kib']+(['allocation_calls','allocation_bytes'] if e['allocation'] else []))
                    row=dict(allocation=e['allocation'],variant=v,workload=kind,n=n,q=q,data=d);f.write(json.dumps(row)+'\n');f.flush()
                    if any(d[k]!=expected[k] for k in ['input_hash','output_hash','queries']):save(out/'diagnostic-failure.json',dict(row=row,rejected=r));raise RuntimeError('Diagnostic output differs')
    print('Separate8allocation+8uninstrumentedRSS diagnostics complete.')


def summarize(args):
    out,_=checked(args);groups={}
    for line in (out/'raw.jsonl').read_text().splitlines():
        r=json.loads(line)
        if r['warmup']:continue
        key=(r['family'],r['compiler'],r['mode'],r['workload'],r.get('variant',r.get('method')))
        groups.setdefault(key,[]).append(r['data'])
    rows=[]
    for key,data in groups.items():
        assert len(data)==3
        stats={k:dict(median=statistics.median(x[k] for x in data),min=min(x[k] for x in data),max=max(x[k] for x in data)) for k in data[0] if k.endswith('_ns')}
        rows.append(dict(family=key[0],compiler=key[1],mode=key[2],workload=key[3],variant=key[4],samples=3,metrics=stats))
    save(out/'summary.json',rows);print(f'Summarized{len(rows)}groups without pooling modes/workloads.')


if __name__=='__main__':
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','measure','diagnostics','summarize']);p.add_argument('--experiment',default='initial');p.add_argument('--production-sha256');args=p.parse_args()
    globals()[args.action](args)
