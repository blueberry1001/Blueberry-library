# Consolidated evidence reuse audit

Merged commit: `a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c`. All four measured production targets match their authoritative parent bytes. No measurement was rerun.

| Family | Reuse classification |
|---|---|
| StaticTopTree | Reference-only historical observations. Identical target, but contemporaneous ACL/toolchain provenance is incomplete; no exact full-closure claim. Release compiled a rejected SAM candidate beside STT, while assert/recheck isolated STT. |
| DynamicFenwickTree | Historical isolated-target evidence. Target/source/ACL snapshots preserved; the compiled unrelated PST header intentionally remains ca8 and differs from the merge. |
| PersistentSegmentTree | Historical isolated-target evidence. Target/source/ACL snapshots preserved; the compiled unrelated DFT header intentionally remains ca8 and differs from the merge. |
| furthest_pair | Historical final paired observations with exact matching project source/harness dependency closure. This is still not a new integrated timing or complete toolchain identity claim. |

Original artifact trees and available prepared snapshots/binaries were hash-checked without alteration. Prior whole-repository checks are not reused as integrated checks. Root performs new correctness checks in this namespace. Existing access/publication blockers remain.

Exact paths, hashes, expected unrelated-header differences and original broad-validation source mismatches are in `evidence-reuse-audit.json`.
