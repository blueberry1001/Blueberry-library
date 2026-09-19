#!/usr/bin/env python3
"""Same RMQ algorithm/inputs/compiler: isolate buffered I/O and tree changes."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import random
import selectors
import statistics
import subprocess
import tempfile
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
PROFILES = {
    'gcc-release': ['g++', '-std=gnu++20', '-O2', '-DNDEBUG'],
    'gcc-verify': ['g++', '-std=gnu++20', '-O2', '-D_GLIBCXX_ASSERTIONS'],
    'clang-release': ['clang++', '-std=gnu++20', '-O2', '-DNDEBUG'],
}
CASES = ['max_random_00', 'small_width_query_00', 'small_values_00']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def interactive(binary):
    # Keep stdin open: each reply must arrive before the next request is sent.
    proc = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        with selectors.DefaultSelector() as selector:
            selector.register(proc.stdout, selectors.EVENT_READ)
            for request, expected in [(b'7\n', b'8\n'), (b'-8\n', b'-7\n')]:
                proc.stdin.write(request); proc.stdin.flush()
                reply = b''; deadline = time.monotonic() + 5
                while len(reply) < len(expected):
                    remaining = deadline - time.monotonic()
                    assert remaining > 0 and selector.select(timeout=remaining), 'interactive input blocked waiting for a full batch'
                    chunk = os.read(proc.stdout.fileno(), len(expected) - len(reply))
                    assert chunk, 'unexpected interactive EOF'
                    reply += chunk
                assert reply == expected
        proc.stdin.close()
        assert proc.wait(timeout=5) == 0
    finally:
        if proc.poll() is None: proc.kill(); proc.wait()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--problem-dir', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=5)
    args = parser.parse_args()
    if args.repeats < 3: parser.error('At least three repetitions required')
    args.output.mkdir(parents=True, exist_ok=False)
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    headers = ['blueberry/data-structure/sqrt-tree.hpp', 'blueberry/utility/fast-io.hpp']
    tracked = [ROOT / p for p in headers] + [HERE / 'benchmark.cpp', HERE / 'interactive.cpp', Path(__file__)]
    hashes = {str(p.relative_to(ROOT)): sha(p) for p in tracked}
    revision = subprocess.check_output(['git', 'rev-parse', 'e0d8e71'], cwd=ROOT, text=True).strip()
    env = {'baseline_revision': revision, 'current_source_sha256': hashes,
           'timestamp_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
           'platform': platform.platform(), 'cpuinfo': Path('/proc/cpuinfo').read_text().split('\n\n')[0],
           'cpu_affinity': cpu, 'profiles': PROFILES, 'repeats': args.repeats, 'warmups': 1,
           'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']},
           'input_revision': subprocess.check_output(['git', '-C', str(args.problem_dir), 'rev-parse', 'HEAD'], text=True).strip(),
           'inputs': {c: {'input': sha(args.problem_dir / 'in' / (c + '.in')), 'output': sha(args.problem_dir / 'out' / (c + '.out'))} for c in CASES},
           'timing': 'total_ms includes read, build, queries, output flush; process_ms also includes startup/destruction and time wrapper. Full output checked every run.',
           'ordering': 'Seed 20260919 shuffle each round; one pinned process at a time; stdin regular file and pipe, stdout regular file.'}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2)+'\n')
    rows = []
    with tempfile.TemporaryDirectory(prefix='blueberry-io-') as directory:
        temp = Path(directory)
        old_header = temp / 'baseline' / headers[0]; old_header.parent.mkdir(parents=True)
        old_header.write_bytes(subprocess.check_output(['git', 'show', f'{revision}:{headers[0]}'], cwd=ROOT))
        env['baseline_header_sha256'] = sha(old_header)
        (args.output / 'environment.json').write_text(json.dumps(env, indent=2)+'\n')
        binaries = {}
        with (args.output / 'compile.log').open('w') as log:
            for profile, flags in PROFILES.items():
                for variant in ['baseline', 'current']:
                    binary = temp / f'{profile}-{variant}'
                    cmd = [*flags, '-I'+str(temp / variant), '-I'+str(ROOT), str(HERE / 'benchmark.cpp'), '-o', str(binary)]
                    log.write(json.dumps(cmd)+'\n'); log.flush()
                    subprocess.run(cmd, stdout=log, stderr=log, check=True)
                    binaries[profile,variant] = binary
                binary = temp / (profile+'-interactive')
                cmd = [*flags, '-I'+str(ROOT), str(HERE / 'interactive.cpp'), '-o', str(binary)]
                subprocess.run(cmd, stdout=log, stderr=log, check=True)
                interactive(binary)
        (args.output / 'interactive.json').write_text(json.dumps({'profiles':list(PROFILES), 'paced_open_pipe': 'passed'})+'\n')
        configs = list(itertools.product(PROFILES, ['baseline','current'], ['streams','fast'], ['file','pipe'], CASES))
        rng = random.Random(20260919)
        inputs = {c: (args.problem_dir / 'in' / (c+'.in')).read_bytes() for c in CASES}
        expected = {c: (args.problem_dir / 'out' / (c+'.out')).read_bytes().split() for c in CASES}
        for c in CASES: (temp / (c+'.in')).write_bytes(inputs[c])
        with (args.output / 'samples.jsonl').open('w') as log:
            for run in range(-1,args.repeats):
                rng.shuffle(configs)
                for profile, variant, io, transport, case in configs:
                    rss = temp / 'rss'; output = temp / 'out'
                    cmd = ['/usr/bin/time','-f','%M','-o',str(rss),str(binaries[profile,variant]),io]
                    with output.open('wb') as dest:
                        start = time.perf_counter()
                        if transport == 'file':
                            with (temp / (case+'.in')).open('rb') as source:
                                result = subprocess.run(cmd,stdin=source,stdout=dest,stderr=subprocess.PIPE,timeout=30)
                        else:
                            result = subprocess.run(cmd,input=inputs[case],stdout=dest,stderr=subprocess.PIPE,timeout=30)
                        elapsed = (time.perf_counter()-start)*1000
                    row = {'profile':profile,'variant':variant,'io':io,'transport':transport,'case':case,'run':run,'warmup':run<0,'returncode':result.returncode}
                    if result.returncode or output.read_bytes().split() != expected[case]:
                        (args.output/'failure.json').write_text(json.dumps({**row,'stderr':result.stderr.decode(errors='replace')}))
                        raise RuntimeError('Execution/output mismatch; see failure.json')
                    row.update(json.loads(result.stderr),process_ms=elapsed,peak_rss_kib=int(rss.read_text()))
                    log.write(json.dumps(row)+'\n'); log.flush()
                    if run>=0: rows.append(row)
                print('Finished round',run,flush=True)
    assert hashes == {str(p.relative_to(ROOT)):sha(p) for p in tracked}, 'Sources changed during benchmark'
    summary=[]
    for profile,variant,io,transport,case in sorted(configs):
        group=[r for r in rows if (r['profile'],r['variant'],r['io'],r['transport'],r['case'])==(profile,variant,io,transport,case)]
        row={k:group[0][k] for k in ['profile','variant','io','transport','case']}
        for field in ['input_ms','build_ms','query_io_ms','total_ms','process_ms','peak_rss_kib']:
            row[field]={ 'median':statistics.median(r[field] for r in group),'min':min(r[field] for r in group),'max':max(r[field] for r in group)}
        summary.append(row)
    (args.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')

if __name__=='__main__': main()
