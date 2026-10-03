#!/usr/bin/env python3
"""Read-only source/artifact comparison; writes only this audit's JSON/notes."""
import hashlib
import json
import pathlib
import re
import subprocess
from datetime import datetime, timezone

ROOT = pathlib.Path('/workspace/Blueberry-library')
OUT = ROOT / '.verification/consolidated-library-2026-10-03'
POINT = '2958f619020f6421870bba5173b6d7a3614e8be4'
FURTHEST = subprocess.check_output(['git', 'rev-parse', 'e7c4e08'], cwd=ROOT, text=True).strip()
MERGE = 'a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c'

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)

def digest(data):
    return hashlib.sha256(data).hexdigest()

def sha(path):
    return digest((ROOT / path).read_bytes())

def read(path):
    return json.loads((ROOT / path).read_text())

def binding(path, expected):
    file = ROOT / path
    actual = digest(file.read_bytes()) if file.is_file() else None
    return dict(path=path, expected_sha256=expected, actual_sha256=actual, matches=actual == expected)

def committed_sha(revision, path):
    return digest(git('show', revision + ':' + path))

def library_closure(path):
    result, pending = {}, [path]
    while pending:
        name = pending.pop()
        if name in result:
            continue
        data = (ROOT / name).read_bytes()
        result[name] = digest(data)
        pending += re.findall(r'^#include\s+"(blueberry/[^"]+)"', data.decode(), re.M)
    return result

def artifact_integrity(revision, prefixes):
    names = git('ls-tree', '-r', '--name-only', revision).decode().splitlines()
    selected = [p for p in names if any(p.startswith(prefix) for prefix in prefixes)]
    tree = {line.split('\t', 1)[1]: line.split()[2] for line in git('ls-tree', '-r', revision).decode().splitlines()}
    mismatches, aggregate = [], []
    for path in selected:
        file = ROOT / path
        if not file.is_file():
            mismatches.append(dict(path=path, reason='missing'))
            continue
        data = file.read_bytes()
        blob = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
        if blob != tree[path]:
            mismatches.append(dict(path=path, expected_git_blob=tree[path], actual_git_blob=blob))
        aggregate.append(path + '\0' + digest(data))
    return dict(authoritative_revision=revision, prefixes=prefixes, tracked_files_compared=len(selected),
                exact_authoritative_blob_match=not mismatches, mismatches=mismatches,
                sorted_path_and_sha256_catalog_digest=digest('\n'.join(sorted(aggregate)).encode()),
                method='Every selected current file hashed as a Git blob and compared with the authoritative immutable tree. Aggregate is SHA256 of sorted path-NUL-fileSHA256 lines. No original file changed.')

def preparations(paths):
    checks, metadata = {}, []
    for path in paths:
        data = read(path)
        metadata.append(dict(path=path, sha256=sha(path), compile_entries=len(data.get('entries', []))))
        for name, expected in data.get('header_sha256', {}).items():
            checks[name] = binding(name, expected)
        for entry in data.get('entries', []):
            name = entry.get('binary')
            if name and entry.get('binary_sha256'):
                checks[name] = binding(name, entry['binary_sha256'])
        for name, expected in data.get('bound_sha256', {}).items():
            checks[name] = binding(name, expected)
    return dict(manifests=metadata, checked_paths=len(checks), all_available_and_matching=all(x['matches'] for x in checks.values()),
                failures=[x for x in checks.values() if not x['matches']],
                checked_path_hash_catalog_digest=digest(json.dumps(checks, sort_keys=True).encode()))

def candidate_headers(path):
    result=[]
    for name, expected in read(path)['header_sha256'].items():
        if '/candidate/include/' in name:
            production=name.split('/candidate/include/',1)[1]
            result.append(binding(production,expected))
    return result

def historical_validation(path):
    data=read(path)
    checked=[binding(p,h) for p,h in data.get('source_sha256',{}).items()]
    return dict(path=path, sha256=sha(path), recorded_source_paths=len(checked),
                current_source_mismatches=[x for x in checked if not x['matches']],
                reuse='Historical scoped results remain preserved. Recorded whole-repository verification is not reused as a check of the merged repository; changed broad-manifest paths are disclosed, not rewritten.')

assert git('rev-parse','HEAD').decode().strip() == MERGE
assert git('rev-parse','improve/point-access-performance').decode().strip() == POINT
assert git('rev-parse','wip/furthest-pair').decode().strip() == FURTHEST
families={}

stt_target='blueberry/graph/static-top-tree.hpp'
stt_paths=['benchmark/results/static-top-tree-sam/prepared.json','benchmark/results/static-top-tree-assert/prepared.json','benchmark/results/static-top-tree-assert-recheck/prepared.json']
stt_headers=candidate_headers(stt_paths[0])
stt_assert_headers=candidate_headers(stt_paths[1])
stt_sources=read('benchmark/results/static-top-tree-sam/final-runner-sha256.json')['sha256']
families['static_top_tree']=dict(
    classification='reference_only_historical_observations_identical_target_missing_historical_dependency_provenance',
    target=binding(stt_target,committed_sha(POINT,stt_target)), project_target_transitive_closure=library_closure(stt_target),
    reproduction_sources=[binding(p,h) for p,h in stt_sources.items()],
    historical_prepared_source_hash=read(stt_paths[0])['source_sha256'],
    prepared=preparations(stt_paths),
    release_candidate_project_headers_vs_integrated=stt_headers,
    assert_candidate_project_headers_vs_integrated=stt_assert_headers,
    exact_full_dependency_closure_proven=False,
    contemporaneous_acl_hashes_recorded=False,
    limitations=[
        'Release candidate changed StaticTopTree and rejected SAM clone-move together. Workloads were separated, but binary layout was not fully isolated; current production SAM intentionally differs from that rejected candidate.',
        'Assert experiment changes StaticTopTree alone; assert focused recheck reused those exact binaries. No isolated STT-only release timing exists in this record set.',
        'Harness includes atcoder/modint, but historical prepared records omit ACL hashes/revision and include environment. Later ACL hashes cannot retroactively close that provenance gap.',
        'Compiler version/flags/binary hashes were recorded, but executable/standard-library/runtime contents were not. Final Python reproduction-source hashes are not proof of the exact runner used for every earlier run.',
        'No fresh integrated measurement or exact full-closure reuse claim; no benchmark rerun merely to fill missing provenance.'
    ],
    immutable_artifacts=artifact_integrity(POINT,['benchmark/results/static-top-tree-sam/','benchmark/results/static-top-tree-assert/','benchmark/results/static-top-tree-assert-recheck/','benchmark/results/suffix-automaton-insertion/','benchmark/results/suffix-automaton-insertion-assert/','benchmark/results/static-tree-string-tuning/']),
    validation=historical_validation('benchmark/results/static-tree-string-tuning/validation.json'))

for family, target in [('dynamic-fenwick','blueberry/data-structure/dynamic-fenwick-tree.hpp'),('persistent-segment','blueberry/data-structure/persistent-segment-tree.hpp')]:
    paths=[f'benchmark/results/point-access-performance/{family}-{mode}/prepared.json' for mode in ['release','assert']]
    prepared=read(paths[0]); source=prepared['source_sha256']
    candidate=candidate_headers(paths[0])
    runner='benchmark/point-access-performance.py'
    runner_expected=committed_sha(POINT,runner)
    amendment_paths=[f'benchmark/results/point-access-performance/{family}-{mode}/runner-amendment.json' for mode in ['release','assert']] if family=='dynamic-fenwick' else []
    acl=[binding(p,h) for p,h in prepared['acl_sha256'].items()]
    prefixes=[f'benchmark/results/point-access-performance/{family}-',f'benchmark/results/point-access-validation/{family}/']
    families[family.replace('-','_')]=dict(
        classification='historical_isolated_target_evidence_project_target_closure_exact_but_full_live_translation_unit_differs',
        target=binding(target,prepared['production_header_sha256']), project_target_transitive_closure=library_closure(target),
        harness_and_support_sources=[binding(p,h) for p,h in source.items()],
        measurement_runner=binding(runner,runner_expected),
        preparation_runner_sha256=prepared['runner_sha256'],
        runner_amendment_chain=[dict(path=p,sha256=sha(p),**read(p)) for p in amendment_paths],
        prepared=preparations(paths),
        candidate_project_headers_vs_integrated=candidate,
        full_live_candidate_project_header_set_equal=all(x['matches'] for x in candidate),
        recorded_acl_hashes=dict(paths=len(acl), all_match=all(x['matches'] for x in acl),failures=[x for x in acl if not x['matches']],
            authoritative_hash_map_path=paths[0],hash_map_key='acl_sha256',map_sha256=digest(json.dumps(prepared['acl_sha256'],sort_keys=True).encode())),
        limitations=[
            'Frozen candidate include roots intentionally keep other data structures at ca8 baseline. The now-improved other point-access header differs from that snapshot. This does not invalidate the recorded isolated-target experiment, but it is not a timing of a freshly built combined repository.',
            'The target header is self-contained except standard headers; its implementation bytes and measured target-source contract match the merge. Original raw negatives and separate rechecks remain unpooled.',
            'Compiler/runtime environment remains the historical one. No new integrated elapsed-time assertion or whole-repository verification reuse.'
        ],
        immutable_artifacts=artifact_integrity(POINT,prefixes),
        validation=historical_validation(f'benchmark/results/point-access-validation/{family}/validation.json'))

furthest_target='blueberry/geometry/furthest-pair.hpp'
fp=read('benchmark/results/furthest-pair-resume/final/prepared.json')
project_bindings={p:h for p,h in fp['bound_sha256'].items() if p.startswith('blueberry/') or p in ['benchmark/furthest-pair-resume.cpp','benchmark/furthest-pair-resume.py','benchmark/furthest-pair-final.py']}
fm=read('benchmark/results/furthest-pair-resume/validation/manifest.json')
families['furthest_pair']=dict(
    classification='historical_exact_project_measurement_source_closure_matches_integrated_no_new_timing_claim',
    target=binding(furthest_target,fp['production_header_sha256']), project_target_transitive_closure=library_closure(furthest_target),
    project_measurement_source_closure=[binding(p,h) for p,h in project_bindings.items()],
    exact_project_measurement_source_closure_equal=all(binding(p,h)['matches'] for p,h in project_bindings.items()),
    prepared=preparations(['benchmark/results/furthest-pair-resume/final/prepared.json']),
    initial_preparation=dict(path='benchmark/results/furthest-pair-resume/initial/prepared.json',sha256=sha('benchmark/results/furthest-pair-resume/initial/prepared.json'),
        note='Initial layout comparison deliberately measured the old production header plus wrappers. Final paired experiment alone binds the adopted direct public header.'),
    limitations=['No ACL source dependency. Standard-library/compiler/runtime content hashes were not recorded, so exact project closure is not an assertion of complete system/toolchain byte identity.',
        'Historical final paired measurements and focused checks remain source-bound evidence. Full historical manifest includes other old library headers that correctly differ after consolidation; do not promote its whole-repository check to the merged tree.'],
    immutable_artifacts=artifact_integrity(FURTHEST,['benchmark/results/furthest-pair-resume/','benchmark/furthest-pair-resume','benchmark/furthest-pair-final.py','benchmark/furthest-pair-report.py','docs/development/furthest-pair.md']),
    validation=historical_validation('benchmark/results/furthest-pair-resume/validation/manifest.json'))

critical_pass=all(
    f['target']['matches'] and f['prepared']['all_available_and_matching'] and
    f['immutable_artifacts']['exact_authoritative_blob_match'] and
    all(x['matches'] for key in ['reproduction_sources','harness_and_support_sources','project_measurement_source_closure'] for x in f.get(key,[])) and
    f.get('measurement_runner',{'matches':True})['matches'] and
    f.get('recorded_acl_hashes',{'all_match':True})['all_match']
    for f in families.values())
assert git('rev-parse','HEAD').decode().strip() == MERGE
report=dict(schema_version=1, recorded_at=datetime.now(timezone.utc).isoformat(), scope='Read-only postmerge evidence/source audit; no builds, test executions, timings, network, archives, commits or original evidence writes.',
    integrated_commit=MERGE, branch=git('branch','--show-current').decode().strip(), source_parents=dict(point_access=POINT,furthest_pair=FURTHEST),
    critical_source_snapshot_and_artifact_checks_pass=critical_pass, families=families,
    global_limits=['All timings remain observations made at their original dates, flags, inputs and shared-host conditions. Hash equality does not create a new timing result.',
        'STT missing historical ACL provenance is retained, not patched with later evidence. DFT/PST intentional unrelated-header snapshots are retained.',
        'No whole-repository validation from either parent is presented as validation of this merge. New integrated correctness runs are owned by root in this new namespace; their completion is not asserted by this audit.',
        'Fastest/AOJ access failures and publication/measurement gate limitations are unchanged; this audit does not bypass or satisfy them.'],
    new_integrated_checks=dict(owner='root',namespace='.verification/consolidated-library-2026-10-03/',status='separate_in_progress_not_reused_from_historical_records'),
    benchmark_rerun_required_for_this_source_unchanged_consolidation=False,
    rerun_rationale='No measured target or its project transitive dependency changed. Reuse preserves the explicitly limited historical classifications above; no new performance claim is made.',
    audit_script_sha256=sha(str(pathlib.Path(__file__).relative_to(ROOT))))
(OUT/'evidence-reuse-audit.json').write_text(json.dumps(report,indent=2)+'\n')
notes=['# Consolidated evidence reuse audit','',f'Merged commit: `{MERGE}`. All four measured production targets match their authoritative parent bytes. No measurement was rerun.','',
'| Family | Reuse classification |','|---|---|',
'| StaticTopTree | Reference-only historical observations. Identical target, but contemporaneous ACL/toolchain provenance is incomplete; no exact full-closure claim. Release compiled a rejected SAM candidate beside STT, while assert/recheck isolated STT. |',
'| DynamicFenwickTree | Historical isolated-target evidence. Target/source/ACL snapshots preserved; the compiled unrelated PST header intentionally remains ca8 and differs from the merge. |',
'| PersistentSegmentTree | Historical isolated-target evidence. Target/source/ACL snapshots preserved; the compiled unrelated DFT header intentionally remains ca8 and differs from the merge. |',
'| furthest_pair | Historical final paired observations with exact matching project source/harness dependency closure. This is still not a new integrated timing or complete toolchain identity claim. |','',
'Original artifact trees and available prepared snapshots/binaries were hash-checked without alteration. Prior whole-repository checks are not reused as integrated checks. Root performs new correctness checks in this namespace. Existing access/publication blockers remain.','',
'Exact paths, hashes, expected unrelated-header differences and original broad-validation source mismatches are in `evidence-reuse-audit.json`.']
(OUT/'evidence-reuse-notes.md').write_text('\n'.join(notes)+'\n')
print('critical_pass',critical_pass)
for name,f in families.items():
 print(name,'artifacts',f['immutable_artifacts']['tracked_files_compared'],'prepared_paths',f['prepared']['checked_paths'],'prepared_failures',f['prepared']['failures'],'broad_validation_source_differences',len(f['validation']['current_source_mismatches']))
print('audit_sha256',sha('.verification/consolidated-library-2026-10-03/evidence-reuse-audit.json'))
