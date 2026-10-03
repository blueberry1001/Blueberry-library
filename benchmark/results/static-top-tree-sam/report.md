# StaticTopTree singleton shortcuts and SuffixAutomaton clone movement

Baseline: commit `9c2914f` (full SHA in `prepared.json`). This is the first,
release-only experiment. Its immutable candidate snapshots change StaticTopTree
singleton handling and SuffixAutomaton clone movement together; each workload
uses only its own family. Later assertion-enabled and insertion experiments
have separate include roots and result directories. No network research or
judge-access retry was performed for this batch.

## Changes and contracts

StaticTopTree returns the sole item before allocating a weight-prefix vector,
and returns a singleton recursive interval before calling `upper_bound`.
The weighted grouping, expression-node creation order, callback counts, API,
and complexity remain unchanged. Direct `<cstddef>` replaces unused
`<functional>` in the candidate include list.

The SuffixAutomaton candidate moves an already-owned cloned `State` into
`states_` instead of copying it a second time. It retains the first required
map copy, generic `Symbol`, transition representation, node IDs, and public
operations. Its snapshot adds a direct `<utility>` include. This candidate
was **not adopted**, because saved allocations did not give a consistent
measured runtime improvement.

## Reproduction and measurement scope

```console
source /tmp/blueberry-setup/env.sh
python3 benchmark/static-top-tree-sam.py prepare
python3 benchmark/static-top-tree-sam.py check
# Coordinate a quiet window before the next command.
python3 benchmark/static-top-tree-sam.py run
python3 benchmark/static-top-tree-sam.py allocations
```

The setup line is specific to the recorded cloud environment; an ordinary
machine needs GCC/Clang and ACL on its include path. The scripts refuse to
overwrite an existing preparation or timing experiment. Preserve the existing
directory before reproducing into that output location.

GCC 14.2.0 and Clang 19.1.7, `-std=gnu++20 -O2 -DNDEBUG`, on an Intel Xeon
Platinum 8573C Linux host. No ISA-specific flags. Five measured samples and
one warmup alternate before/after and compiler order. Other agents paused
builds/tests. External shared-host load and CPU-frequency changes remain
possible; no CPU affinity was requested. Commands, header/harness/binary
hashes, raw samples, medians, minima, maxima, and process snapshots are saved.

- StaticTopTree: N=100,000, Q=200,000, path/star/balanced/random/broom trees;
  uint64 affine DP and modulo-998244353 affine DP. Each update is followed by
  `all_prod()` and a checksum. Construction and operations are separate.
- SuffixAutomaton: N=200,000, Q=20,000; random alphabets of size 4 and 26,
  periodic and equal strings, and generic integer symbols. Construction,
  first lazy occurrence-count rebuild, repeated count/contains, and LCS on a
  second length-N sequence are separate.
- Input generation, process startup, output, and structure destruction are
  outside phase timers. RSS includes generated inputs and is collected from
  a child of a small shell, avoiding Python's inherited RSS floor.
- Allocation/callback instrumentation uses separate GCC binaries. Its
  durations are discarded; requested allocation bytes are cumulative, not
  peak-live storage. Callback labels are vertex, edge, compress, rake,
  identity.

All 90 small preparation profiles and all 360 large profiles (300 measured,
60 warmups) agreed on input and result checksums. The 30 separate diagnostic
profiles also agreed; StaticTopTree callback counts were identical. These are
before/after comparisons, supplementing the independent correctness and
official tests handled by integration.

## StaticTopTree results

Construction medians, milliseconds:

| Shape / scalar | GCC before | GCC after | Clang before | Clang after |
| --- | ---: | ---: | ---: | ---: |
| Path / uint64 | 13.191 | 12.612 | 11.976 | 11.466 |
| Path / mod998 | 8.972 | 8.581 | 8.823 | 8.355 |
| Star / uint64 | 20.324 | 17.972 | 19.176 | 18.271 |
| Star / mod998 | 15.540 | 12.999 | 15.778 | 13.965 |
| Balanced / uint64 | 22.591 | 16.185 | 18.178 | 16.163 |
| Balanced / mod998 | 16.257 | 13.027 | 16.965 | 13.601 |
| Random / uint64 | 38.692 | 36.749 | 28.616 | 25.890 |
| Random / mod998 | 28.285 | 26.805 | 29.276 | 26.423 |
| Broom / uint64 | 14.636 | 14.078 | 14.939 | 13.186 |
| Broom / mod998 | 11.599 | 9.712 | 11.802 | 11.549 |

Construction medians improved by 2.1–28.4% across these profiles. The
allocation diagnostics explain a structural reduction rather than a changed
DP: allocation calls decreased by 99,998 on the star, 74,999 on the balanced
tree, 40,908 on the random tree, and 49,999 on the broom. The path retained 61
construction allocations, so its smaller gain comes from avoiding singleton
search/recursion work. Both implementations allocated zero times in updates.

Operations were unchanged in implementation and callback counts, with mixed
median differences of roughly ±5%. No update-speed improvement is claimed.
For example, GCC balanced/mod998 operations measured 69.673 ms before and
73.084 ms after; the respective ranges were 68.743–76.995 and 69.515–74.181 ms.
Clang measured 76.769 versus 76.085 ms for the same workload. Construction
there was 16.257 [13.236,18.306] versus 13.027 [11.646,13.263] ms under GCC,
and 16.965 [15.786,17.728] versus 13.601 [11.521,14.804] ms under Clang.
All ranges and the other phases remain in `results.json`/`raw.json`.

Decision: adopt the small StaticTopTree construction shortcuts, subject to
the separate assertion-enabled follow-up and integration checks. Preserve
the existing update implementation and weighted balancing algorithm.

## SuffixAutomaton clone-move results

Construction medians, milliseconds:

| Input | GCC before | GCC after | Clang before | Clang after |
| --- | ---: | ---: | ---: | ---: |
| Random 4 | 104.749 | 104.711 | 118.667 | 121.377 |
| Random 26 | 105.819 | 113.865 | 107.640 | 107.776 |
| Integer symbols | 114.913 | 113.424 | 116.369 | 110.103 |
| Periodic | 18.996 | 20.321 | 19.915 | 20.135 |
| Equal | 20.186 | 19.634 | 21.885 | 19.038 |

The move saved 139,033 allocation calls on random-4 input, 52,722 on
random-26, and 51,255 on integer input. Periodic/equal input created no
clones and saved no allocations. These real reductions did not produce a
consistent speed benefit. GCC random-26 was 7.6% slower; its before/after
ranges were 98.718–116.518 and 109.377–127.183 ms. Clang random-26 was nearly
unchanged, with ranges 101.285–115.371 and 97.480–111.650 ms. Some improvements
on clone-free controls further caution against assigning all timing changes
to the edited statement.

Decision: retain the original clone copy in production. The negative result
is preserved in full. A subsequent, isolated experiment evaluates replacing
the separate `contains`/`emplace` lookups with `try_emplace`; it is not combined
with this clone-move change.
