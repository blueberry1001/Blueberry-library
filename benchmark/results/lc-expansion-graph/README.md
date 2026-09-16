# Graph expansion: research and measurements (2026-09-17)

## Scope and source review

Added stack-safe Hopcroft–Karp with minimum vertex cover, vertex-biconnected
components with a block-cut incidence forest, and directed/undirected Euler trails.
Existing `LowLink` handles bridges, articulation points and **two-edge** components;
the new biconnected header computes **vertex** blocks and is deliberately separate.
Matching provides a direct bipartite API beyond ACL's generic maximum flow.
Cycle detection was researched but deferred to keep this expansion bounded.

Official problem specifications and reference solutions were inspected from the
[Library Checker graph directory](https://github.com/yosupo06/library-checker-problems/tree/master/graph).
Matching permits empty partitions; the library additionally accepts repeated edges.
Biconnected components follows the official loop-free multigraph convention:
bridges are two-vertex blocks and isolated vertices are singleton blocks.
Euler trails support loops and parallel edges and return original edge IDs.
Degree conditions are followed by an all-edges-consumed check; isolated vertices
do not prevent a trail. See the three API pages for precise empty/start contracts.

`submissions.json` preserves the API response metadata and source SHA-256 values.
Retrieved 2026-09-16T22:19Z, C++ AC submissions ordered by time, three distinct users
per problem; the first two sources were read. The downloaded code remains in the
ignored research directory and was not copied into implementation or benchmark.

| Problem | Selected fastest | Reported time | Observations from source inspection |
| --- | --- | ---: | --- |
| Bipartite matching | [396318](https://judge.yosupo.jp/submission/396318) | 70 ms | Flat CSR, queue-based distance-label reassignment and periodic BFS; despite the namespace name, not textbook layered HK DFS |
| Biconnected components | [287412](https://judge.yosupo.jp/submission/287412) | 68 ms | Iterative DFS, flat edge storage, edge stack, vertex stamps; special tree shortcut |
| Euler directed | [286963](https://judge.yosupo.jp/submission/286963) | 21 ms | Iterative Hierholzer, flat arrays, reserved traversal stack, degree and consumed-edge checks |
| Euler undirected | [286968](https://judge.yosupo.jp/submission/286968) | 29 ms | Same design with shared used-edge state |
| Cycle directed (deferred) | [401446](https://judge.yosupo.jp/submission/401446) | 23 ms | Packed destructive DFS storage and fast integer I/O |
| Cycle undirected (deferred) | [286590](https://judge.yosupo.jp/submission/286590) | 47 ms | Iterative three-color DFS with parent edge IDs |

These are leaderboard maximum-case times, **not** comparable with local timings.
Judge hardware, compiler, I/O and datasets may differ. Generic feature flags in
the JSON can match bundled code or commented pragmas and do not establish causes.

The second matching source [339108](https://judge.yosupo.jp/submission/339108)
uses a similar queue/relabel approach. We retained standard shortest-layer HK
to keep the complexity argument straightforward. Second BCC source
[334464](https://judge.yosupo.jp/submission/334464) uses iterative generic DFS;
Euler sources [219214](https://judge.yosupo.jp/submission/219214) and
[215569](https://judge.yosupo.jp/submission/215569) use recursion. Our headers
use heap-backed explicit stacks for deep graphs.

Other primary implementation references inspected for API/algorithm comparison:
[KACTL matching](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/HopcroftKarp.h),
[KACTL biconnected components](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/BiconnectedComponents.h),
[hitonanode matching](https://hitonanode.github.io/cplib-cpp/graph/bipartite_matching.hpp.html),
[Nyaan biconnected components](https://nyaannyaan.github.io/library/graph/biconnected-components.hpp.html),
[CP-Algorithms Euler](https://lib.cp-algorithms.com/cp-algo/graph/euler.hpp.html).
KACTL's edge callback excludes bridges, so its result convention differs here.
Parent-edge IDs are essential for correct multigraph traversal.

## Comparable local experiments

Run from the repository root with ACL available under `.deps/ac-library`:

```sh
python3 benchmark/graph-expansion-benchmark.py
python3 benchmark/graph-expansion-benchmark.py --compiler clang++
```

The runner generates nested-adjacency variants of **our own** BCC/Euler headers,
changing only adjacency construction/access. Matching compares our direct solver
with ACL `mf_graph<int>`, including residual graph/source/sink construction.
Both candidates receive the same already-generated edge vector and checksum
the result. Timed work includes construction, solving, checksum and destruction;
input generation and I/O are excluded. Euler measurements are directed only.
Candidate order alternates, with one excluded warmup and five measured runs.
Fixed `mt19937` seed 20260917; all checksums agreed.

Environment: Intel Core i7-14650HX, WSL2 Ubuntu, GCC 13.3.0 / Clang 18.1.3,
`-std=c++20 -O2 -DNDEBUG -Wall -Wextra -Werror`. Full commands and source hashes
are in `*-environment.json`, individual samples in `*-runs.csv`, and median/min
in `*-summary.json`. Other agents' benchmark runs were excluded from this final
measurement window, but official verification/compilation could still run in the
background. CPU frequency was not pinned. Treat relative results as evidence
for these shapes rather than stable absolute latency or universal dominance.

| Shape | N, M (matching N per side) | GCC selected / alternative median ms | Clang selected / alternative median ms |
| --- | --- | ---: | ---: |
| Matching sparse | 30000, 100000 | 61.606 / 132.795 | 72.299 / 179.260 |
| Matching alternating ladder | 10000, 19999 | 0.403 / 2.696 | 0.564 / 3.863 |
| Blocks path | 100000, 99999 | 15.403 / 21.589 | 13.337 / 23.522 |
| Blocks path + random chords | 100000, approximately 200000 | 15.597 / 54.027 | 15.672 / 53.941 |
| Euler path | 100000, 99999 | 2.475 / 7.157 | 3.706 / 9.200 |
| Euler closed random walk | 100000, 200001 | 16.432 / 54.739 | 34.338 / 93.126 |

The measured shapes support retaining contiguous adjacency in the immutable
graph APIs: it avoids per-vertex allocation without changing the public API.
Dedicated matching avoided generic residual-graph overhead on both shapes.
We did not import custom I/O, target pragmas or special tree cases into headers.
Minimum vertex cover and block-cut forest extraction were correctness-tested,
but are not included in these solve benchmarks.

An initial Clang benchmark build failed because ACL assert-only local variables
triggered `-Werror` with `NDEBUG`; treating the external ACL directory as a system
include resolved this without relaxing warnings on our code. The failed log is
retained in `.build/lc-expansion-graph/clang-initial-compile-failure.log`; no failed
or absent measurement was substituted with zero.

## Correctness evidence

- TDD first compile failed for missing new headers, recorded in the ignored
  `tdd-red.log`, before implementation.
- `tests/random/graph-expansion.cpp`: GCC seeds 1..50, 250 random rounds each.
  Independent subset matching, maximal vertex-block enumeration and edge-mask
  trail search compare answers. Covers empty graphs, repeated edges, loops where
  allowed, disconnected components and every explicit start on small inputs.
- Seed 1 includes 200000-vertex matching ladder, block path and Euler paths;
  GCC, Clang, and GCC ASan+UBSan runs passed without recursive-stack dependence.
- Each of the three minimal documentation examples compiled and ran with GCC
  and Clang under C++20 with `-Wall -Wextra -Werror`.
- Independent review additionally exhausted 65536 4x4 bipartite graphs, 1024
  simple five-vertex graphs, 1023 repeated-edge block graphs and 4096 tiny Euler
  conditions. No concrete finding; details remain in
  `.build/lc-expansion-math/graph-review.md`.
- Official drivers passed 22 BCC + 44 matching + 27 directed Euler + 27
  undirected Euler cases = **120 cases**. `official-verify.json` retains
  dataset/driver hashes and measured verification metadata; header hashes are
  recorded in the benchmark environment files. Its revision
  identifies the baseline working-tree revision, not a committed header version.
  These one-run correctness timings are not performance comparisons.
