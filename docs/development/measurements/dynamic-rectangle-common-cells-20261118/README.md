# Online 2D Fenwick common-cell experiment (2026-11-18)

The candidate merges the descending x Fenwick paths for `left` and `right`. When the paths reach the same cell it shares the hash lookup and the two y-prefix calculations, then accumulates them separately into the four original prefix results. Each prefix's own accumulation order and the final `ru - lu - rd + ld` expression are preserved for ordinary value types. The tree still supports unknown online update coordinates and has the same API, asymptotic bounds and stored state. Operations on distinct accumulators are interleaved and a shared prefix result is reused, so behavior of user-defined arithmetic with externally observable side effects was not assessed.

**The candidate was not adopted.** A direct WSL comparison on all 14 official Rectangle Sum cases, GCC 13.3 and CPU affinity 0, alternated the old and candidate implementations three times each; all 84 cases passed. The median of each run's worst case was 6.9781 → 7.4438 seconds, and median total solution time was 51.9216 → 54.6491 seconds. Those WSL results are a separate same-environment comparison on official data, not measurements from the Windows synthetic workloads below. The associated report, case results, logs, sources and hashes are retained in `official/`; the large raw oj JSON files remain in WSL `.verification/official-mode5-20261118/` and are identified by SHA-256 in `official/raw-oj-manifest.json`. Local timings and comparison with the problem's time limit are not a submitted judge verdict.

The official comparison used the unchanged driver from revision `179d99a`, before its coordinate-boundary fix. The same checker path and input/output dataset hash were checked against the earlier WSL run, but the earlier checker binary's hash was not recorded. This comparison pins one CPU whereas the earlier screening did not, so their timings are not combined into a before/after comparison. `official/independent-audit.json` records a separate check of all six raw-log hashes, 84 case results, 28 input/output file hashes and the current checker hash; `official/audit.py.txt` preserves the audit procedure.

## Implementations and validation

`baseline-original.hpp.txt` is the original public header at revision `179d99a`, Git blob `a854af1c6a4d6fb77098b122bbbfc67873e04596`. `prepare.py.txt` generates separate namespaces from that source. Mode 0 is the old implementation; mode 4 shares hash lookups but evaluates both y prefixes separately for each x path; mode 5 also merges common x cells. Modes 1–3 are the earlier arithmetic-reordering experiments and were compiled and checked, but not timed in this study. The earlier measurements and rejection reasons are in [the separate study](../dynamic-rectangle-sum-study-20261118/README.md).

The Windows build passed fixed seeds 1, 17 and 731 against a naive grid for signed integers, ACL modint and a two-component additive group with no `-=` operator. It covers updates, duplicates, negative values, empty rectangles and `LLONG_MAX` dimensions. Modes 4 and 5 also match the baseline bit-for-bit on seeded floating-point queries and pass a signed-integer boundary example where the regrouped modes can overflow. `build-mergedwin.json` preserves the full compiler command, source hashes, output and test exit statuses. The same three seeds also passed WSL GCC 13.3 with `-O1 -fsanitize=undefined -fno-sanitize-recover=undefined`; `ubsan-validation.json` and `ubsan.py.txt` preserve commands, outputs and source hashes. Every recorded Windows benchmark run completed successfully and produced the same checksum across modes; missing or failed results are not represented as zero.

## Windows synthetic measurements

Windows 11, Intel64 Family 6 Model 183 Stepping 1, Clang 18.0.3 targeting x86_64-pc-windows-msvc, `-std=gnu++20 -O2 -pipe -Wall -Wextra -Wshadow -D_GLIBCXX_ASSERTIONS`, one pinned CPU (`0x1`). MSVC's standard library does not use `_GLIBCXX_ASSERTIONS`; `NDEBUG` was not set. Each process generates the same seed-20261118 points and queries before the timer. Construction and querying are timed separately; input generation, I/O and destruction are excluded. Query values below are milliseconds, median followed by `[minimum, maximum]`, from five samples after one warm-up with alternating implementation order. Each value is the **total** for the listed queries. `—` means the mode was not measured for that workload.

| Workload | N | Q | Old | Mode 4 median | Mode 5 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Small, broad random | 32 | 100000 | 63.5904 [61.8323, 73.9999] | 36.4108 | 38.3212 [37.5871, 38.8700] |
| Small, duplicate points | 32 | 100000 | 12.6378 [12.1021, 13.8201] | 10.0085 | 9.7356 [9.3141, 10.3881] |
| Broad random | 20000 | 30000 | 211.142 [191.792, 229.428] | 197.689 | 191.101 [185.560, 217.846] |
| Duplicate points | 20000 | 30000 | 4.0604 [3.9012, 4.5960] | 4.8867 | 4.7030 [4.5841, 5.8875] |
| Concentrated x, narrow | 20000 | 30000 | 104.351 [98.598, 112.078] | — | 82.677 [78.419, 93.942] |
| Full domain | 20000 | 30000 | 9.6930 [9.1507, 10.7634] | — | 10.4007 [8.9876, 12.1531] |
| Empty rectangles | 20000 | 30000 | 0.0575 [0.0546, 0.0632] | — | 0.0584 [0.0544, 0.1510] |
| Tiny rectangles | 20000 | 30000 | 96.158 [90.313, 122.539] | — | 71.207 [63.115, 79.530] |
| No points | 0 | 30000 | 2.7357 [2.3599, 2.8906] | — | 2.9940 [2.6821, 4.1182] |
| Updates only | 20000 | 0 | 0 [0, 0] | — | 0 [0, 0.0001] |
| Broad random, maximum N and Q | 200000 | 200000 | 5056.56 [5005.16, 5112.54] | — | 4828.91 [4811.68, 4863.35] |

For 200000 points, median construction was 1579.24 ms for the old code and 1620.14 ms for mode 5; construction itself is unchanged. Median process-wide peak working set was 256.6 MiB for each, including the generated workload and runtime. The duplicate-point query regression is 0.6426 ms per 30000 queries, or about 21 ns per query. The zero query timings on the updates-only workload are actual readings with zero queries, not missing values. Each result JSON contains every sample and warm-up, checksum, exit status, elapsed build/query time, process peak memory and timestamps.

The synthetic input generator and seed are preserved, but no independent input-file hash was recorded. These Windows measurements do not contain the official problem's input parsing, output formatting or checker; they cannot be directly compared with the WSL official-case timings above. In particular, the clear Windows synthetic improvement at maximum N and Q did not carry over to the separate direct official-case comparison.

## Reproduction

Use a checkout whose public `blueberry/data-structure/dynamic-fenwick-tree-2d.hpp` matches `baseline-original.hpp.txt`; the preparation script verifies its Git blob. Copy `prepare.py.txt`, `run.py.txt` and `study.cpp.txt` to `.build/dynamic-rectangle-sum-study/` without the `.txt` suffix. From the repository root:

```sh
python3 .build/dynamic-rectangle-sum-study/run.py --build --tag local
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-short --build-tag local --only small-random,small-duplicate,random-wide,duplicate
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-edge --build-tag local --modes 0,5 --only concentrated-narrow,full,empty-range,tiny-range,no-points,add-only
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-large --build-tag local --modes 0,5 --only official-size-random
```

The runner defaults to modes 0, 4 and 5; the commands specify modes 0 and 5 where only those were timed. It verifies source and executable hashes after building, pins child processes to one available CPU, saves full samples and failures under unique tags, and refuses to reuse an existing output tag. Adjust the compiler, SDK and ACL paths for another machine. The paths and hashes in `build-mergedwin.json` identify the original measurements; a different machine or compiler is a separate result.
