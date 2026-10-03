# Final benchmark decisions

Adopt the two StaticTopTree singleton construction shortcuts. Keep
SuffixAutomaton unchanged: neither clone movement nor `try_emplace` showed
a consistent improvement across compilers and inputs. No production
SuffixAutomaton source was changed by this work.

## Experiments and exact separation

All comparisons use the full baseline commit resolved from `9c2914f` in their
own `prepared.json`. Immutable baseline/candidate header snapshots,
`candidate.patch`, exact compiler commands/versions, harness/header/binary
SHA-256 hashes, input/result hashes, all raw samples, and environment/process
snapshots accompany every experiment.

| Result directory | Candidate | Assertion mode | Samples + warmup per profile | Process profiles |
| --- | --- | --- | --- | ---: |
| `static-top-tree-sam` | STT shortcuts plus SAM clone move in one include root; each workload uses only its family | `-DNDEBUG` | 5 + 1 | 360 |
| `suffix-automaton-insertion` | SAM `try_emplace` only; original clone copy and identical STT headers | `-DNDEBUG` | 5 + 1 | 120 |
| `static-top-tree-assert` | STT shortcuts only; identical original SAM headers | assertions enabled | 5 + 1 | 240 |
| `suffix-automaton-insertion-assert` | SAM `try_emplace` only; original clone copy and identical STT headers | assertions enabled | 5 + 1 | 120 |
| `static-top-tree-assert-recheck` | Exact binaries from the STT assertion experiment; three focused workloads | assertions enabled | 9 + 1 | 120 |

Total: **960 timed process profiles, including 152 warmups**, leaving 808
measured profiles. There were also 210 small preparation checks across these
four preparations and 70 separate large allocation-diagnostic profiles.
Unused combined assertion preparation was retained only in ignored
`.build/static-top-tree-sam-assert/unused-preparation/`; it contributed no
timing results and is not counted above.

GCC 14.2.0 and Clang 19.1.7 use `-std=gnu++20 -O2 -Wall -Wextra`, the exact
immutable include roots, and repository/ACL includes. Release adds
`-DNDEBUG`; assertions-enabled runs omit it. Only separate GCC allocation
diagnostics add `-DPROFILE_ALLOCATIONS`. No `-march=native` or explicit ISA
flag is present. Results from different assertion modes/experiments are never
pooled. Before/after execution alternates, with one warmup and all samples
retained. Source/header/binary hash changes abort execution.

## Scope and limits

StaticTopTree uses N=100,000 and Q=200,000 on path, star, balanced, random,
and broom trees. Both uint64 affine DP (arithmetic modulo 2^64) and
modulo-998244353 affine DP are measured. Construction is separate from
`set` followed by `all_prod` and a result checksum. The candidate preserves
callback counts and expression construction; diagnostics found no update
allocations.

SuffixAutomaton uses N=200,000, Q=20,000, random-4/random-26 alphabets,
periodic/equal strings, and generic integer symbols. Its phases are
construction, the first lazy count rebuild, repeated count/contains, and
LCS on a second length-N sequence. State counts and result checksums matched across all
candidate comparisons.

All phase timers exclude input generation, process startup, output, and
structure destruction. These are in-memory library profiles; no parse/format
speed improvement is claimed. Other agents paused heavy work during timings.
The shared host, scheduler, CPU frequency, and unpinned execution remain
limitations. Wide sample ranges and outliers are kept, not filtered or
replaced with zero.

RSS is Linux process peak in KiB, including generated inputs and runtime
overhead. A small shell launches each benchmark child to avoid inheriting
Python's high RSS floor. It is not precise live allocation usage or vector
capacity. Allocation diagnostics count ordinary `operator new/new[]` calls
and total requested bytes while each phase is active. They are separate
instrumented builds with all durations discarded, not peak-live byte counts.

## StaticTopTree adoption evidence and exceptions

Release construction medians improved on all 20 compiler/shape/scalar
profiles, by 2.1–28.4%. Initial assertion construction improved on 18/20.
Construction allocation calls decreased by 99,998 on the star, 74,999 on
the balanced tree, 40,908 on the random tree, and 49,999 on the broom.
The path retained 61 allocations and only exercises the recursive shortcut.

The original assertion exceptions remain explicit:

- GCC balanced/uint64 construction: 15.551 to 17.391 ms, +11.8%.
- Clang path/uint64 construction: 10.853 to 11.855 ms, +9.2%.
- Clang broom/mod998 operations: 65.348 to 71.534 ms, +9.5%.

A separate nine-sample recheck reused the exact original binaries and
retained all earlier results. Those three regressions did not repeat:

| Focused check | Before median [min,max] ms | After median [min,max] ms |
| --- | ---: | ---: |
| GCC balanced/uint64 construction | 18.034 [16.191,19.575] | 16.030 [13.084,18.312] |
| Clang path/uint64 construction | 11.490 [8.893,12.665] | 10.397 [8.445,12.003] |
| Clang broom/mod998 operations | 69.780 [60.689,93.494] | 67.510 [61.672,94.453] |

The focused run itself had a smaller contrary result: GCC path/uint64
construction increased 3.6%, from 12.088 [10.160,13.934] to
12.526 [9.734,13.987] ms. GCC broom operations increased 1.9%; the other
five focused operation medians decreased. These measurements support
reducing construction work, especially on branching trees, while they do
not establish a universal speedup or an update-speed improvement. The
observed update regressions were not reproduced consistently; they are not
erased or declared impossible.

## Rejected SuffixAutomaton changes

Clone movement saved 139,033 / 52,722 / 51,255 construction allocations on
random-4 / random-26 / integer inputs, but timings varied in direction.
GCC random-26 became 7.6% slower; clone-free controls also changed timing.
The first experiment's complete report and negative samples are preserved.

`try_emplace` retained original clone copying and made no allocation-count
change. In release mode GCC random-26/integer construction improved
5.9%/6.1%, while Clang random-4/random-26/equal regressed 9.5%/7.7%/8.6%.
Assertions-enabled random-4 construction regressed on both compilers;
Clang integer construction regressed 10.3%. Since the benefit was not
consistent and several representative profiles worsened, this candidate
was also rejected. No third SAM candidate was attempted.

## Reproduction

Run from the repository root with GCC, Clang, and ACL available. In the saved
cloud environment, first `source /tmp/blueberry-setup/env.sh`.

```console
python3 benchmark/static-top-tree-sam.py prepare
python3 benchmark/static-top-tree-sam.py check
python3 benchmark/static-top-tree-sam.py run
python3 benchmark/static-top-tree-sam.py allocations

python3 benchmark/suffix-automaton-insertion.py prepare
python3 benchmark/suffix-automaton-insertion.py check
python3 benchmark/suffix-automaton-insertion.py run
python3 benchmark/suffix-automaton-insertion.py allocations

python3 benchmark/static-top-tree-assert.py prepare
python3 benchmark/static-top-tree-assert.py check
python3 benchmark/static-top-tree-assert.py run
python3 benchmark/static-top-tree-assert.py allocations

python3 benchmark/suffix-automaton-insertion-assert.py prepare
python3 benchmark/suffix-automaton-insertion-assert.py check
python3 benchmark/suffix-automaton-insertion-assert.py run
python3 benchmark/suffix-automaton-insertion-assert.py allocations

python3 benchmark/static-top-tree-assert-recheck.py
```

Coordinate a quiet window before each `run` or focused recheck; preparation
and correctness work must finish first. Each output directory refuses to
overwrite existing preparation/timing artifacts. On this already-measured
workspace, preserve/move the existing corresponding result directory before
reproduction. The focused recheck requires the original assert preparation
and reuses its binaries without recompilation.
