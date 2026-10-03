# Static convex hull: implementation and performance study

Measured on 2026-10-03 on branch `feat/static-convex-hull`, independently of the
unavailable dominator/general-matching checkpoint. This batch adds a strict
integer-coordinate hull, a geometry operation absent from ACL. The public API
returns coordinates counterclockwise from the lexicographic minimum, omits
collinear edge-interior points, removes duplicates, and accepts empty input.

## Source research and limitations

- [KACTL ConvexHull.h](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ConvexHull.h)
  explicitly declares the Unlicense. Its monotone-chain implementation sorts
  coordinates and writes into one preallocated hull buffer. This motivated
  testing initialized-buffer versus reserved-vector storage.
- [maspypy convex_hull.hpp](https://github.com/maspypy/library/blob/main/geo/convex_hull.hpp)
  sorts indices, builds chains, and offers additional modes. It was studied for
  layout and API ideas; its code was not copied. Its coordinate arithmetic and
  index-returning API do not by themselves supply this batch's contract.
- [Library Checker official solution at 1814c4e](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/static_convex_hull/sol/correct.cpp)
  also sorts indices and constructs chains. The benchmark reimplements that
  layout independently, converts the result back to coordinates, widens
  arithmetic, and uses the same input-copy convention as the public API.

The public Fastest query was attempted with the repository's
`scripts/fetch_lc_fastest.py`, requesting the three fastest AC C++ submissions.
The request failed with `Tunnel connection failed: 403 Forbidden`; the web
Fastest page was also inaccessible. No Fastest source or rank was obtained,
and no comparison against the current fastest submission is claimed. The
required Fastest-source research remains blocked by access. The failure log,
URLs, retrieval date, and primary-source SHA-256 hashes are preserved under
`benchmark/results/convex-hull/`; downloaded research source is only in ignored
`.build/convex-hull/research/`.

## Equal-contract candidates

All candidates use signed 64-bit point coordinates, exact signed `__int128`
orientation, identical strict-hull semantics, and O(N log N) time with O(N)
auxiliary memory. They accept a point vector by value and return coordinates.
Kernel calls copy an immutable input vector; I/O calls move parsed input into
the selected implementation. Assertions enforce the same coordinate range in
every candidate when enabled.

The supported bound is B = 2^62 - 1 and every coordinate is in [-B, B]. Each
coordinate difference is computed after conversion to `__int128`, so its
absolute value is at most 2B = 2^63 - 2. Each product is below 2^126 and the
absolute difference of the two products is at most
2(2^63 - 2)^2 = 2^127 - 2^66 + 8, below the signed 128-bit maximum. This
conservative independent-product bound suffices without relying on geometric
correlation between the terms.

| Candidate | Sort and output storage | Deliberate tradeoff |
| --- | --- | --- |
| `blueberry` | Sort coordinate vector; reserve one combined hull vector; push/pop without zero-initializing capacity | Current public header |
| `value-buffer` | Sort coordinate vector; initialize N+1 output slots; explicit used index | Removes vector size updates but writes the entire output allocation |
| `index-chains` | Sort `size_t` indices; build lower and upper index vectors; materialize coordinates | Smaller sortable records, extra indirection and chain/result storage |

The index candidate uses `size_t` to avoid imposing an `int` point-count limit.
It does not implement maspypy's optional inclusive/sorted/one-chain APIs. The
benchmark compares the common strict full-hull operation only.

## Measurement protocol

The reproducible harness is `benchmark/convex-hull.cpp` with runner
`benchmark/convex-hull.py`. The measured host was an x86-64 Intel Xeon Platinum
8573C, with GCC 14.2.0 and Clang 19.1.7. Both used `-std=gnu++20 -O2`; release
also used `-DNDEBUG`. No `-march=native` or special ISA was enabled. Compiler
commands, versions, binary/source/input hashes, CPU details, and raw samples
are recorded in the result files.

There are eight deterministic N=200,000 inputs: uniform random coordinates;
1,024 possible duplicate coordinates; identical points; collinear points;
shuffled parabola points with all N on the hull; sorted random points;
reversed sorted points; and random coordinates across the supported wide
range. The seed is `0x434f4e564558`. Each kernel configuration has one warmup
and five measured repetitions. Candidate order rotates across workloads.
Each I/O configuration also has one warmup and five repetitions, alternating
candidate order. There are 96 kernel configurations and 240 measured I/O
processes in total. All kernel input/hull digests and all I/O output hashes
agreed between candidates and compiler/assertion configurations.

Other agents paused compilation, tests, and Jekyll during timing. The process
snapshot records only PID, CPU percentage, and command name. Shared-host
activity outside this workspace remains possible. The benchmark does not pin
a CPU, and small differences should not be treated as universal advantages.

Kernel `full_ns` includes the input copy and implementation's internal
allocation/destruction, excluding destruction of the returned hull. Separate
staged calls report input copying, validation/index setup plus sorting and
deduplication, and scan/output construction. The public header is measured
intact: its combined post-copy work is recorded without inserting clocks
inside production code. Stage medians therefore need not sum to full-call
medians. I/O runs separately report parsing, moved-input hull construction,
formatting/flushing, and process end-to-end time; output is captured through a
pipe for exact comparison. This is a one-case harness rather than the
multi-case Library Checker driver.

## Release full-call medians

Times below are milliseconds for N=200,000. Every raw sample, median, and
minimum is in `kernel.json`; assertion-enabled measurements are separate rows
there and are not mixed into this table.

| Input | GCC public | GCC buffer | GCC indices | Clang public | Clang buffer | Clang indices |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Random | 23.87 | 27.30 | 37.41 | 23.40 | 25.67 | 34.04 |
| Duplicates | 15.82 | 16.09 | 23.73 | 13.82 | 13.69 | 19.26 |
| All same | 3.52 | 2.91 | 4.41 | 3.97 | 3.33 | 4.28 |
| Collinear | 20.65 | 21.67 | 33.13 | 19.65 | 25.54 | 27.53 |
| Full hull/parabola | 20.58 | 21.00 | 35.16 | 20.11 | 20.34 | 30.98 |
| Sorted | 11.34 | 12.54 | 16.09 | 9.75 | 11.22 | 11.75 |
| Reversed | 10.09 | 12.40 | 15.38 | 9.18 | 10.33 | 10.76 |
| Wide coordinates | 25.10 | 25.65 | 38.05 | 23.98 | 24.68 | 34.00 |

The public implementation beat the index-chain candidate on every release
input/compiler pair. For random input, GCC's index variant spent 24.87 ms
preprocessing/sorting and 12.69 ms scanning, versus the coordinate-buffer
variant's 18.02 ms and 7.84 ms. Clang showed the same direction: 23.51/10.69 ms
for indices and 16.37/7.27 ms for the coordinate buffer. Both index sorting and
scanning suffer from indirect access; smaller sortable records did not offset
that cost on this workload.

Retain the reserved-vector layout. The initialized buffer gave no broad
advantage and incurred extra writes when the hull was small. The all-same
exception exposes a separate degenerate-result capacity issue: returning the
deduplicated input retains its original N-element capacity, whereas the
independent buffer candidate returns a compact copy for one/two unique
points. This does not establish that compacting is beneficial for tiny
inputs, where an additional allocation can dominate the whole operation.
The public header is unchanged pending the targeted evaluation below.

### Pending degenerate-result experiment

`benchmark/results/convex-hull/compact-candidate.cpp` and
`compact-prepared.json` preserve a prepared follow-up without changing the
initial benchmark source or measured public header. Its three candidates are
the current header, a wrapper that copies a result of size at most two into
compact storage, and a wrapper that only does so when capacity exceeds size.
The latter avoids a second allocation for ordinary exact-capacity tiny
inputs. All wrappers preserve the coordinate-returning API.

The follow-up uses empty/singleton/two-point inputs with 500,000 calls per
sample, plus 200,000-point all-same, two-unique, random, and collinear cases.
Five samples after a warmup rotate candidate order. The timed loops include
input copying and result destruction, so their values must be compared only
within this follow-up, not directly against the earlier table. Correctness
comparison against the unchanged header is built into the executable.

Four GCC/Clang release/assertion binaries compiled successfully. **No
follow-up timings or runtime checks have run at this checkpoint.** Integration
with the now-available graph checkpoint takes priority; the compact-result
decision remains pending a later coordinated quiet window. Initial timing
and memory results above remain the evidence for the unchanged public header.

## I/O and memory

GCC release medians, in milliseconds:

| Input / implementation / I/O | Parse | Hull | Format/flush | End-to-end |
| --- | ---: | ---: | ---: | ---: |
| Random / public / FastIO | 9.44 | 23.89 | 0.03 | 35.89 |
| Random / public / iostream | 23.69 | 24.94 | 0.04 | 51.02 |
| Random / indices / FastIO | 9.51 | 37.82 | 0.03 | 49.85 |
| Full hull / public / FastIO | 8.02 | 20.49 | 6.65 | 38.56 |
| Full hull / public / iostream | 20.14 | 22.98 | 16.97 | 63.31 |
| Full hull / indices / FastIO | 7.81 | 35.18 | 6.29 | 52.50 |

FastInput/FastOutput is justified for the verify driver. The I/O comparison
uses the same hull implementation and input for each row; it does not credit
parsing improvements to the hull algorithm. Both compilers and assertion
configurations remain individually available in `io.json` and `results.json`.

The original Python-launched process `ru_maxrss` figures were floored by the
launcher's resident size and cannot distinguish these small allocations.
They are retained in the raw logs but are not used for memory conclusions.
Separate memory-only runs launch the benchmark as a child of a small shell,
use FastIO with output sent to `/dev/null`, and record Linux
`getrusage(RUSAGE_SELF).ru_maxrss`. These runs occurred after the quiet timing
window; their timing fields are deliberately discarded. Three repetitions
for each compiler/mode/input/candidate gave these release peaks on both
compilers:

| Input | Public | Initialized buffer | Index chains |
| --- | ---: | ---: | ---: |
| Random, hull size 31 | 4,736 KiB | 7,936 KiB | 6,400 KiB |
| Full hull, size 200,000 | 7,936 KiB | 7,936 KiB | 11,008 KiB |

RSS measures touched physical pages and process overhead, not vector capacity
or exact live allocation bytes. Reserving an output vector need not touch
every reserved page. The results support avoiding the buffer initialization
and multiple index chains; all implementations still have O(N) space bounds.

## Reproduction and remaining work

From a compiler-equipped repository environment:

```console
python3 benchmark/convex-hull.py prepare
# Pause concurrent compilation/tests before the next command.
python3 benchmark/convex-hull.py run
python3 benchmark/convex-hull.py memory --repeats 3
```

`prepare` records hashes and compiles before timing. `run` refuses changed
sources/binaries/inputs or visible active compilers. `memory` performs no
speed comparison. Reruns overwrite the current result files, so preserve
earlier measurements before evaluating a changed header.

Official datasets and independent randomized/orientation-oracle checks are
handled by the integration task; benchmark agreement supplements those
checks rather than replacing them. Current Fastest-source access is the
remaining research limitation, and this report makes no fastest-submission
claim.
