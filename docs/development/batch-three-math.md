---
title: Math expansion candidate comparison (2026-09-18)
---

## Scope and sources

Added Taylor shift, product of polynomial sequence, second-kind Stirling row,
and Montmort numbers. ACL supplies convolution and static modular arithmetic;
these four sequence/polynomial operations are not ACL APIs. All code here was
written independently from the identities below, without copying reference code.

Inspected primary implementations:

- Taylor shift: [Nyaan](https://github.com/NyaanNyaan/library/blob/master/fps/taylor-shift.hpp)
  and [maspypy](https://github.com/maspypy/library/blob/main/poly/poly_taylor_shift.hpp).
  Both reduce factorial-weighted coefficients to one convolution; one builds
  powers using inverses while the other separates powers and inverse factorials.
  We compute local factorial tables so there is no shared cache or lifetime rule.
- Product sequence: [Nyaan Pi](https://github.com/NyaanNyaan/library/blob/master/fps/fps-utility.hpp)
  and [official solution](https://github.com/yosupo06/library-checker-problems/blob/master/polynomial/product_of_polynomial_sequence/sol/correct.cpp).
  Pairwise rounds and a FIFO merge queue both avoid sequential multiplication's
  quadratic behavior for many short factors. We use pairwise rounds and move
  coefficient buffers into ACL. Degree-weighted heap scheduling could improve
  skewed inputs but adds bookkeeping; no claim of fastest skewed performance.
- Stirling row: [Luzhiled](https://github.com/ei1333/library/blob/master/math/fps/stirling-second.hpp)
  and [maspypy](https://github.com/maspypy/library/blob/main/seq/famous/stirling_number_2.hpp).
  Both convolve i^n/i! and (-1)^i/i!. Our inverse factorial table needs one
  inversion rather than an inversion per coefficient; powers use binary powering.
- Montmort: [Luzhiled](https://github.com/ei1333/library/blob/master/math/combinatorics/montmort.hpp)
  and [official solution](https://github.com/yosupo06/library-checker-problems/blob/master/enumerative_combinatorics/montmort_number_mod/sol/correct.cpp)
  use D(i)=iD(i-1)+(-1)^i. We explicitly return D(0)=1 mod m and support the
  entire positive uint32 modulus range with uint64 intermediate arithmetic.

## Comparable candidate experiment

[Benchmark source](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-math.cpp),
[raw five-run CSV](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-math-results.csv),
and [environment log](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/batch-three-math-environment.txt)
preserve inputs, flags and measurements. Compile command:

```text
g++ -std=gnu++20 -O2 -I. -I.deps/ac-library benchmark/batch-three-math.cpp -o .build/batch-three-math-bench
```

GCC 13.3, same WSL host, fixed RNG seed123, 5 runs each, no warmup removal.
The alternatives are independent straightforward implementations of Horner
substitution, sequential ACL multiplication, Stirling recurrence DP, and the
two-term derangement recurrence. Checksums agree for every pair and repetition.
Inputs and timed ownership/allocation costs match within each pair. Parallel
development load can affect these absolute times; these are algorithm-selection
measurements, not comparable with public judge times or previous revisions.

| Operation / input | Selected median / minimum (µs) | Alternative median / minimum (µs) |
| --- | --- | --- |
| Taylor shift, 2048 coefficients | 110.110 / 108.576 | 3949.660 / 3868.130 |
| Product, 512 linear factors | 95.298 / 92.905 | 255.015 / 253.964 |
| Stirling row, n=2048 | 223.877 / 223.759 | 9829.700 / 9614.370 |
| Montmort, n=1000000, m=1000000000 | 3305.750 / 3235.970 | 4794.640 / 4695.850 |

The first three choices avoid quadratic growth. Both Montmort candidates are
linear; the single-predecessor recurrence is short and measured faster here.
Storage is linear for all selected algorithms. No global fastest claim is made.

## Validation design

Official drivers correspond to `polynomial_taylor_shift` (N≤524288),
`product_of_polynomial_sequence` (including zero factors),
`stirling_number_of_the_second_kind` (n≤500000), and `montmort_number_mod`
(n≤1000000, modulus≤1000000000). The library additionally tests zero-length
sequences and modulus UINT32_MAX.

`tests/random/math_batch_three.cpp` accepts the runner seed and compares against
coefficient-by-coefficient substitution, schoolbook multiplication, partition DP,
and the independent two-term derangement recurrence. n≤9 derangements are also
checked by full permutation enumeration. Explicit empty inputs, trailing zeros,
zero shifts, skewed product sizes and NTT-sized vectors are covered. Official
execution was completed once for all four drivers, with every official case
passing. The local metrics are in `.verification/batch-three-math/current.json`;
their revision field records base HEAD 6a1e754 plus then-uncommitted additions,
not a claim that the base commit already contained these headers. GCC 13 / Clang
18 with gnu++20 / gnu++23 each passed random seeds 1, 2, 3, 7 and 23. Full-repository
checks and publication remain the integrating task's responsibility.
