# Weighted Wavelet Matrix performance investigation

The weighted variant supports fixed values, online point-add weights, and
index/value-range sums, with arbitrary totally ordered `Value` and additive
group `Weight`. The regular Wavelet Matrix and offline point-registration
adapters have related uses but different contracts. This investigation starts
with an existing, previously unmeasured opportunity: construct sorted ranks by
sorting positions and writing ranks to their original positions, instead of
sorting values and then doing N independent `lower_bound` calls. No frequency
ranking from user activity is available; wavelet range queries and dynamic
rectangle sums are common competitive-programming use cases, not a measured
frequency claim.

## Public source research

The Library Checker v3 API returned [Point Add Rectangle Sum](point-fastest.md)
and [Range Kth Smallest](kth-fastest.md) snapshots on 2026-11-15. The API
language filter `C++23` returned no rows; that empty result is not a timing.
The saved snapshots omit that filter and deduplicate submitters. Leaderboards
move and judge times cannot be divided by local measurements to obtain a speedup.

The first C++ point-add-rectangle-sum submission in the snapshot,
[206958](https://judge.yosupo.jp/submission/206958), is by oldyan; its source
identifies [CP-template](https://github.com/old-yan/CP-template). It sorts and
registers future points before answering, uses bit-rank plus Fenwick trees, and
reads via mmap when given a regular file. [404520](https://judge.yosupo.jp/submission/404520)
has a similar OY wavelet layout. Registration of future points and unsigned
problem-specific types are additional conditions relative to the standalone
WeightedWaveletMatrix API. The submission and repository root did not expose
an explicit license in the reviewed materials; submissions are fetched into
ignored local storage for research and are not redistributed in this repository.
Fetched source SHA-256: 206958 `e8c7eb1bff6cb726990f92fa4477c93f4ff90c235ac55752abb28e7bda0090d4`,
404520 `45d171bf57b259f27b04f7ad91ab7c344bf1f35f823e652c3af94fdbb636e514`.

[Nyaan's Fenwick-on-Wavelet Matrix](https://nyaannyaan.github.io/library/data-structure-2d/fenwick-tree-on-wavelet-matrix.hpp.html)
registers points, merges duplicates, and uses rank arrays plus Fenwick trees.
It explicitly targets POPCNT/BMI/BMI2 in several functions, so ISA and point
registration are part of its comparison conditions. The repository is
[CC0-1.0](https://github.com/NyaanNyaan/library/blob/master/LICENSE). The
rank-kth leader [403008](https://judge.yosupo.jp/submission/403008) batches
queries by level and includes SIMD; its online semantics differ from this API.
Its fetched source SHA-256 is
`7cc67d7309d6df040e48068974c2c5c46d5f89e55f10bce2a0586480c21bd7db`.
These sources were read as algorithm references; the experiment does not copy
their implementation into the library.

## Rank writeback: initial samples

Baseline revision `efa8b4e97ce68395657c643fd6c04f2fba56b826`; the exact
baseline, Fenwick dependency, candidate, driver and runner are saved under
`benchmark/weighted-wavelet-rank-*.hpp.txt`,
`benchmark/weighted-wavelet-rank-driver-v1.cpp` and
`benchmark/run_weighted_wavelet_rank_v1.py`. Three measured runs follow one
warmup in alternating order, on WSL2 / Intel i7-14650HX. Input generation is
outside the timer. Query results are compared across all variants. Construction
and interleaved updates/queries are timed separately; process RSS includes
input and executable. Inputs use seed 1729, signed weights, 25% updates, and
random index/value intervals. The first version's random integer values lie in
`[0, LLONG_MAX]`; the subsequent expanded benchmark includes signed extrema.

GCC 13.3, `-std=gnu++20 -O2 -DNDEBUG`: median construction in ms, baseline →
the public candidate, was 67.75 → 52.42 for 200,000 broad random values,
20.40 → 18.14 for 200,000 values drawn from 32 choices, 28.02 → 20.32 for
200,000 ascending values, and 22.43 → 16.28 for 40,000 random strings. Clang
18 release gave 42.49 → 39.11, 13.64 → 14.11, 22.33 → 19.67, and 19.57 →
14.24 ms respectively. The small duplicate-case difference under Clang needs
additional sampling before being treated as a meaningful regression. The
20,000 interleaved operations and peak RSS are recorded separately in JSON;
they should not be inferred from construction alone.

Individual samples, flags, compiler version, hashes and checksums are in
`benchmark/results/weighted-wavelet-rank-gcc-release.json`,
`weighted-wavelet-rank-clang-release.json` and
`weighted-wavelet-rank-gcc-assert.json`, with adjacent logs. These first runs
compare the actual candidate to the frozen baseline and a separate rank
experiment. They do not measure final proposed radix, dense-rank or shared
query paths, AtCoder verdicts, nor isolated construction subphases.

To reproduce an earlier runner, work in a disposable checkout and copy the
saved `benchmark/run_weighted_wavelet_rank_v1.py` over the current runner and
`benchmark/weighted-wavelet-rank-driver-v1.cpp` over the current driver before
invoking it with `--candidate benchmark/weighted-wavelet-rank-candidate-v1.hpp.txt`.
The v2 and v3 runners use the saved v2 driver. This avoids overwriting the live
worktree. Git attributes preserve benchmark sources, snapshots and logs
byte-for-byte. The original candidate snapshot contains CRLF; both its raw
hash and the LF-normalized hash used by the range experiment are recorded.

## Expanded compression experiments

The expanded driver uses signed full-width 64-bit random values and the same
signed weights and update/query mix. The candidate is the actual working-tree
header at measurement time. Experimental variants sort positions (`rank`),
use byte-wise radix sorting for integers (`radix`), or use a bounded dense
rank array when the integer span is at most both 2N and 2^20 (`dense`). The
hybrid first checks for an already sorted input, then tries dense compression;
otherwise it uses radix only up to six passes and falls back to comparison
sort. `hybrid-sample` also allows up to eight passes if fewer than half of the
first 256 values are the global minimum or maximum. Non-integers retain the
comparison-sort path. The density cap, six-pass rule, and sample are trial
heuristics, not proven universal thresholds. Dense compression can allocate up
to 1,048,577 extra integers; benchmark RSS includes temporary peak allocation.

The simple radix variant had regressions: on GCC, 200,000 inputs mixing extrema
and broad random values took 37.41 ms versus 33.58 ms for the public candidate,
and ascending data took 23.31 versus 21.46 ms. The following table gives median
construction milliseconds for the later hybrid comparison, with the same input,
same compiler/flags and three measured runs. Candidate / hybrid-sample:

| Input | GCC 13.3 | Clang 18.1 |
| --- | ---: | ---: |
| 128 signed full-width random | 0.015 / 0.015 | 0.014 / 0.015 |
| 200,000 signed full-width random | 49.33 / 44.01 | 41.00 / 32.96 |
| 200,000 values from 32 choices | 18.15 / 12.64 | 14.76 / 9.51 |
| 200,000 signed values from [-100,100] | 24.28 / 16.72 | 20.51 / 13.74 |
| 200,000 extrema mixed with random values | 27.03 / 26.18 | 25.75 / 24.59 |
| 200,000 ascending | 20.74 / 18.46 | 19.73 / 18.71 |
| 40,000 random strings | 13.86 / 14.59 | 13.09 / 12.52 |

Every variant produced the same checksum. These results include only three
samples per case and one string distribution, so they do not establish that
the heuristic is universally faster. A distribution whose first 256 values are
unrepresentative can choose the slower sorting path. Hashes, operation times,
peak RSS, seeds and all samples are in
`benchmark/results/weighted-wavelet-expanded-{gcc,clang}.json` and
`benchmark/results/weighted-wavelet-hybrid-{gcc,clang}.json`. The former used
`benchmark/run_weighted_wavelet_rank_v2.py`; the latter used
`benchmark/run_weighted_wavelet_rank_v3.py`. The matching earlier driver is
`benchmark/weighted-wavelet-rank-driver-v2.cpp`.

## Shared-prefix range sums and overflow

The unmodified range sum evaluates two prefix-like traversals. A prototype
shares their equal upper bit path and computes only the suffixes after the
paths diverge. Simply subtracting the shared signed prefix changes intermediate
values and is **not safe**: with values 0..15, weights `w[0]=-1`, `w[8]=1`,
`w[12]=INT_MAX`, others zero, `sum(0,16,10,14)` is `INT_MAX`. The original
intermediate sums and Fenwick cells fit, but the direct suffix prototype adds
`1 + INT_MAX`. UBSan confirms the original succeeds while this prototype
fails; see `benchmark/results/weighted-wavelet-overflow-{baseline,unsafe}.log`.

The signed-integral prototype instead obtains each Fenwick interval exactly as
before and accumulates those results in the corresponding unsigned type. It
subtracts in that same modular ring and converts back at the end. The original
intermediate-value preconditions ensure the retained individual Fenwick calls
are valid. The shared terms cancel modulo 2^N, and a representable original
result has the same modulo-2^N representative. C++ integral conversion gives
the destination value congruent modulo its width; unsigned arithmetic is also
defined modulo its width ([integral conversions](https://eel.is/c++draft/conv.integral),
[fundamental types](https://eel.is/c++draft/basic.fundamental)). Integer
promotions for narrow types require conversion back to the corresponding
unsigned type before the final signed conversion. The prototype uses the old
path for floating point, user-defined types and upper-rank sentinel cases. This
avoids changing floating-point addition order and generic operator demands.
UBSan passes the counterexample for `int` and `long long`; one float case gives
the same hexadecimal result as the original. Full randomized type and boundary
testing is still required before treating the prototype as production-ready.

With N=200,000, Q=100,000, long long weights, 25% updates and negative
weights, GCC baseline / signed-safe median operation times in ms were equal
endpoints 103.58 / 45.07, narrow value ranges 110.58 / 68.40, almost-full
ranges 61.47 / 60.37, and random ranges 142.09 / 122.54. Clang gave 149.04 /
56.90, 162.02 / 91.98, 81.09 / 80.43, and 188.80 / 178.11 ms respectively.
All compared checksums matched. Each JSON records all three samples,
construction, RSS and the candidate hashes:
`benchmark/results/weighted-wavelet-ranges-safe-{gcc,clang}.json`. The older
unsafe experiment is retained separately in `weighted-wavelet-ranges-gcc.json`;
its exact generator is `benchmark/run_weighted_wavelet_ranges_v1.py`. Both
experiments used the LF-normalized header snapshot
`benchmark/weighted-wavelet-range-baseline.hpp.txt`; its SHA-256 differs from
the byte-for-byte CRLF snapshot of the same candidate used in earlier runs.
The safe experiment used `benchmark/run_weighted_wavelet_ranges_v2.py`.

## Official-input end-to-end comparison

Four cached official Point Add Rectangle Sum cases were executed as separate
processes under GCC 13.3 with `-std=gnu++20 -O2 -DNDEBUG`, warmup plus three
samples, rotated order. The two Blueberry executables use the same independent
adapter and `FastInput`/`FastOutput`; 206958 retains its own mmap-based I/O.
Both adapters pre-register future coordinates, and every run's complete output
matched the official expected output. These are same-input, same-compiler local
end-to-end results; different I/O, memory layout, supported types, and
registration contracts mean they are not a data-structure-only comparison.

| Official case | Original ms / KiB | Candidate ms / KiB | 206958 ms / KiB |
| --- | ---: | ---: | ---: |
| max_random_00 | 303.66 / 43,588 | 307.66 / 44,064 | 250.73 / 38,924 |
| xy_concentrate_00 | 175.88 / 34,636 | 171.37 / 34,136 | 255.29 / 38,480 |
| many_points_00 | 263.00 / 51,404 | 255.49 / 51,272 | 227.89 / 43,516 |
| many_queries_00 | 243.45 / 34,224 | 237.61 / 35,476 | 232.60 / 33,180 |

Individual runs, case hashes, source hashes, and process RSS are in
`benchmark/results/weighted-wavelet-official-gcc.json`, with the adjacent log.
The comparison does not isolate parsing, formatting, memory allocation, or
destruction, and does not claim an official judge result. The exact external
source is intentionally not committed; `benchmark/run_weighted_wavelet_official.py`
accepts the downloaded submission source as `--external`.

## Generated assembly

`benchmark/inspect_weighted_wavelet_assembly.py` compiled wrappers for `add`
and range `sum`, using the safe prototype header and frozen Fenwick dependency.
[Assembly and metadata](assembly/metadata.json) retain both GCC 13.3 and
Clang 18.1 with portable `-O2` and a separate `-O2 -march=native` build.
GCC portable emitted nine `__popcountdi2` call sites in the whole translation
unit; the native build emitted nine `popcnt` instructions instead. Clang
portable expanded popcount using shifts and masks; native used `popcnt` for
the update wrapper and vector gather/nibble-lookup operations in the range
path. None of these four translation units contained an integer divide
instruction. These are static instruction observations, not dynamic counts or
speed measurements. `-march=native` is hardware-specific and is not a new
requirement for the public library.

## Final candidate and evenly spaced sampling

The integrated candidate initially sampled the first 256 values when deciding
whether a full-width radix pass should be used. On the deliberately biased
input whose first 256 values were random and whose remainder alternated signed
extrema, this chose radix and regressed construction relative to the original.
A variant instead takes evenly spaced samples at `size_t(i) * N / sampled`.
It is still a heuristic: an input deliberately aligned to those positions can
misrepresent the rest of the sequence. The measured header uses the evenly
spaced variant, which remains in the live header.

On 200,000 values, median construction ms for first-256 / evenly-spaced were:
GCC random 40.94 / 40.67, extreme-biased head 43.41 / 37.44, extreme-biased
tail 15.92 / 11.00; Clang random 32.71 / 31.77, head 37.01 / 30.94, tail
14.40 / 10.91. Each case uses three measured runs and one warmup; all
checksums agreed. Clang ascending was 17.54 / 18.64 ms and descending was
18.39 / 19.59 ms, despite those branches not consulting the sample; the
variation should not be interpreted as an isolated sampling cost. All input
shapes, compiler flags, hashes and samples are retained in
`benchmark/results/weighted-wavelet-strided-{gcc,clang}.json` with adjacent
logs. GCC/Clang also ran the earlier unmodified final-candidate variant on
signed min/max-adjacent and biased distributions in
`weighted-wavelet-final-{gcc,clang}.json`.

The evenly spaced source snapshot is
`benchmark/weighted-wavelet-strided-candidate.hpp.txt`, raw SHA-256
`92cb649d2475f25ada3f07499f81173e6613db3201c5b2e264e451b7dd631f9f`;
the live header matched those bytes at measurement time. Both normalize to
`88e55a4b2400aa82762916cda6d9fb97b6128168c92e7d43a5d9b750dcd26ae8`
with LF line endings. `benchmark/run_weighted_wavelet_rank_v5.py` and the
current driver generated the strided comparison; the prior first-256 source
is preserved as `benchmark/weighted-wavelet-final-candidate.hpp.txt`.

After these measurements, the live header's two byte-counting shifts were
changed to widen their operands before shifting, avoiding MSVC warnings for
8-bit value types. The resulting file SHA-256 is
`285628f787b84a65d4de8092c63e982b56ead6943f8e9534e2aec2be5328fc26`,
while the measured snapshot's hash remains the raw `92cb649d2475f25ada3f07499f81173e6613db3201c5b2e264e451b7dd631f9f` above.
GCC and Clang performance was not remeasured after this change. MSVC 14.43 with C++20,
`/O2 /W4 /WX`, compiled the random test including signed 8- and 16-bit
weight overflow cases and signed 8-bit value compression; seeds 1, 2, 17,
1729 and 8675309 passed. GCC, Clang and official verification were not run
for this post-measurement revision.

## Windows/MSVC follow-up

[An exploratory comparison](../../../../benchmark/results/weighted-wavelet-msvc-handoff.json)
uses MSVC 19.43.34809 x64 on Windows 11, `/std:c++20 /EHsc /O2 /DNDEBUG`, with
the [runner snapshot](../../../../benchmark/run_weighted_wavelet_msvc_contended.py.txt),
current Fenwick header, and the same [driver](../../../../benchmark/weighted-wavelet-rank.cpp).
It compares the old header, the pre-shift-fix measured snapshot and the current
header in rotated order. Each case has one warmup and five timed runs; input
generation is outside the construction and operation timers. RSS was not
measured. The CPU was not pinned, and a Dijkstra benchmark ran concurrently;
this run is exploratory only and must not be used for adoption or regression
decisions. It is distinct from the WSL/GCC and Clang experiments above.
All variants produced the same checksum for each case.

For 200,000 signed 64-bit values and 20,000 interleaved queries/updates,
median construction / operation milliseconds for old, pre-fix, current were:
random 51.13 / 27.12, 29.53 / 26.09, 30.68 / 30.31; 32-value duplicate
15.65 / 5.30, 9.79 / 5.33, 9.39 / 4.98; ascending 22.31 / 22.12,
14.84 / 21.08, 14.31 / 21.43. Random-operation samples overlapped across
variants, and the competing workload prevents drawing a conclusion about the shift fix.
All samples, the input shapes and source hashes are in the JSON; the result
does not measure an official submission or isolate parsing and memory.

The comparison was then [repeated without the competing benchmark](../../../../benchmark/results/weighted-wavelet-msvc-isolated.json),
using the same source headers, driver, flags and eight shapes with the updated
[runner](../../../../benchmark/run_weighted_wavelet_msvc.py). It was not pinned
to a CPU and does not measure RSS. One warmup and five measured runs were
rotated across variants; all checksums matched. The table gives median
construction / operation milliseconds for the old, pre-shift-fix and current
headers. These are Windows/MSVC measurements and are not combined with the
WSL/GCC and Clang results.

| Values | Old | Pre-fix | Current |
| --- | ---: | ---: | ---: |
| Random | 51.17 / 25.58 | 26.79 / 23.60 | 26.70 / 24.33 |
| 32 choices | 14.68 / 5.31 | 7.88 / 4.86 | 7.88 / 4.76 |
| Signed small | 22.26 / 8.39 | 11.07 / 8.43 | 11.21 / 8.58 |
| Extrema | 25.66 / 15.85 | 20.25 / 15.01 | 20.65 / 14.31 |
| Ascending | 21.83 / 19.90 | 15.39 / 19.17 | 14.59 / 18.42 |
| Random strings | 18.08 / 18.15 | 12.88 / 17.04 | 12.58 / 17.05 |

The test also records two deliberately biased distributions. The current and
pre-fix samples are close, with no consistent effect established for the shift
fix. No additional implementation change is adopted from this measurement.

The current header's random and boundary test also compiled with MSVC 19.43,
`/std:c++latest /EHsc /O2 /W4 /WX`, and passed seeds 1 through 20. This does
not replace the required GCC/Clang builds or official verifier runs.

The measured source passed the signed-overflow example under both GCC and Clang
UBSan; the `double` fallback produced the same hexadecimal result as the old
code. Logs are `benchmark/results/weighted-wavelet-final-overflow-{gcc,clang}.log`.
The preceding final-range GCC samples in
`weighted-wavelet-final-ranges-gcc.json` used the first-256 snapshot: their
range operation code is unchanged, but their source hash is different and
they are not presented as measurements of the final byte sequence.

Final evenly spaced header versus original versus submission 206958 on the
same four official input files, GCC `-O2 -DNDEBUG`, median process times in
milliseconds, respectively: max_random 217.14 / 236.80 / 193.29;
xy_concentrate 149.13 / 154.73 / 194.57; many_points 195.51 / 220.38 /
216.11; many_queries 248.65 / 226.65 / 191.20. In the last case, candidate
samples were 225.83–255.68 ms and the submission's were 181.22–288.43 ms;
three samples do not settle that difference. Both adapters use registered
future coordinates, but the I/O and supported types differ. All runs matched
the official expected outputs. Complete data are in
`benchmark/results/weighted-wavelet-official-final-gcc.json`; this is a local
comparison, not a Library Checker or AtCoder submission result.
