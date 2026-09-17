#!/usr/bin/env python3
"""Run only during a coordinated quiet window; source/binary hashes stay logged."""
import hashlib
import json
import pathlib
import platform
import shutil
import statistics
import subprocess
import tempfile
import time
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = ROOT / '.build/operation-structures'
OUTPUT = ROOT / 'benchmark/results/operation-structures'
prepared = json.loads((BASE / 'prepared.json').read_text())
for name, expected_hash in prepared['header_sha256'].items():
    header = ROOT / 'blueberry/data-structure' / (name + '.hpp')
    if hashlib.sha256(header.read_bytes()).hexdigest() != expected_hash:
        raise RuntimeError(f'Header changed after compilation: {name}; run preparation again')
for variant in prepared['rows']:
    if hashlib.sha256((ROOT / variant['source']).read_bytes()).hexdigest() != variant['source_sha256']:
        raise RuntimeError(f'Source changed after compilation: {variant["source"]}')
rows = []
input_hashes = {}
with tempfile.TemporaryDirectory(prefix='blueberry-operation-benchmark-') as directory:
    native = pathlib.Path(directory)
    for problem in sorted({row['problem'] for row in prepared['rows']}):
        source_input = BASE / (problem + '.in')
        local_input = native / (problem + '.in')
        shutil.copyfile(source_input, local_input)
        input_hashes[problem] = hashlib.sha256(source_input.read_bytes()).hexdigest()
        variants = [r for r in prepared['rows'] if r['problem'] == problem and r['compile_returncode'] == 0]
        for variant in variants:
            shutil.copy2(ROOT / variant['binary'], native / pathlib.Path(variant['binary']).name)
        expected = None
        for repeat in range(-1, 5):
            for variant in variants[::1 if repeat % 2 == 0 else -1]:
                binary = native / pathlib.Path(variant['binary']).name
                with local_input.open('rb') as source:
                    start = time.perf_counter_ns()
                    result = subprocess.run([str(binary)], stdin=source, capture_output=True, timeout=60)
                    elapsed = time.perf_counter_ns() - start
                digest = hashlib.sha256(b'\n'.join(result.stdout.split())).hexdigest()
                row = dict(problem=problem, variant=variant['variant'], repeat=repeat, ns=elapsed,
                           returncode=result.returncode, token_sha256=digest,
                           stderr=result.stderr.decode(errors='replace'))
                rows.append(row)
                (OUTPUT / 'raw.json').write_text(json.dumps(rows, indent=2) + '\n')
                result.check_returncode()
                if expected is None:
                    expected = digest
                if expected != digest:
                    raise RuntimeError(f'Output mismatch: {problem} {variant["variant"]}')
summary = []
for problem, variant in sorted({(row['problem'], row['variant']) for row in rows}):
    samples = [r['ns'] for r in rows if r['problem'] == problem and r['variant'] == variant and r['repeat'] >= 0]
    summary.append(dict(problem=problem, variant=variant, samples=samples,
                        median_ns=statistics.median(samples), min_ns=min(samples)))
report = dict(generated_at=datetime.now(timezone.utc).isoformat(), prepared=prepared, input_sha256=input_hashes,
              compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              cpu=subprocess.check_output(['lscpu'], text=True), platform=platform.platform(),
              conditions='One warmup, five alternating measured repetitions. Native Linux temporary files/binaries, regular-file stdin, process startup and each program I/O included. Other agent compile/verify processes paused by coordination; desktop background activity may remain.',
              summary=summary)
(OUTPUT / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(summary, indent=2))
