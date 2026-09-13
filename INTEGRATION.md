# Parallel development integration

Base: main `4448ce540b75dddc34ff40effd9d0c013808d079` (2026-09-13).
Integration branch: `work/integration`.

## PR review and merge route

| PR | Head | Decision |
| --- | --- | --- |
| #5 API / prior work | `f1efae1` | Adopt short aliases, documentation/style and legacy research CLI; replace overlapping Tree implementation with #9 |
| #6 Audit | `1e88178` | Adopt historical audit, add current roadmap status |
| #7 Verify / CI | `0a04b8a` | Adopt discovery and matrix; extend matrix to Tree/DS differential tests |
| #8 Data Structure | `5871390` | Adopt measured optimizations; fix new `-=` requirement in Fenwick get |
| #9 Tree | `6c8c5a8` | Adopt ordered HLD and RMQ LCA; migrate SparseTable call to prod |
| #10 Benchmark | `c3f6250` | Adopt runner/results; correct memory estimate after SparseTable layout change |

Use the integration PR as the single merge route. Do not independently merge overlapping
#5/#9 after this integration. #6 is independently applicable as historical documentation;
#7/#10 need the integration's Makefile/README resolutions; #8 needs its compatibility fix.
All six heads are retained in the integration ancestry. No main merge is performed here.

## Findings and fixes

- Resolved Tree header/docs/verify conflicts using #9; only one HLD and one RMQ LCA class remain.
- Git auto-merged two HLD catalog records without a conflict. `make test` caught this; deduplicated.
- SparseTable keeps both prod and product. RMQ LCA and official DS verify use prod.
  Random tests compare both aliases, including SegmentTree all_prod/all_product.
  Remaining old names in implementations/compatibility tests are intentional.
- Fenwick optimized get used `-=`, unlike the original binary `-` contract. Restored binary
  subtraction, retaining the one-traversal algorithm; added a custom additive type regression.
- Preserved the prior LCA adjacency range assertion when selecting the Tree implementation.
- HLD bool convention differs between unmerged PRs: #5 `vertex=true`, #9 `edge=false`.
  Adopted #9 consistently, documented migration: explicit bool users of #5 must invert it.
  Default vertex queries are unchanged; named vertex_query/edge_query avoid ambiguity.
- Tree/DS Python tests originally hardcoded GCC/C++20 and bypassed the compile matrix's
  random directory. They now honor CXX/CXX_STANDARD and execute in each of the four CI jobs.
- Benchmark structure estimate no longer counts the removed SparseTable log array;
  it counts flat elements and size_t offsets. Historical result files remain unmodified.
- Research CLIs overlap, but CLI arguments/report formats differ. The newer cached,
  retrying submissions client is the recommended entrypoint; older CLI remains compatible.

## Correctness, complexity and dependencies

SparseTable selects two overlapping intervals of power-of-two length; associativity and
idempotence are required. It retains O(N log N) build/space and O(1) nonempty queries.
RMQ LCA takes a minimum depth between first Euler occurrences; a nonempty singleton tree
is supported. Pair-valued SparseTable needs no external lifetime-bearing comparator.
RMQ LCA build/space is O(N log N), query/distance O(1).

HLD selects the maximum subtree child, so any light edge halves the remaining subtree
size. Build/space is O(N), paths have O(log N) intervals, ancestor/jump/LCA O(log N).
Construction is iterative. Ordered path recursion crosses light edges only and has
O(log N) stack depth. Reverse flags preserve noncommutative order. Edge queries omit LCA;
empty edge paths/subtrees invoke no callbacks. Vertex/edge position conversions are O(1).

Fenwick get subtracts earlier Fenwick blocks in the target node's range. Rollback history
restores both saved roots; equal roots identify no-op merges, which still consume one state.
SegmentTree preserves left/right order for noncommutative operations. No algorithmic
complexity regression was identified in these changes.

Supported headers use C++20 (bit_width/bit_floor/no_unique_address); C++17 is not supported.
The catalog and supported compatibility entrypoints are standalone include targets.
Tree includes SparseTable, not ACL. FPS and the umbrella entrypoint need ACL.
Six historical uncatalogued headers remain outside the supported include gate, as recorded
in ROADMAP.md; passing this gate does not claim every historical header is usable.

## Performance assessment

#8 supplies same-input/compiler interleaved measurements: SparseTable query -23.7%,
Fenwick get -59.1%, rollback history RSS -19.3%. #10's checked-in raw results independently
motivate flat layout. These measurements are prior Work evidence, not integration remeasurements.
The diagnostic blocked RMQ uses less memory but slower queries; do not promote it on a
leaderboard ratio alone. Native/SIMD, FPS backend, prime sieve, and I/O changes are deferred
until comparable measurements justify them. The integration makes no speculative optimization.

## Validation

Validation is ongoing; final results will replace this section before review readiness.
Initial preflight identified and fixed duplicate catalog entries. The first official verify
attempt failed before execution because the local user scripts directory was absent from PATH;
rerun uses PATH=/root/.local/bin:$PATH. This is not an algorithm failure.

## Remaining roadmap

See the current update at the top of ROADMAP.md. Priority: isolate/migrate six legacy headers;
cover additional FPS/Dijkstra/rollback boundaries; measure linear-space RMQ/prime enumeration;
then add non-ACL gaps such as Wavelet Matrix and offline dynamic connectivity.
