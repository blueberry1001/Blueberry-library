# Polynomial libraries, fourth expansion

## Algorithm investigation

Read on 2026-09-18:

- [Nyaan multipoint evaluation](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html): product/remainder tree with a direct evaluation cutoff and an optional NTT-specialized product construction.
- [Luzhiled multipoint evaluation](https://ei1333.github.io/library/math/fps/multipoint-evaluation.hpp.html): simple product tree and all-leaf remainder propagation.
- [Luzhiled polynomial interpolation](https://ei1333.github.io/library/math/fps/polynomial-interpolation.hpp.html): derivative evaluation and weighted product-tree reconstruction.
- [Luzhiled sample shift](https://ei1333.github.io/library/math/combinatorics/sample-point-shift.hpp): consecutive-point Lagrange weights reduced to convolution.

These are algorithm references, not copied source. All new code and benchmark candidates were independently written. Nyaan's general-point evaluator also supplies an alternative to specialized sample shifting when combined with interpolation. ACL remains responsible for coefficient arithmetic and convolution.

Multipoint evaluation uses O((N+M) log²(N+M)) arithmetic and O(N+M log M) memory. A fixed small-leaf Horner cutoff avoids tiny divisions. Direct Horner evaluation is O(NM) time, O(M) output memory. Interpolation uses O(N log² N + N log mod) time and O(N log N) memory, versus the independently implemented O(N²+N log mod) synthetic-division Lagrange candidate using O(N) memory. Sample shift uses O((N+M) log(N+M)+log mod) time and O(N+M) memory, versus the general interpolation/evaluation pipeline with its logarithmic extra factor and tree storage. Memory figures are structural bounds, not measured peak RSS.

The selected sample shift batches reciprocals with one inversion per non-overlapping segment, copies known samples directly, and splits at modulus wrap. This removes zero-denominator cases without imposing restrictions on the starting point. General evaluation permits repeated points; interpolation requires distinct points.

## Comparable local candidate measurements

Source: `benchmark/batch-four-math.cpp`. Seed 1729, modulus 998244353, random coefficients, points 0 through N-1, shift start N and output length N. Every result is checked for equality. All candidates run in one executable with GCC 13.3.0, `-std=gnu++20 -O2`, identical ACL, CPU and WSL Ubuntu environment, three serial runs, no warmup. Raw per-run milliseconds and environment are preserved under `benchmark/batch-four-math-*`.

| Operation / candidate | N=512 median (min) ms | N=2048 median (min) ms |
| --- | --- | --- |
| Evaluation, Horner | 0.974824 (0.950486) | 16.1849 (15.799) |
| Evaluation, remainder tree | 0.411674 (0.407891) | 2.40809 (2.38111) |
| Interpolation, quadratic | 2.75426 (2.73235) | 47.4604 (46.377) |
| Interpolation, product tree | 0.633426 (0.608518) | 3.49531 (3.41888) |
| Shift, general interpolation/evaluation | 1.04209 (1.04113) | 5.92764 (5.85142) |
| Shift, convolution | 0.052825 (0.052456) | 0.231122 (0.231043) |

These compare independently written algorithm candidates, not compiled external libraries or public judge submission timings. They justify scalable algorithms, not a claim of fastest implementation. Shared machine load and small input sizes limit extrapolation. No failed measurement was replaced with zero.

## Verification

`tests/random/math-batch-four.cpp` compares arbitrary-point evaluation to Horner, interpolation to original coefficients, and shifted values to Horner. It covers empty inputs, non-power-of-two lengths, repeated evaluation points, starts within known samples, boundary starts and modulus wrap. GCC C++20 seed 42 and Clang C++23 seed 1729 passed before the final assertion-only bound checks; final checks and official results are recorded below after completion.


Initial official run: all three targets passed with `verify_with_metrics.py --repeats 1 --timeout 900`; raw structured measurements are preserved in `benchmark/batch-four-math-official-initial.json`. This initial run is before the final assertion-only size checks and is not a repeated-performance baseline. Final GCC C++20 seed 42 and Clang C++23 seed 1729 random tests also passed after those checks. The integration run records official verification for the final tree separately.

Final targeted official run: all three targets passed again after the assertion changes (`--repeats 1 --timeout 900`). Raw results: `benchmark/batch-four-math-official-final.json`. This is a separate run from the initial snapshot. Combined repeated-inclusion and API instantiation smoke tests also passed GCC/Clang with both gnu++20 and gnu++23, `-O2 -DNDEBUG -Wall -Wextra -Wshadow -Werror`, and ACL supplied via `-isystem`.
