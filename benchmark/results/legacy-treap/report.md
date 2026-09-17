# Treap migration research and validation

Two distinct structures replace the obsolete key-ordered `implicit_treap.hpp`:

- `ImplicitTreap`: positions, insert/erase/get/set/prod/reverse/pointwise lazy apply, forward and backward aggregates, ACL composition order, owned index pool and recycled slots. Linear Cartesian build, iterative split/merge/set/traversal, no tree-height recursion.
- `OrderedMultiset`: stable comparator-equivalent occurrences, erase-first-equivalent, rank/count/bounds/kth and noncommutative inorder aggregate. Each duplicate owns a node; no `operator==` or default constructor requirement on the value. Recursive expected logarithmic treap height; adversarial worst depth is documented.

Independent clean implementation; no public submission source copied into library or committed benchmarks. Public sources stay under ignored `.build/legacy-treap/fastest-sources`.

## Primary-source findings (retrieved 2026-09-17 UTC)

Metadata and source hashes are in `benchmark/results/legacy-treap/fastest-metadata.json`.

- Dynamic Sequence Range Affine Range Sum [402977](https://judge.yosupo.jp/submission/402977), fetched fastest: blockwise balanced sequence with capacity 512, occupancy 8, balance 4, specialized modular affine/sum kernels and AVX2 I/O. It processes chunks rather than one node per item. [278236](https://judge.yosupo.jp/submission/278236): fixed-array rank splay with sentinels, specialized arithmetic and recursive splay. Both perform the same sequence operations; our generic node/action interface and owned/reusable pool incur additional memory and constant factors.
- Double Ended Priority Queue [403001](https://judge.yosupo.jp/submission/403001): main instantiates `partition_priority_queue` with partition size 32 (the same file also contains an unused interval heap). [343905](https://judge.yosupo.jp/submission/343905): vector min/max level heap. These avoid maintaining general order statistics or range aggregates. They are workload references, not equivalent full-feature replacements.
- Point Set Range Composite [402223](https://judge.yosupo.jp/submission/402223): fanout-16 inverse-prefix tree using nonzero slopes, batched inverses, Montgomery arithmetic, prefetching/AVX2 and optional huge pages. [278231](https://judge.yosupo.jp/submission/278231): fixed-array binary segment tree, reverse physical layout and bit-scan query traversal. They exploit fixed index positions; our multiset driver exercises dynamic key removal/insertion and noncommutative rank aggregation, so higher cost is expected. For the fixed-array task itself, use ACL segtree rather than OrderedMultiset.
- [CP-Algorithms treap](https://github.com/cp-algorithms/cp-algorithms/blob/main/src/data_structures/treap.md) and [Nyaan reversible RBST](https://nyaannyaan.github.io/library/rbst/lazy-reversible-rbst.hpp.html) supplied algorithm/API comparisons. Our reverse keeps each leaf unchanged and swaps the aggregate order; its pointwise action must commute with reversal.

A first attempted fastest compile without `-march=native` failed with the source's explicit AVX2 target requirement. The failure log is retained, and local comparisons compile every candidate with the same GCC 13, GNU++23, O2, DNDEBUG, march=native flags. No failed measurement becomes zero.

## TDD and scoped correctness evidence

- `.build/legacy-treap/tdd-red.log`: expected missing-header compile failure before implementation.
- `.build/legacy-treap/tdd-green.log`: initial randomized implementation PASS.
- Independent reviewer found qualified `std::swap` fails when std::array/tuple are declared after the header. `.build/legacy-treap/adl-red.log` records the new committed regression failing before the fix. Using std::swap plus unqualified swap restores ADL; header-first array/tuple tests now pass.
- `.build/legacy-treap/tests.log`: GCC13 and Clang18, GNU++20 and GNU++23, O2/Wall/Wextra/Wshadow/Werror, 20 fixed seeds each PASS. Sum-affine/vector oracle, noncommutative text concatenation + character-affine actions + reversal, deleted-default value/action types, optional bounds, comparator-state equivalence order, deletion/reuse, tracked lifetimes, copy/move/assignment/self-assignment and moved-from reuse.
- Same log: release standalone/double includes, both minimal docs examples and lazy-action example PASS; Clang ASan/UBSan 3 seeds PASS.
- Independent reviewer `.build/legacy-fenwick/review`: 50,000 matrix-product/conjugation steps with noncommutative action and aggregate, reverse, stable duplicate classes, ASan/UBSan PASS after removing include workaround; no open findings.
- Official `scripts/verify_with_metrics.py` four drivers, `--repeats 1 --timeout 1800 --output .build/legacy-treap/verification`: 87/87 cases PASS (dynamic affine 33, reverse/sum 20, composite 16, duplicate priority queue 18). `current.json` and `report.md` record exact compiler/input hashes. Verification used helper 60-second cutoff, not each LC problem's limit. Runs overlapped other compilation and one coordinated suspension; elapsed numbers are not clean performance measurements and must not be described as judge submission timing.

## Benchmark method

`benchmark/legacy-treap.cpp` compares independently authored same-input oracles: sequence N=50,000 Q=20,000 with vector affine/reverse, and duplicate multiset N=20,000 Q=10,000 with std::multiset linear rank/range folds. Generation and I/O are excluded, but construction/destruction included; one warmup, five alternating runs, checksum equality required. `legacy-treap.py` saves all raw samples and compiler/source hashes.

`benchmark/legacy-treap-prepare.py` generates N=Q=50,000 fixed seed 20260917 LC input files and builds the current drivers and fetched top two submissions with identical flags. `legacy-treap-fastest.py` measures end-to-end separate processes with regular-file input, warm caches after one warmup, five alternating runs and integer-token SHA-256 equality (raw output hashes retained). It includes each program's I/O differences; it does not isolate the data structure kernel or reproduce the judge hardware. Ordinary desktop background load may remain even when other agents are paused.

Final measurements are recorded in `benchmark/results/legacy-treap/` after the coordinated quiet slot. All benchmark baselines are read-only public sources, never copied into product headers.

## Final measurements and interpretation

All nine local public/candidate programs compiled successfully with matched native flags. On the fixed random workloads, every integer output token matched; the initial byte comparison failed solely because submission 278236 prints space-padded fields instead of newlines. That failed attempt is preserved as `fastest-byte-comparison-initial.json`. It was diagnosed by checking all tokens, not bypassed by ignoring differing values.

Clean-window kernel medians/minimums, milliseconds (5 measured + 1 warmup):

| Workload | Variant | Median | Minimum |
| --- | --- | ---: | ---: |
| sequence N50k/Q20k | ImplicitTreap | 37.84 | 37.13 |
| same sequence | vector brute | 152.97 | 151.43 |
| duplicate/rank/range N20k/Q10k | OrderedMultiset | 7.07 | 6.69 |
| same multiset | std::multiset linear rank/fold | 476.49 | 471.57 |

End-to-end LC-shaped N=Q=50k medians/minimums, milliseconds:

| Problem | Variant | Median | Minimum |
| --- | --- | ---: | ---: |
| Dynamic sequence affine sum | Blueberry | 240.82 | 225.64 |
| same | 402977 block sequence | 158.87 | 138.87 |
| same | 278236 splay | 167.87 | 147.51 |
| Duplicate priority queue | Blueberry | 67.20 | 60.03 |
| same | 403001 partition queue | 54.68 | 46.63 |
| same | 343905 minmax heap | 21.72 | 16.97 |
| Fixed position composite | Blueberry | 318.98 | 181.53 |
| same | 402223 inverse-prefix tree | 331.65 | 241.38 |
| same | 278231 segment tree | 517.68 | 326.16 |

These public-program runs use inputs and binaries on the Windows-mounted WSL filesystem; source-specific mmap/read behavior and process startup are included. Especially the composite figures have large dispersion. They do not justify a claim that a generic treap beats a specialized segment tree or the judge ranking. They do establish the expected tradeoff: value ownership, generic monoids/actions and order statistics have real cost relative to specialized block/heap designs. The sequence kernel is practically useful on the mixed workload without duplicating highly specialized AVX2 machinery.

The official composite worst case `max_random_01`, previously 9.99s under overlapping compilation, was repeated once in the quiet window: 4.046s with original verification flags and 3.931s with native benchmark flags, both correct and below that problem's 5s limit locally. This is single-case evidence on this WSL machine, not a promise about all judge hardware. All 87-case correctness results used the helper's 60s cutoff.

Readiness: library code, full public API docs, boundary/random/sanitizer coverage, four official drivers, standalone/double-includes, and independent review are complete. Parent owns full repository verification/docs, catalog registration, legacy deletion and commits.