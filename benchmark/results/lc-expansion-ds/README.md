# Stateful data structure selection, 2026-09-17

Three independent non-ACL additions: potential Union Find, online ordered set,
and point-update persistent segment tree. The scope does not include persistent
range affine updates or copying an interval between versions.

## Reproduction and limits

Run `python3 benchmark/run_data_structure_expansion.py` from the repository root
under Linux/WSL with GCC and `/usr/bin/time`. This builds the repository's own
comparison candidates in `benchmark/data-structure-expansion.cpp` using GCC
13.3.0, C++20, `-O2 -DNDEBUG`, and identical seeded input. Each pair is run three
times with alternating order. N=200,000. All paired checksums matched.

`environment.json` records compiler, flags, CPU and platform; `raw.jsonl` preserves
stdout/stderr/status/RSS; `runs.csv` contains every observation; `summary.json`
contains median, minimum and peak RSS. Generation is outside the measured region;
construction, operations and destruction are inside. RSS is process-wide and
includes the same generated input arrays, so it is not a per-node measurement.

Other domains' timed comparison runs had finished before this comparison began.
Official verifications still used the shared WSL host. The environment was not
isolated, so small timing differences and absolute judge-time comparisons do not
justify performance claims. These are comparison results for the chosen inputs,
not a guarantee on all inputs.

## Same-environment results and decision

| Workload | Selected median / min seconds | Alternative median / min seconds | Selected / alternative peak KiB |
| --- | ---: | ---: | ---: |
| Union Find, additive | 0.006298 / 0.005299 | 0.008713 / 0.007081 | 14908 / 15644 |
| Union Find, noncommutative matrices | 0.042611 / 0.033256 | 0.042623 / 0.041778 | 19508 / 20176 |
| Ordered set, random insert/rank/erase/reinsert | 0.625800 / 0.616856 | 0.728409 / 0.710311 | 21548 / 21160 |
| Ordered set, monotone keys with churn | 0.206317 / 0.200622 | 0.180896 / 0.162497 | 22100 / 22020 |
| Persistent tree, linear history | 0.216556 / 0.188145 | 0.560895 / 0.523765 | 80736 / 161948 |
| Persistent tree, branching history | 0.194035 / 0.177771 | 0.494477 / 0.486651 | 80104 / 161896 |

- Union Find: separate parent/potential arrays versus an independently written
  array-of-structures candidate; identical group operations and compression.
  Matrices are determinant-one 2x2 matrices over 998244353. The matrix timings
  are indistinguishable. Keep separate arrays for the smaller observed memory
  footprint and straightforward ownership, without claiming a universal speedup.
- Ordered set: index-pool treap with reusable erased slots versus a pointer treap
  with individual `new`/`delete`. The priority sequence and logical operations are
  identical. The pool won the random workload but lost the monotone workload;
  peak memory was similar because vector capacity rounds up. Keep the pool for
  allocation reuse, value semantics and good random-workload behavior. Fixed
  deterministic priorities do not provide a worst-case adversarial height bound.
- Persistent tree: append-only index pool versus a pointer path-copy tree with
  independently allocated nodes and an ownership list. Both use sparse identity
  roots and the same versions, point assignments and prefix queries. The pool
  materially reduced time and peak RSS on both histories. No fixed giant pool,
  `shared_ptr` overhead or references across reallocation are needed.

## Fastest research: inspected algorithms, not copied source

`fastest-metadata.json` was fetched on 2026-09-16 22:19 UTC using the official
Library Checker API with `scripts/fetch_lc_submissions.py`. It contains ranking
metadata and source hashes/profiles, not source text. The fastest two sources
for each problem were read to identify the actual algorithm used by `main`.
Raw source remains solely in ignored `.build/lc-expansion-ds/cache/`.

| Problem / submissions | Observed algorithm and distinction |
| --- | --- |
| [Ordered Set 403024](https://judge.yosupo.jp/submission/403024), [400970](https://judge.yosupo.jp/submission/400970) | First uses an online counted radix structure specialized to bounded 30-bit integers; second reads all queries, radix-sorts/compresses keys and uses a SIMD/multiway counting structure. The selected treap accepts arbitrary comparison keys online, including strings and custom equivalence classes. |
| [Potential UF 224108](https://judge.yosupo.jp/submission/224108), [368008](https://judge.yosupo.jp/submission/368008) | Potential-weighted compression/union plus mmap or specialized modular parsing/prefetch. The library retains caller-supplied group operations and normal standalone ownership. |
| [Noncommutative UF 393949](https://judge.yosupo.jp/submission/393949), [236404](https://judge.yosupo.jp/submission/236404) | Generic group compression with determinant-one matrix inverse, together with specialized input. The same algebraic direction is supported by the short merge/diff API. |
| [Rectangle Sum 393934](https://judge.yosupo.jp/submission/393934), [220264](https://judge.yosupo.jp/submission/220264) | Actual main functions use an offline x-sweep with Fenwick/word-block prefix storage. A bundled persistent implementation in the first source is not its active algorithm. Static offline rectangle sum can exploit lower memory; this library instead exposes arbitrary version branching. |
| [Persistent Range Affine 401007](https://judge.yosupo.jp/submission/401007), [393912](https://judge.yosupo.jp/submission/393912) | Persistent lazy propagation and cross-version interval copying, large preallocated pools/reserves, specialized I/O. This richer problem is explicitly outside the point-update API and is not claimed as verification. |

Primary comparison libraries read for APIs/layouts: [Nyaan potential UF](https://github.com/NyaanNyaan/library/blob/master/data-structure/union-find-with-potential.hpp),
[mtsaka group UF](https://mtsaka.github.io/library/ds/union-find/weighted-union-find.hpp),
[KACTL treap](https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/Treap.h),
[suisen index-pool treap](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/datastructure/bbst/implicit_treap.hpp),
[ei1333 persistent tree](https://github.com/ei1333/library/blob/master/structure/segment-tree/persistent-segment-tree.hpp),
[Nyaan persistent tree](https://github.com/NyaanNyaan/library/blob/master/segment-tree/persistent-segment-tree.hpp).
These informed algorithm and API choices; no external implementation was copied.

The official drivers use `unionfind_with_potential`, its noncommutative variant,
`ordered_set`, `rectangle_sum` through historical x-prefix versions, and
`point_set_range_composite` for noncommutative aggregation. Their local per-case
measurements are separate from the submission ranking and from this benchmark.
