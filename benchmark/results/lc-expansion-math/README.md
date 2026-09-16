# Mathematical library expansion: implementation choices

Measured on 2026-09-17 JST with GCC 13.3.0, `-std=gnu++20 -O2 -DNDEBUG`,
WSL2 on Intel Core i7-14650HX. `results.json` records full flags, CPU, source
hashes and raw samples; `raw.tsv` includes the warmup. Run from the repository
root with `python3 benchmark/math-expansion.py` (ACL in `.deps/ac-library`).

Each comparison uses identical generated inputs and compiler settings, one
warmup, five samples, and alternating order. This is algorithm-only timing;
input generation, input/output and equality checks are outside the timed region.
The machine also ran official verification, so CPU scheduling affects samples.
The numbers justify broad choices, not precise percentage claims.

| Workload | Implementation | Median ms | Minimum ms |
| --- | --- | ---: | ---: |
| 60 semiprimes, each factor around 1–2 million | Pollard–Brent (library) | 2.504 | 2.419 |
| same | Pollard–Floyd, gcd every step (comparison) | 19.165 | 16.401 |
| 4,000 mixed residues, mod 998244353 / 1000000007 | Tonelli–Shanks (library) | 3.832 | 3.511 |
| same | scalar Cipolla (comparison) | 3.947 | 3.528 |
| recurrence d=128, k=10^18 | ACL convolution Bostan–Mori (library) | 2.351 | 1.793 |
| same | quadratic Kitamasa (comparison) | 11.131 | 6.226 |
| random prefix length 10,000 | quadratic Berlekamp–Massey | 152.252 | 144.756 |

Comparison candidates are independently written in `benchmark/math-expansion.cpp`.
Equality is checked before measuring. Brent reduces gcd frequency; its two
implementations use different local random walks, so this is a comparison of
complete candidates, not an isolated estimate of the benefit of batching alone.
Tonelli and Cipolla are close at this workload, and the observed difference is
too small for a speed claim. Tonelli was retained for its small scalar API and
clear bounds. Bostan–Mori scales to the official degree 100,000, which quadratic
Kitamasa does not. BM's length 10,000 workload remains practical without adding
half-GCD and an additional polynomial arithmetic framework.

## Fastest research

`fastest-metadata.json` preserves three AC submission records per problem,
top-two source SHA-256 hashes, URLs and the retrieval timestamp. The sources
themselves remain in ignored `.build/lc-expansion-math/fastest-sources/`; none
were copied into product or comparison code. These leaderboard rows may use
different testcase revisions (`is_latest` is preserved), compilers or machines.
Their maximum-case time is not comparable with the local workload above.

| Problem | Inspected fastest | Observed design / decision |
| --- | --- | --- |
| primality_test | [365922](https://judge.yosupo.jp/submission/365922) | two witnesses, one selected by a large hash table, Montgomery; use established seven witnesses and full uint64 arithmetic instead of importing an unproven table |
| factorize | [317402](https://judge.yosupo.jp/submission/317402) | Montgomery, two simultaneous rho sequences, batches of 600; retain standard Brent batching and bounded restarts |
| sqrt_mod | [169592](https://judge.yosupo.jp/submission/169592) | eight SIMD lanes for varying moduli; unsuitable as a mandatory dependency for a scalar API |
| find_linear_recurrence | [282926](https://judge.yosupo.jp/submission/282926) | half-GCD and custom NTT; asymptotic improvement is real, but not required for the measured N=10,000 scope |
| kth_term_of_linearly_recurrent_sequence | [400681](https://judge.yosupo.jp/submission/400681) | Bostan–Mori, cached transforms, NTT doubling and AVX2; use ACL convolution before considering a specialized backend |
| matrix_det | [401316](https://judge.yosupo.jp/submission/401316) | Montgomery/SIMD and several pivot updates together; deferred with the matrix API |
| inverse_matrix | [401290](https://judge.yosupo.jp/submission/401290) | Schur complement, Strassen products, Gaussian fallback; deferred with the matrix API |

Representative references read: Nyaan's `prime/miller-rabin.hpp`,
`prime/fast-factorize.hpp`, `modulo/mod-sqrt.hpp`, `fps/berlekamp-massey.hpp`;
emthrm's Berlekamp–Massey and Bostan–Mori; ei1333's `math/matrix/matrix.hpp`.
Reference limitations were checked: Nyaan's factorization routine stops above
2^62 even though its primality routine has a larger-range fallback. The new
library explicitly covers all uint64 values and tests maximum-range factors.

## Validation and reproduction

`tests/random/math-expansion.cpp` uses trial division, exhaustive modular roots,
Gaussian consistency for minimum recurrence degree, direct sequence generation
and independent Kitamasa reduction. The initial compile failed with the expected
missing-header error before implementation (`.build/lc-expansion-math/tdd-red.log`).
GCC/Clang × C++20/23 each passed seeds 1–20; release standalone/double includes
and the three documentation examples passed; Clang ASan/UBSan passed seeds 1–3.

All five official drivers passed three repetitions (95 cases, 285 executions).
`official-summary.json` retains compiler/flags, dataset identities and run times.
The worst cases across all three repetitions were 0.168 s (factorization),
0.160 s (BM), 2.792 s (kth), 0.365 s (square root) and 0.751 s (primality).
These are local timings, not measurements from the Library Checker judge host.
The metrics runner's generic 60 s timeout differs from official problem limits;
all observed cases also fit their respective official 5 s / 10 s limits.

Reproduce the official measurements with:

```sh
CPLUS_INCLUDE_PATH=.deps/ac-library python3 scripts/verify_with_metrics.py \
  verify/math/primality-test.test.cpp verify/math/factorize.test.cpp \
  verify/math/mod-sqrt.test.cpp verify/math/find-linear-recurrence.test.cpp \
  verify/math/kth-term-of-linearly-recurrent-sequence.test.cpp \
  --repeats 3 --timeout 1800 --output .build/lc-expansion-math/verification
```

The public API is intentionally constrained: modular square root requires a prime
modulus at most INT32_MAX; BM requires a field; kth-term evaluation requires an
ACL static prime modint and a supported NTT length. These assumptions, empty
inputs, coefficient order, ownership and overflow boundaries are in the API docs.
