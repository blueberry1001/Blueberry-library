# Berlekamp–Massey measurement (2026-11-17)

The original computes a division for every nonzero discrepancy and copies the current polynomial on every such iteration. The public version caches the inverse of the most recent degree-changing discrepancy and copies the polynomial only when the degree changes. Both updates use only the field operations already required by the generic API. The quadratic algorithm and coefficient convention remain unchanged.

[Nyaan’s implementation](https://nyaannyaan.github.io/library/fps/berlekamp-massey.hpp.html) also avoids the polynomial copy when the degree does not change, but divides for each nonzero discrepancy. [Half-GCD](https://codeforces.com/blog/entry/101850) provides a faster asymptotic alternative over suitable polynomial rings, whereas the existing API accepts arbitrary exact fields without requiring fast polynomial multiplication. This change retains that generality; it is not a claim that the quadratic implementation matches the fastest large-input submissions. A directly comparable fastest-submission measurement was not available.

Windows 11; Intel64 Family 6 Model 183 Stepping 1; Clang 18.0.3 targeting x86_64-pc-windows-msvc; `-std=gnu++20 -O2 -DNDEBUG`; affinity `0x1`; ACL `modint998244353`. Input generation is outside the timed region. Each result measures repeated construction of a complete recurrence and hashing every output coefficient, without I/O. Three variants use the same input: the original, copy-only optimization, and the checked-out public header. Each cell is the median of five timed samples after one warm-up; execution order alternates and result hashes must match. Times below are milliseconds for the **total repetitions**.

| Input | N | Repetitions | Original | Copy only | Public header |
| --- | ---: | ---: | ---: | ---: | ---: |
| Random | 16 | 10000 | 21.0804 | 18.8652 | 12.7990 |
| Random | 128 | 1000 | 23.8898 | 22.4531 | 16.7123 |
| Random | 1024 | 40 | 30.9936 | 28.3784 | 26.4466 |
| Random | 4096 | 4 | 34.9326 | 33.6954 | 33.3311 |
| Random | 10000 | 1 | 47.8636 | 48.1848 | 46.9374 |
| Fibonacci, degree 2 | 10000 | 1 | 0.0272 | 0.0293 | 0.0301 |
| Sparse, one random term per 97 | 10000 | 1 | 27.5652 | 27.7409 | 28.5096 |
| All zero | 10000 | 1 | 0.0093 | 0.0090 | 0.0099 |
| First half zero, second half random | 10000 | 1 | 25.2342 | 24.2127 | 23.5027 |

For sparse N=10000, the original sample range is 24.9008–28.2600 ms and the public range is 23.5878–30.4465 ms. The observed 3.4% median regression is smaller than the overlapping spread; this run does not establish its cause. The very short Fibonacci and zero results are likewise sensitive to noise: their absolute median differences are 0.0029 ms and 0.0006 ms per call. Low-degree and zero cases are therefore shown explicitly rather than claimed as improvements. Five distributions and all five input sizes are recorded in `results.json`.

The operation-count experiment wraps the same ACL field, counts field copy construction/assignment, explicit field multiplication and division, and is separate from timing. At random N=128 it reports copies 4672 → 2528, explicit multiplications 8193 → 8321 and divisions 128 → 64 (original → public). At N=10000 with a zero first half, divisions fall 5000 → 1 and counted copies 25020004 → 15006. Counts include vector and value operations, are specific to this instrumented wrapper and do not represent allocation counts. They explain the mechanism without implying a fixed timing improvement for every field or input.

Correctness: the final driver checks exact output equality with the original and copy-only versions for each of five input distributions at every length 0–200. The separate `tests/random/berlekamp-massey.cpp` tests non-ACL exact fields with Gaussian elimination as an independent minimum-degree oracle: all GF(2) sequences up to length 9 and 500 seeded GF(7) sequences of length below 25, plus a long GF(7) recurrence and division count check. Clang gnu++20/gnu++23 and MSVC C++20 debug/release builds pass seeds 1, 17 and 731 (`validation.json`); assertions are not used for test checks. `validation-failed.json` preserves the first build failure: with the project root in the ordinary include path, an ACL `_umul128` unused-value warning was promoted to an error. The final Clang command uses `-iquote` for the project and `-isystem` for ACL. GCC, full make targets and official Library Checker runs were unavailable on this Windows host.

`results.json` records every sample, command, output hash, affinity, operation count and source hash. `pilot.log`, `pilot3.log`, `build3.log` and `.txt` source snapshots preserve the earlier exploratory runs; timing in those logs is separate. Reproduce from the repository root with `python docs/development/measurements/bm-20261117/validate.py` and `python docs/development/measurements/bm-20261117/run.py`, adjusting the hard-coded compiler, MSVC and SDK paths for the machine. The tracked ACL dependency must be available at `.deps/ac-library`. The baseline and copy-only versions are fixed snapshots; the final variant includes the actual checked-out public header. Different machines, compilers, revisions or input distributions require separate comparisons.
