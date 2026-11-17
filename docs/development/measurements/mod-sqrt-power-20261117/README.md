# Modular square root: shared powers

`mod_sqrt` handles a single signed 64-bit input and a prime up to `INT32_MAX`,
returning the smaller root. For `p % 4 == 3`, test the candidate root by
squaring it instead of computing a separate Legendre symbol. Otherwise, with
`p - 1 = odd * 2^twos`, use `t = a^(odd/2)` to obtain both
`root = a*t` and `residue = root*t`. Squaring the latter `twos-1` times
tests the Legendre symbol before Tonelli–Shanks starts. The API, its domain,
and its asymptotic time and memory bounds remain unchanged.

The previously collected [fast submission 169592](https://judge.yosupo.jp/submission/169592)
uses AVX2 Montgomery arithmetic for eight independent inputs with moduli below
`2^30`. Its batch interface, smaller modulus range, and compiler target differ
from this scalar API. [Submission 215966](https://judge.yosupo.jp/submission/215966)
uses Cipolla and Montgomery arithmetic. These are reference submissions, not
a claim about the current leaderboard. The scalar Cipolla comparison here is
independently adapted from this repository's `benchmark/math-expansion.cpp`,
and uses the same full-range contract, input data and output convention as the
two Tonelli–Shanks implementations. It is not a timing of either submission.

The exact historical header is saved as `baseline-original.hpp.txt`; only its
namespace changes in `baseline.hpp.txt`. `public-bench.cpp.txt` includes the
actual updated public header and computes a checksum in the measured loop.
`public-run.py.txt --build` compiles and checks the three implementations;
running it without arguments performs the benchmark. The scripts assume the
recorded local Windows Clang and SDK paths and should be adjusted on another
machine. Compilation, checks, commands, hashes and full measurements are in
`public-build.json`, `test-results.json` and `public-results.json`. The initial
shorter exploration is retained separately in `build.json` and `results.json`.

Clang 18.0.3, MSVC ABI, `gnu++20 -O2 -DNDEBUG`, Windows x64; one pinned CPU,
4,096 fixed-seed inputs repeated 20 times, one warmup and seven measured runs,
alternating implementation order. Times are median milliseconds; allocations
and input generation are outside the timed loop. Cases include negative inputs.

| Inputs | Original | Updated | Scalar Cipolla |
| --- | ---: | ---: | ---: |
| `p=1000000007`, squares | 29.783 | 14.461 | 64.818 |
| `p=1000000007`, nonsquares | 14.451 | 14.067 | 15.033 |
| `p=1000000007`, mixed | 23.425 | 17.730 | 43.477 |
| `p=998244353`, squares | 122.628 | 119.031 | 99.656 |
| `p=998244353`, nonsquares | 13.058 | 12.476 | 13.329 |
| `p=998244353`, mixed | 67.231 | 62.600 | 55.331 |
| `p=2147483647`, mixed | 22.052 | 13.904 | 41.449 |
| `p=2013265921`, mixed | 116.362 | 114.573 | 49.547 |
| Mixed small and large moduli | 36.351 | 30.828 | 23.826 |

There is considerable run-to-run variance, so small differences are not
conclusive. Cipolla is a stronger candidate for primes with large powers of two
in `p-1`, but this experiment does not establish a portable selection rule;
the existing algorithm remains in place. Whole-process peak working set is
recorded with each run and includes the runtime and input buffers, so it is not
a measurement of incremental per-call memory.

The public-header benchmark checks exhaustive inputs for primes through 500,
including negative representatives, and 10,000 random signed inputs for each
of six larger primes against the previous implementation. The added standalone
test compares small cases against a brute-force root table and large cases
against an independent Euler-criterion existence test and root verification,
including `INT64_MIN` and `INT64_MAX`. Clang `gnu++20` and `gnu++23`, each
with and without `NDEBUG`, passed. GCC, `make test`, `make verify` and
`make docs` were not available in this Windows environment.
