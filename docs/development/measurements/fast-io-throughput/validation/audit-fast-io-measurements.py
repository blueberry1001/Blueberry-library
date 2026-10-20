import hashlib
import json
from pathlib import Path
import statistics

root = Path('docs/development/measurements/fast-io-throughput')
keys = ['profile', 'variant', 'facade', 'case', 'mode', 'transport']
total = 0
for directory in sorted(root.iterdir()):
    if not (directory / 'summary.json').is_file():
        continue
    env = json.loads((directory / 'environment.json').read_text())
    samples = [json.loads(line) for line in (directory / 'samples.jsonl').read_text().splitlines()]
    summaries = json.loads((directory / 'summary.json').read_text())
    assert all(r['returncode'] == 0 and r['output_byte_match'] for r in samples)
    assert len(samples) == len(summaries) * (env['repeats'] + env['warmups'])
    for row in summaries:
        group = [r for r in samples if not r['warmup'] and all(r[k] == row[k] for k in keys)]
        assert sorted(r['run'] for r in group) == list(range(env['repeats']))
        for field in ['setup_ms', 'operation_ms', 'process_ms', 'peak_rss_kib']:
            values = [r[field] for r in group]
            assert row[field] == {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
    for source, snapshot in [('blueberry/utility/fast-io.hpp', 'fast-io.hpp.snapshot'),
                             ('benchmark/fast-io-throughput.cpp', 'benchmark.cpp.snapshot'),
                             ('benchmark/fast-io-throughput.py', 'run.py.snapshot')]:
        path = directory / snapshot
        assert path.is_file(), path
        assert hashlib.sha256(path.read_bytes()).hexdigest() == env['source_sha256'][source], path
    if directory.name.startswith('final-'):
        assert env['git_revision'] == '8729666f80f04e080f945506fd3cb94be7081185'
        assert env['repeats'] == 5
        assert env['source_sha256']['blueberry/utility/fast-io.hpp'] == '8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406'
    total += len(samples)
    print(f'{directory.name}: {len(samples)} byte matches, summaries and snapshots verified')
print(f'Total: {total} successful recorded executions')
