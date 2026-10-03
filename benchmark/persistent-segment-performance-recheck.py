#!/usr/bin/env python3
"""One bounded same-binary Persistent control recheck; preserve all main profiles."""
import importlib.util
import os
from pathlib import Path
import platform
import statistics
from datetime import datetime, timezone

SOURCE = Path(__file__).with_name('point-access-performance.py')
spec = importlib.util.spec_from_file_location('final_point_profiles', SOURCE)
profile = importlib.util.module_from_spec(spec)
spec.loader.exec_module(profile)
OUT = profile.RESULTS / 'persistent-segment-recheck'
GROUPS = [
    ('assert', 'dense-sequential', 'clang++', 'Unchanged controls: build+36.676%, update+14.817%, range+10.2051% with disjoint slower range samples; large get/update outliers.'),
    ('assert', 'dense-branch', 'clang++', 'Candidate get outlier and overlapping slower build/update controls.'),
    ('release', 'sparse-branch', 'g++', 'Unchanged range+7.2715% with disjoint slower samples.'),
    ('release', 'dense-sequential', 'clang++', 'Unchanged range+5.5427% with disjoint slower samples.'),
]


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if (OUT/'prepared.json').exists():
        raise RuntimeError('Preserve existing focused recheck before reproduction')
    ready, metadata = [], []
    for mode, shape, compiler, reason in GROUPS:
        data = profile.prepared('persistent-segment', mode)
        profile.checked_correctness('persistent-segment', mode)
        case = ('persistent-segment', shape, 'u64', 100000, 200000)
        if case not in profile.cases('persistent-segment'):
            raise RuntimeError('Recheck must use an unchanged existing input case')
        entries = [(entry, variant) for entry, variant in profile.entries_for(data, 'persistent-segment')
                   if entry['compiler'] == compiler]
        if len(entries) != 2: raise RuntimeError('Expected baseline and candidate binaries')
        for entry, _ in entries: entry['failure_directory'] = str(OUT)
        _, original = profile.location('persistent-segment', mode)
        metadata.append(dict(mode=mode, shape=shape, compiler=compiler, reason=reason, case=case,
            original_prepared_sha256=profile.digest(original/'prepared.json'),
            original_correctness_gate_sha256=profile.digest(original/'correctness-gate.json'),
            original_raw_sha256=profile.digest(original/'raw.json'),
            production_header_sha256=data['production_header_sha256'],
            source_sha256=data['source_sha256'], header_sha256=data['header_sha256'], entries=entries))
        ready.append((mode, case, entries))
    limits={}
    for name in ['cpu.max', 'cpu.weight', 'cpuset.cpus.effective', 'memory.max']:
        p=Path('/sys/fs/cgroup')/name
        limits[name]=p.read_text().strip() if p.exists() else 'unavailable'
    profile.write(OUT,'prepared.json',dict(created_at=datetime.now(timezone.utc).isoformat(),
        baseline_commit=profile.BASELINE, groups=metadata, repeats=9, warmups=1,
        runner_sha256=profile.digest(Path(__file__)), shared_runner_sha256=profile.digest(SOURCE),
        platform=platform.platform(), cgroup=limits, pid_affinity=sorted(os.sched_getaffinity(0)),
        criteria='Exactly the four preselected cells above. One warmup then nine alternating variants; all stages retained. Compare independently against original medians/ranges; do not pool, replace or filter either run. No new inputs, compiled source, binaries, flags or optimization candidates. Shared host/unpinned; other-tenant/frequency variation remains possible.'))
    profile.quiet(OUT)
    rows, summary = [], []
    started=datetime.now(timezone.utc)
    for mode, case, entries in ready:
        expected=None
        for repeat in range(-1,9):
            for entry, variant in entries if repeat%2 else entries[::-1]:
                row=profile.execute(entry,case,variant,False)
                if expected is None: expected=profile.identity(row)
                if profile.identity(row)!=expected:
                    profile.write(OUT,'failed-profile.json',dict(rejected=row,expected=expected))
                    raise RuntimeError('Focused checksum disagreement')
                row.update(mode=mode,repeat=repeat,warmup=repeat<0)
                rows.append(row); profile.write(OUT,'raw.json',rows)
        for entry,variant in entries:
            selected=[r for r in rows if not r['warmup'] and
                (r['mode'],r['shape'],r['compiler'],r['implementation'])==(mode,case[1],entry['compiler'],variant)]
            fields=[k for k in selected[0] if k.endswith('_ns')]
            metrics={k:dict(median=statistics.median(r[k] for r in selected),
                min=min(r[k] for r in selected),max=max(r[k] for r in selected)) for k in fields}
            summary.append(dict(family=case[0],shape=case[1],scalar=case[2],mode=mode,
                compiler=entry['compiler'],implementation=variant,metrics=metrics))
        print(f'Completed focused {mode}/{case[1]}/{entries[0][0]["compiler"]}',flush=True)
    profile.write(OUT,'results.json',dict(started_at=started.isoformat(),finished_at=datetime.now(timezone.utc).isoformat(),
        repeats=9,warmups=1,summary=summary))
    print(f'Completed {len(rows)} focused profiles including warmups; checksums agree.',flush=True)


if __name__=='__main__':main()
