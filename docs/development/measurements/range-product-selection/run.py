#!/usr/bin/env python3
"""Static/dynamic range-product comparison; preserves every independent sample."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[4]
OUT = Path(__file__).resolve().parent
HEADERS = ['disjoint-sparse-table', 'sqrt-tree', 'dynamic-sqrt-tree']
FILES = [ROOT / 'benchmarks/range-product-selection.cpp',
         *[ROOT / f'blueberry/data-structure/{name}.hpp' for name in HEADERS],
         *sorted((ROOT / '.deps/ac-library/atcoder').glob('*'))]

def hashes():
    return {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in FILES if p.is_file()}

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
    initial_hashes = hashes()
    metadata = {
        'timestamp_utc': datetime.now(timezone.utc).isoformat(),
        'platform': platform.platform(),
        'cpuinfo': Path('/proc/cpuinfo').read_text().split('processor\t: 1')[0],
        'compiler': subprocess.check_output(['g++', '--version'], text=True),
        'flags': ['-std=gnu++20', '-O2', '-DNDEBUG', '-I.', '-I.deps/ac-library'],
        'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'acl_revision': subprocess.check_output(['git', '-C', '.deps/ac-library', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'source_sha256': initial_hashes,
        'n': [4096, 65536], 'operations': [100, 100000],
        'update_percent': [0, 1, 50], 'seed': 20260919, 'runs': 5,
        'memory': 'Linux VmHWM KiB; current executable including identical generated inputs',
        'timing': 'Construction and operations separately; input generation/destruction excluded; operations include checksum mixing and final full-range query',
        'order': 'Rotate candidate order across repetitions; one fresh process per sample, no warmup',
        'workload': 'Signed 64-bit range sum, values 0..1000; two uniform endpoints sorted into nonempty ranges; periodic exact update percentages; same prefix at both operation counts',
        'excluded': 'DST and static SqrtTree omitted when updates>0; rebuilding static structures after each update is not a supported competitive use case',
    }
    (output /  'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    rows = []
    with tempfile.TemporaryDirectory(prefix='blueberry-range-selection-') as temporary:
        binary = Path(temporary) / 'benchmark'
        command = ['g++', *metadata['flags'], 'benchmarks/range-product-selection.cpp', '-o', str(binary)]
        compiled = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
        (output /  'compile.log').write_text(' '.join(command) + '\n' + compiled.stdout + compiled.stderr)
        compiled.check_returncode()
        with (output /  'samples.jsonl').open('w') as stream:
            for n in metadata['n']:
                for operations in metadata['operations']:
                    for rate in metadata['update_percent']:
                        variants = ['dst', 'static', 'dynamic', 'acl'] if rate == 0 else ['dynamic', 'acl']
                        for run in range(metadata['runs']):
                            order = variants[run % len(variants):] + variants[:run % len(variants)]
                            for variant in order:
                                command = [str(binary), variant, str(n), str(operations), str(rate), str(metadata['seed'])]
                                sample = subprocess.run(command, cwd=ROOT, text=True, capture_output=True, timeout=60)
                                if sample.returncode:
                                    (output /  'failure.json').write_text(json.dumps({'command': command, 'returncode': sample.returncode,
                                        'stdout': sample.stdout, 'stderr': sample.stderr}, indent=2))
                                sample.check_returncode()
                                row = json.loads(sample.stdout)
                                row['run'] = run
                                row['total_ms'] = row['build_ms'] + row['operations_ms']
                                stream.write(json.dumps(row) + '\n'); stream.flush()
                                rows.append(row)
                        group = [r for r in rows if (r['n'], r['queries'], r['update_percent']) == (n, operations, rate)]
                        assert len({r['input_hash'] for r in group}) == 1, ('input mismatch', n, operations, rate)
                        assert len({r['checksum'] for r in group}) == 1, ('result mismatch', n, operations, rate)
                        print('PASS', n, operations, rate, flush=True)
    assert len(rows) == 160
    assert hashes() == initial_hashes, 'Source changed during measurement; logs retained, not comparable.'
    summary = []
    for n, operations, rate, variant in sorted({(r['n'], r['queries'], r['update_percent'], r['implementation']) for r in rows}):
        group = [r for r in rows if (r['n'], r['queries'], r['update_percent'], r['implementation']) == (n, operations, rate, variant)]
        item = {'n': n, 'operations': operations, 'update_percent': rate, 'implementation': variant,
                'input_hash': group[0]['input_hash'], 'checksum': group[0]['checksum'], 'runs': len(group)}
        for field in ['build_ms', 'operations_ms', 'total_ms', 'peak_process_rss_kib']:
            item[field + '_median'] = statistics.median(r[field] for r in group)
            item[field + '_min'] = min(r[field] for r in group)
        summary.append(item)
    (output /  'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('Complete:', len(rows), 'samples; all hashes and paired checksums matched.')


if __name__ == '__main__':
    main()
