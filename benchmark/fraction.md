# Fraction comparison (2026-09-17)

`python3 benchmark/run_fraction.py` compares the current Fraction, its original
128-bit Euclid implementation, and the locally installed Boost.Rational<int64_t>.
The reference snapshot is preserved in `results/legacy-fraction/results.json`, so
the runner also works from a fresh checkout. No third-party source is committed.

Input: 300,000 fixed-seed pairs, denominators 1..1,000,000 and numerators within
±1,000,000 (the divisor is nonzero). All operations are inside both libraries'
contracts. Input generation is excluded; construction and normalization are included.
GCC 13.3.0, GNU C++20, `-O2 -DNDEBUG`, WSL2 Ubuntu, i7-14650HX.
One warmup and five interleaved measured runs; agent compilation/verification was
paused. Compiler, CPU, flags, hashes, raw output/errors and source snapshots are recorded.

| Workload | Adopted 128-bit Euclid | Explicit 64-bit gcd branch experiment | Boost.Rational |
| --- | ---: | ---: | ---: |
| Construct + compare, median ms | 38.627 | 37.956 | 32.067 |
| Construct + four arithmetic results, median ms | 165.434 | 165.878 | 115.462 |

Checksums match in every run. The extra gcd branch did not materially improve the
arithmetic workload, so it was reverted. In the result JSON, `before` is the adopted
implementation and `blueberry` is the rejected experiment; `adopted_variant` makes
that distinction explicit. `before_source` and `experimental_source` retain both
implementations. The earlier independent batch is preserved under `before/` and
is not used to compute these within-batch comparisons.

Boost is faster on these bounded inputs. Fraction is adopted as a short standalone
value type with signed 128-bit intermediates, explicit int64 representability
contracts, and support for reducible INT64_MIN denominators. This is not a claim
of being the fastest general rational implementation. Large arithmetic workloads
should account for gcd cost; comparison alone uses exact cross products.

Correctness is checked separately with an unbounded Boost.Multiprecision oracle,
integer endpoints and cancellation, all comparisons/compound operators, and the
official Sort Points by Argument checker. The official problem verifies construction
and comparison; the random oracle covers all arithmetic operations.
