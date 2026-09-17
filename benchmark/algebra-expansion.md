# Algebra expansion: design and reproducible comparison

The new headers provide bitwise AND/OR/XOR and subset convolution, plus exact
field-based matrix determinant/inverse/rank/linear-system operations. Ordinary
polynomial convolution remains ACL's responsibility; coefficient examples and
official drivers use ACL modint.

## Primary-source research

Fastest snapshots were fetched with `scripts/fetch_lc_fastest.py` on 2026-09-17.
The raw metadata and inspected submitted sources are confined to
`.build/algebra/fastest`. No source was copied into the library.

| Problem | Inspected submissions | Algorithm or implementation distinction |
| --- | --- | --- |
| bitwise AND | [400554](https://judge.yosupo.jp/submission/400554), [362198](https://judge.yosupo.jp/submission/362198) | Superset transform with SIMD, dedicated modular arithmetic and I/O |
| bitwise XOR | [400553](https://judge.yosupo.jp/submission/400553), [357194](https://judge.yosupo.jp/submission/357194) | Hadamard butterflies; vectorization and modular-reduction constants dominate |
| subset convolution | [352953](https://judge.yosupo.jp/submission/352953), [339759](https://judge.yosupo.jp/submission/339759) | Rank-vector processing, SIMD, and dedicated transforms/arithmetic; first also uses blocked XOR-transform processing |
| determinant | [401316](https://judge.yosupo.jp/submission/401316), [361510](https://judge.yosupo.jp/submission/361510) | Blocked/vectorized elimination and deferred modular reduction |
| inverse | [401290](https://judge.yosupo.jp/submission/401290), [314161](https://judge.yosupo.jp/submission/314161) | SIMD/modulus-specific elimination |
| rank | [401291](https://judge.yosupo.jp/submission/401291), [401308](https://judge.yosupo.jp/submission/401308) | Specialized field arithmetic; second is Nim, so leaderboard time is not a portable-C++ comparison |
| linear equations | [401292](https://judge.yosupo.jp/submission/401292), [401312](https://judge.yosupo.jp/submission/401312) | Same exact-field task with specialized arithmetic and language/runtime differences |

The subset algorithm follows the ranked zeta/Möbius approach from
[Björklund–Husfeldt–Kaski–Koivisto (2006)](https://arxiv.org/abs/cs/0611101).
Its flat mask-major representation overwrites products in descending degree,
keeps two rank arrays instead of three, and skips coefficients that cannot
contribute to the final diagonal. Matrix elimination uses one field inverse per
pivot and contiguous row-major storage; no CPU-specific arithmetic is imposed
on the public generic field API.

OR has no separate official Library Checker problem in the examined inventory.
`bitwise-or-via-and.test.cpp` maps official AND cases through bit complementation
and runs the OR API; independent direct OR brute-force tests supplement it.

## Comparison harness

Run from the worktree root on Linux/WSL:

```sh
python3 benchmark/run_algebra_expansion.py
```

All candidates use GCC with `-std=gnu++20 -O2 -DNDEBUG`, the same ACL revision,
and seed 1729. One warmup precedes five measured runs, alternating candidate
order in separate processes. The JSON records compiler/CPU/OS/flags, source
hashes, checksums, individual runs, median/min time and peak RSS.

- AND/XOR: 2^18 coefficients, blocked butterfly loops versus a branch-per-mask
  textbook implementation.
- Subset: 2^18 coefficients, packed two-array/rank-bounded implementation versus
  a full three-array rank-major textbook implementation.
- Matrix determinant: 500×500 dense random field matrix, contiguous storage
  versus an independently written row-vector Gaussian elimination.

Input generation is excluded. Necessary input copies, construction, output
checksum and destruction are included; RSS includes inputs and the process.
These are same-environment local reference comparisons, not recompiled public
submissions and not cross-machine comparisons with displayed Fastest times.

## Quiet-slot result (2026-09-17)

[Raw measured runs and environment](results/algebra-expansion.json): GCC 13.3.0,
Intel Core i7-14650HX, WSL2 Ubuntu. Other local compile/test/benchmark jobs were
paused. All candidate checksums matched. Five measured runs after one warmup:

| Operation | New median / min ms | Reference median / min ms | New / reference peak KiB (median) |
| --- | ---: | ---: | ---: |
| AND, N=2^18 | 5.30 / 5.12 | 8.41 / 7.83 | 7,348 / 7,368 |
| XOR, N=2^18 | 7.52 / 7.37 | 10.38 / 10.05 | 7,336 / 7,336 |
| subset, N=2^18 | 107.39 / 96.57 | 177.23 / 163.91 | 45,000 / 64,696 |
| determinant, 500×500 | 40.86 / 39.41 | 41.39 / 39.28 | 7,080 / 7,100 |

Blocked bitwise loops and bounded in-place rank processing improved these
workloads relative to their simple references. Subset peak RSS was also lower.
Matrix results are close and do not support claiming a meaningful speedup;
its value is the common exact-field API, explicit shape handling and ownership.
None of these comparisons establishes parity with modulus-specific SIMD
Fastest submissions. Such a specialization would require a separate API/design
decision and equal-environment comparison.

## Verification

`tests/random/algebra-expansion.cpp` compares bitwise/subset operations with
quadratic pair enumeration. It checks matrix rank through the exhaustive image
over F3, enumerates every solution vector to test completeness and independence
of returned affine bases, expands determinants over permutations, and multiplies
inverse results. Empty shapes, zero columns, singular/inconsistent systems,
different characteristics, signed-ring coefficients and copy/move are covered.

The initial regression compile failed on the missing new header before
implementation. GCC 13/Clang 18 × gnu++20/23 passed seeds 1, 2, 3, 17 and
998244353, all minimal documentation examples, and standalone/repeated release
includes. Clang ASan/UBSan seed 42 passed. Raw checks are in
`.build/algebra/check.log`.

The first official run lacked the ACL include-path environment and failed
before case execution. `.build/algebra/official.log` preserves that failure;
the corrected run uses `.build/algebra/official-resumed` and a distinct log.
Failed/missing timings are not replaced by zero or counted as successful runs.

The resumed run passed all 164 official cases: AND 13, OR-via-AND 13, XOR 13,
subset 11, determinant 25, inverse 26, rank 36 and linear systems 27. It used
one repetition with the repository runner's 60-second per-case timeout.
These concurrent-development timings are correctness evidence, not the quiet
benchmark comparison above.

An independent reviewer additionally enumerated 5,054 GF2 systems and all GF2
convolution input pairs of size at most 8 under ASan/UBSan, with no findings.
Review artifacts are `.build/operation-structures/review-math.cpp` and its log.
