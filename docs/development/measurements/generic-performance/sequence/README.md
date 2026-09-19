# FastSet / WaveletMatrix / StaticRangeLIS performance comparison

The same benchmark source is compiled against the three headers at `e0d8e71`
and the working tree. Baseline headers are retrieved with `git show` into a
temporary directory; neither the checkout nor previous evidence is overwritten.
The LIS comparison includes the WaveletMatrix change it uses internally.

Run from the repository on Linux/WSL with GCC and Clang installed:

```sh
python3 docs/development/measurements/generic-performance/sequence/run.py --output /tmp/sequence-gcc
python3 docs/development/measurements/generic-performance/sequence/run.py --compiler clang++ --output /tmp/sequence-clang
```

`--assertions` enables assertions; `--kind set|wavelet|lis` selects a subset.
`--cpu` selects the same CPU for both candidates. Each case has one warmup and
five measured runs by default. All invocations are sequential and candidate order
alternates. Do not run unrelated builds or measurements concurrently.

FastSet uses universe 2^20, one million operations and empty, half-full, full,
duplicate-insertion and sequential-fill workloads. The initial densities change
as operations execute. The mixed workloads use equal probabilities of insert,
erase, next, prev and contains; insert-only workloads are identified separately.
WaveletMatrix uses 500,000 signed 64-bit values and 200,000 online API calls mixing
kth-smallest, rank, count and kth-largest. StaticRangeLIS uses 65,536 signed 64-bit
values and 100,000 arbitrary-order queries. Both array structures cover random,
17 distinct values, all-equal, sorted and reverse-sorted data. The deterministic
seed is 20260920. Input generation is outside the measured regions; construction
and operations (including checksum accumulation) are timed separately.

`environment.json` records compiler/flags, CPU/OS, baseline revision and source
hashes; `compile.log` preserves commands and diagnostics. `samples.jsonl` retains
every warmup and measurement, input hashes, checksums and peak process RSS.
RSS includes inputs and runtime allocations, so it is not the structure's exact
storage. `summary.json` contains medians/minima/maxima of measured runs only.
Paired input hashes and result checksums must match; working sources must remain
unchanged during the run. Failures retain their logs and never become zero-time
samples. Old output directories are rejected rather than overwritten.

This experiment isolates generic data-structure changes. It does not measure I/O
and does not claim direct equivalence with Library Checker Fastest submissions.

## Adopted implementation and final results

The adopted changes preserve the public API, input types, query order, boundary
behavior and asymptotic guarantees:

- FastSet propagates insertion only when a word changes from empty to nonempty.
- WaveletMatrix assigns ranks from sorted positions instead of performing N
  binary searches. Arrays with N < 64 or N < 128 times the required 8-bit digit
  count use comparison sort. Other arrays use stable
  8-bit radix passes over the unsigned difference from the minimum, stopping at
  the actual required width. The unsigned conversion happens before subtraction;
  signed extremes, duplicates and GNU 128-bit integers remain supported. The two
  position buffers are reused for level construction. Dictionary copying is kept
  to avoid adding another allocation/layout change to this candidate.
- StaticRangeLIS shares a constructor-local scratch buffer across its recursive
  Monge multiplications. Each simultaneous recursion frame uses
  `n + 5 * ceil(n/2)` integers; siblings reuse the same region. The buffer is freed
  before the WaveletMatrix is built. The LIS numbers include its internal
  WaveletMatrix change, so they do not isolate the two contributions.

Final large-input evidence is in `gcc-release-selected`, `clang-release-selected` and
`gcc-assert-selected`: **450 measured samples and 90 warmups**, each with matching
paired input hashes and checksums. GCC 13.3 / Clang 18, GNU C++20, O2, CPU 0;
release defines NDEBUG and the assertion run does not. Representative medians
in milliseconds are below; these are local in-memory measurements, not submitted
Library Checker times.

| Operation / workload | GCC before → after | Clang before → after |
| --- | ---: | ---: |
| FastSet operations, sequential fill | 3.682 → 2.264 | 4.407 → 2.162 |
| FastSet operations, repeated random insert | 6.791 → 5.766 | 6.611 → 5.024 |
| WaveletMatrix build, random signed 64-bit | 126.604 → 71.658 | 77.101 → 47.881 |
| WaveletMatrix build, 17 distinct values | 29.499 → 18.180 | 20.262 → 12.350 |
| StaticRangeLIS build, random | 408.583 → 155.355 | 391.216 → 154.474 |
| StaticRangeLIS build, sorted | 286.043 → 71.280 | 290.356 → 72.770 |

FastSet mixed-query workloads improve much less: dense-set operations are
7.526 → 7.784 ms under GCC and 7.494 → 7.661 ms under Clang; GCC with assertions
has a total-time median of 8.204 → 8.288 ms. Earlier rounds also show small dense
regressions. This is not a claim that every workload gets faster.
The WaveletMatrix query code is unchanged, and individual query timings fluctuate
or regress (GCC 17-value queries: 13.436 → 13.930 ms; Clang sorted: 65.191 → 70.194 ms).
Final total-time medians improve in all five WaveletMatrix distributions for the
measured N/Q balance, but that does not establish a query-only speedup for all
query-heavy workloads. See every sample and its range rather than extrapolating
these small differences.

StaticRangeLIS trades a modest peak allocation increase for fewer allocations:
for the random input GCC peak process RSS is 9,580 → 10,224 KiB and Clang is
9,548 → 10,312 KiB. This includes input/query buffers; asymptotic memory is unchanged.
The query phase stays approximately the same and is not the claimed improvement.

## Rejected candidates and preserved evidence

The first implementation used index comparison sort without radix and allocated
the dictionary after the position buffers. `gcc-release`, `clang-release` and
`gcc-assert` retain that experiment (450 measurements, 90 warmups).
`wavelet-index-first.hpp.txt` exactly matches its archived header hash. It improved
many builds, but Clang's 17-value query phase regressed 12.432 → 14.421 ms and total
time increased. That candidate was not adopted.

`clang-wavelet-copy` restores the baseline dictionary copy/allocation order while
retaining index comparison sort (50 measurements, 10 warmups). Its source is
`wavelet-copy-first.hpp.txt`. The 17-value query median became 11.857 → 11.965 ms,
but construction still regressed 20.721 → 22.814 ms. This source was not adopted
either. These experiments suggest that layout/code generation affects the query
measurements; they do not isolate a single proven cause.

`clang-wavelet-radix` is the initial radix experiment (50 measurements, 10 warmups);
`wavelet-radix-preview.hpp.txt` matches its source hash. Its 64-bit benchmark passed,
but the expanded type matrix exposed Clang's `-Wshift-count-overflow` warning for
`unsigned char >>= 8` in the shrinking-width loop. The final code spells the
promoted shift and conversion explicitly. The original failure is retained in
`rejected-narrow-shift-warning.log`; it is not reported as a successful check.
Performance evidence was remeasured after that fix in the `*-final` directories.
Despite their earlier names, these are the superseded N=64-threshold candidate;
`wavelet-radix-threshold64.hpp.txt` matches their header hash. The selected source
and final evidence are described next.

## Small-array threshold: a rejected 3× regression

`small.cpp` and `small-run.py` compare baseline, the copied-dictionary comparison
sort, and the working candidate. Sizes are 32, 63, 64, 65, 128, 256, 512, 1024,
2048 and 4096; both signed 64/128-bit types and full-width/17-value distributions
are covered. Each process repeats construction of the same input
`max(64, 262144/N)` times, accumulating only constructor time. Input generation,
get/checksum, and destruction are excluded. This is a warm repeated-construction
microbenchmark, not a cold end-to-end measurement. Both halves of 128-bit values
are included in checksums. There are five measured processes and one warmup per
candidate/case, with candidate order rotating.

`small-gcc` and `small-clang` exposed an important regression in the fixed N=64
switch: at N=64 and full 128-bit values, GCC baseline/index-sort/radix took
1.240/1.032/3.753 µs per build, while Clang took 1.171/1.122/3.477 µs.
The 16 histogram passes dominated such small inputs. For full 64-bit values,
radix became competitive near N=1024; for full 128-bit values near N=2048;
17-value arrays benefited from radix near N=128. This motivated selecting by
the **actual digit count**, without restricting the input type or value range.

The adopted condition uses comparison sort when N < 64 or N < 128 × required
8-bit digits, and radix otherwise. Equal values require zero passes. The final
small evidence is `small-gcc-selected` and `small-clang-selected`:

| Build workload | GCC baseline → selected (µs) | Clang baseline → selected (µs) |
| --- | ---: | ---: |
| N=64, full 128-bit | 1.217 → 0.937 | 1.187 → 1.192 |
| N=256, full 128-bit | 6.016 → 4.352 | 5.384 → 4.874 |
| N=512, full 128-bit | 27.670 → 17.504 | 12.380 → 10.536 |
| N=128, 17 values, 64-bit | 1.686 → 1.202 | 1.486 → 1.045 |

The selected code is not fastest at every crossover under every compiler. For
example, GCC's N=2048/full-128-bit selected median is 179.953 µs versus 174.972 µs
for comparison sort, whereas Clang favors radix there. The condition avoids the
large small-array regression without introducing CPU-specific dispatch or an
input restriction. Further crossover tuning would require additional evidence.

Across all large and small adopted/rejected experiments, **3,850 measured samples
and 770 warmups** are preserved. The five `*-selected` directories describe the
adopted source (1,650 measurements, 330 warmups); earlier directories are distinct
experiments, never pooled into the selected medians.

## Correctness coverage

`validation.sh` reproduces GCC/Clang × GNU C++20/23, three fixed seeds each for
FastSet (in specialized structures), WaveletMatrix and StaticRangeLIS, followed
by Clang AddressSanitizer/UndefinedBehaviorSanitizer with seed 17. The successful
final output is `validation.log`; exact source hashes are in `validation.json`.
Coverage includes empty ranges, copy/move behavior where supported, 64-ary word
boundaries through four levels, every existing WaveletMatrix query, signed/unsigned
8/16/32/64-bit values, char/wchar_t/char8_t/char16_t/char32_t, GNU signed/unsigned
128-bit extremes, and strict LIS with duplicates and a non-default-constructible
input type supporting only ordering. Official verification remains the repository
integration workflow's responsibility.
