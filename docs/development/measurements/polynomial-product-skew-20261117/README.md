# Polynomial product scheduling experiment (2026-11-17)

The baseline is an exact snapshot of the original public header and uses ACL convolution. The candidate study uses the same ACL convolution and preserves nonempty factor lengths, including trailing zero coefficients. Mode 1 merges shortest factors first, mode 2 combines constant factors and otherwise uses the baseline, mode 3 combines constants and uses a heap, and mode 4 uses the hybrid in `candidates.hpp`. For three or fewer factors the hybrid uses the original function. It returns the correct-length zero vector when a constant is zero, combines constants when at least half of the factors are constant, and selects a heap when there are at least eight remaining factors and the maximum nonconstant length is at least eight times the minimum. These thresholds are a measured heuristic, not a proof of optimality. The public implementation is measured separately in `public-results.json` against the same snapshot.

[emthrm's implementation](https://emthrm.github.io/cp-library/include/emthrm/math/formal_power_series/product_of_polynomial_sequence.hpp) merges the shortest factors using a priority queue but trims polynomial coefficients, unlike this API. [CP-Algorithms' Library Checker verification](https://lib.cp-algorithms.com/verify/poly/prod_sequence.test.cpp) combines constant factors and merges products of similar degree. Neither is measured here under the same conditions, and no claim of matching the current fastest submission is made. ACL provides the convolution used by both sides but no polynomial-sequence product operation.

Windows 11, Intel64 Family 6 Model 183 Stepping 1, Clang 18.0.3 targeting x86_64-pc-windows-msvc, `-std=gnu++20 -O2 -DNDEBUG`, pinned to one available logical CPU. Each process generates the same seeded inputs before the timer, then moves them into the function. Timed values include construction and all convolutions but exclude input generation and checksum; each row uses the indicated number of independent inputs. Results are medians in milliseconds over five samples after one warm-up, reversing the execution order on alternating samples. `public-results.json` contains all samples, checksums, timestamps, exit codes and process-wide peak working sets. The table compares the public header to the old snapshot.

| Workload | Inputs | Baseline ms | Hybrid ms |
| --- | ---: | ---: | ---: |
| Empty | 100000 | 3.1199 | 3.0270 |
| One length-64 factor | 8000 | 0.1950 | 0.2199 |
| Two length-8 factors | 5000 | 0.8622 | 0.8559 |
| 8192 factors of length 2 | 2 | 6.5976 | 6.7125 |
| 1024 factors of length 16 | 3 | 10.5264 | 9.9053 |
| Length 65536 then 1023 length-2 factors | 2 | 45.2864 | 10.7522 |
| 1023 length-2 factors then length 65536 | 2 | 49.6551 | 10.7738 |
| Length 65536 and 8191 constants | 2 | 3.8024 | 0.5890 |
| 8192 constants | 5 | 2.2898 | 0.6833 |
| 512 factors, 25% constants, remainder length 2 | 200 | 12.3747 | 12.3997 |
| 512 factors, 50% constants, remainder length 2 | 150 | 9.5437 | 7.9708 |
| 512 factors, 75% constants, remainder length 2 | 150 | 5.0850 | 3.2202 |
| 512 alternating lengths 2 and 16 | 15 | 17.3952 | 13.0853 |
| 512 alternating lengths 2 and 8 | 15 | 7.4628 | 7.4498 |

The table is not per-call latency: each row reports the total time for its independent inputs. Small differences can be noise. The public run also measured 512 length-2 factors with 8 constants, repeated 100 times: 6.8730 to 7.8854 ms, with overlapping individual samples (old 6.8063–8.4520, new 6.8791–8.1526 ms). Four skewed factors repeated 5000 times measured 4.1559 to 4.4903 ms, again with overlapping samples. On the first pilot, the unconditional heap slowed 8192 equal linear factors (6.755 to 7.978 ms), and unconditional constant folding slowed a single factor (0.200 to 0.975 ms). The first hybrid also slowed the 25%-constant workload (1.94 to 2.21 ms for 30 inputs), motivating the half-constant threshold. Their source snapshots, build records and raw timings are retained as `*-pilot.*` and `*-hybrid-v1.*`; the initial failed build and matching initial sources are also retained. It failed because `-Werror` exposed ACL warnings; the later build marks the repository include path as `-isystem` and succeeded.

`public-bench.cpp` checks both versions against naive multiplication on 5000 seeded random cases, plus skewed, zero-constant and trailing-zero cases. Every timed input has equal complete-output checksums across modes. `tests/random/polynomial-product.cpp` adds independent naive comparisons for three static moduli, trailing zeros, empty inputs and factors, threshold boundaries, skewed and zero factors. It passed with seeds 1, 17 and 731 under Clang gnu++20/gnu++23 and MSVC C++20, with and without `NDEBUG`; `validation.json` preserves every command and result. GCC, full make targets and Library Checker execution were not available in this Windows environment.

To reproduce in the same Windows toolchain, adjust the compiler, MSVC and Windows SDK paths in `public-run.py` and `validate.py` if necessary, then run `python docs/development/measurements/polynomial-product-skew-20261117/public-run.py --build`, `python docs/development/measurements/polynomial-product-skew-20261117/public-run.py` and `python docs/development/measurements/polynomial-product-skew-20261117/validate.py` from the repository root. `run.py` reproduces the separate candidate comparison. Other compiler versions, flags, CPUs, workloads and revisions are separate measurements.
