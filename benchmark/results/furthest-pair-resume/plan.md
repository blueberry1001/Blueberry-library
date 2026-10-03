# furthest_pair resume: comparison plan

Saved WIP checkpoint: `da912761b09e4c4cf6d67d2c154c03672ca8658b`.
The original `benchmark/furthest-pair.cpp`, `.py`, and `results/furthest-pair/`
are retained byte-for-byte. Their four binaries and 93-case brute gates were
preparation evidence only; no original timing/official success is inferred.

This resumed experiment compares independently implemented layouts under a
common immutable-input contract: signed 64-bit coordinates within
`[-(2^62-1), 2^62-1]`, N <= INT_MAX, signed 128-bit geometric arithmetic,
normalized distinct original indices for N>=2, and {-1,-1} for N<2.
Ties are compared by the squared-distance objective, not index identity.
The independent candidates validate even singleton inputs when assertions are
active. Public template support for smaller signed coordinate types is checked
by the separately owned library validation; timings here use `long long` only.

| Variant | Construction/layout | Calipers/recovery |
|---|---|---|
| blueberry | Exact snapshotted production function; copies points into coordinate hull | Wrapped support scan, final original-index search |
| indices | Allocate/sort original `int` indices, combined strict hull | Indirect coordinate loads; hull indices already identify originals |
| records | Allocate/sort 24-byte coordinate/index records, combined strict hull | Direct coordinate loads; original indices stored in records |
| fast-cases | Benchmark wrapper; validate and return directly for N<=2 or all-equal points | Otherwise calls exact production function; all-equal detection can scan N points |

No production edit is made by the benchmark owner. Initial adoption evidence is
separate from a later exact-final-header binding experiment. Wrapper timings do
not count as final production-code verification. Any adoption preserves type,
bounds, assertions, return values and worst-case O(N log N) time/O(N) space.

## Matrix and scopes

- GCC 14 and Clang 19, `-std=gnu++20 -O2 -Wall -Wextra`, release adds
  `-DNDEBUG`; no `-march=native`. Modes/compilers are never pooled.
- N=2,048 and 200,000: random square, duplicates (32x32), all-same,
  collinear, shuffled integer parabola (H=N), sorted random, reversed sorted
  random, full coordinate-bound random, and all-equal prefix with one different
  final point. The last distribution exposes a full failed all-equal scan.
- One tiny batch with size parameter200,000 (100,000 cases): empty, singleton,
  two points, square, equal triple, and collinear triple in rotation.
- Fixed input seed `0x4655525448455354`; independent small brute fuzz seed
  `0x46505f46555a5a31`. Input/objective digests and actual N/H statistics retained.
- Five samples plus one warmup for each of 4x4x19 kernel configurations:
  1,824 process profiles (304 warmups,1,520 measured). Variant order reverses
  each repetition, offset by compiler/mode and workload. Fresh process per
  sample. Inputs, answer storage and validation are outside kernel timers.
- Each full-call timer covers all calls in that workload, including candidate
  input copies, checks, construction, calipers, recovery and temporary cleanup.
  A separate staged call decomposes construction (validation/input layout,
  sort/unique/scan), calipers and coordinate recovery. It uses independent
  `int` calipers, not instrumentation inside the production `size_t` function.
  Therefore stages are explanatory proxies, not additive parts of full time.
  Fast-cases/tiny stage values are null; index/record recovery is null.
- Random and tiny input files: FastInput/FastOutput versus unsynchronized
  iostream, same complete cases and output pairs, 5 samples+warmup:384 profiles
  (64 warmups,320 measured). Parse/solve/format separately timed. External
  end-to-end includes small-shell/process startup, pipes and post-solve objective
  validation. All cases are held to separate phases; memory is not the streaming
  verifier's memory footprint. No I/O/generation is included in kernel timers.
- Separate GCC release/assert allocation diagnostics cover7 workloads x4variants
  =56 profiles. Timing fields from these instrumented binaries are not used.
  Requested calls/bytes cover full algorithm calls only, not input/answer storage,
  staged calls or metadata. Bytes are cumulative payload, not peak-live memory.
- Separate noninstrumented whole-process RSS:3workloads x4variants x4modes=48
  profiles, Linux getrusage KiB beneath a small shell. Runtime, inputs, allocator
  retention and all phases contribute; RSS is not an algorithm-only allocation
  capacity. One run/cell is diagnostic, not a precise memory difference claim.

## Gates, retention and decision

Compile sequentially and check brute-force objectives plus every variant's
invalid singleton assertion behavior. Preserve schema/identity gates across all
compiler/mode/diagnostic binaries. Every preparation snapshots the harness,
runner and exact three production headers; record compiler commands/versions,
source/header/binary/input hashes and current production header hash.

Measurement refuses changed snapshots/binaries/inputs/runner/live production
header, preserves raw warmups and measured rows, and writes subprocess failure,
timeout, malformed JSON/schema and identity rejection evidence before stopping.
Derived summaries retain median/min/max; no failed row is replaced by zero.
Do not overwrite existing experiment or measurement directories.

Timing only starts after parent authorizes a quiet window. Capture process names
(no command arguments), CPU/platform, cgroup quota and affinity. The host is
shared and unpinned; process snapshots do not establish absence of host noise.
Both gains and regressions/overlapping ranges are reported. A clear tiny or
all-equal win alone does not justify an unreported general/late-prefix regression.
Do not claim fastest among submitted solutions: Fastest access was blocked.

Reproduce initial preparation from the repository root:

```sh
source /tmp/blueberry-setup/env.sh
ulimit -c 0
python3 benchmark/furthest-pair-resume.py prepare --experiment initial
# Only in a coordinated quiet window:
python3 benchmark/furthest-pair-resume.py run --experiment initial
python3 benchmark/furthest-pair-resume.py diagnostics --experiment initial
```

Existing directories intentionally fail on reproduction; select a fresh
`--experiment` name. All compiler builds are sequential. Hashes in the original
prepared experiment remain the source of truth for its actual measured binaries.
