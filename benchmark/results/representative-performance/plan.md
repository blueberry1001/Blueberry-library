# Representative data structure survey

Baseline: `ca8a4df35269a6980cf0b9612878e420744fc860`. No production changes are made by this harness. Prior StaticTopTree/SAM artifacts remain independent.

The bounded survey measures existing dense DSU, Fenwick and SegmentTree against their shared-operation ACL subsets, and evaluates two isolated point-access candidates. Dense Fenwick block cancellation and bit_floor, and SegmentTree no_unique_address, were already shipped; this survey does not reimplement them.

## Reproduction and frozen inputs

From the repository root, with the provided compiler environment:

```sh
source /tmp/blueberry-setup/env.sh
python3 benchmark/representative-performance.py prepare
python3 benchmark/representative-performance.py check
# Reserve a quiet window with other workers before these commands:
python3 benchmark/representative-performance.py run
python3 benchmark/representative-performance.py diagnostics
```

Preserve/move existing corresponding result directories before reproducing an experiment; preparation and measurement intentionally refuse to overwrite completed artifacts. `--experiment dense|dynamic-fenwick|persistent-segment` and `--mode release|assert` restrict each action. Exact compiler commands, versions, header/source/ACL/binary SHA-256 values, and selected include environment are in each prepared.json. Source snapshots originate from git show of the exact baseline. Each candidate include root changes only its target header. GCC14 and Clang19 use `-std=gnu++20 -O2 -Wall -Wextra`; release adds `-DNDEBUG`. No ISA tuning is used.

Each matrix cell uses one warmup then five measured repetitions, alternating compiler/implementation order. Raw rows retain every result and checksum; reports give median/min/max. Process snapshots omit command arguments. Small profiles use N127/Q211 and brute-force oracles, outside retained timing sections. Timing starts only after those checks and a coordinated quiet window.

The deterministic RNG uses the harness mix function with these initial states: dense DSU `0x46454e4345445355`, dense arrays `0x44454e5345415252`, DynamicFenwick `0x44594e46454e`, PersistentSegmentTree `0x50455253495354`. See [the shared harness](../../representative-performance.cpp) and [dense module](../../representative-dense.hpp) for the exact generator and generation order.

## Workloads and stage boundaries

Dense profiles use N100,000/Q200,000 and deterministic random or sequential/clustered patterns. DSU discards incompatible merge return values and checksums connectivity/component-size results. Fenwick zero construction is separate from the same N initialization adds in both implementations; no linear bulk constructor is compared against repeated ACL adds. Fenwick point gets are compared with ACL sum(p,p+1), which returns the same value but has a different implementation cost. SegmentTree immediate value consumption avoids reference-lifetime API differences. Uint64 arithmetic wraps identically. Each implementation receives identical generated inputs. Dense query_ns is the sum of its two separately measured query stages, not an additional interval; initialization_ns is zero/not applicable for vector-built SegmentTree. Dense allocation counts are unavailable and are not reported as zero.

Sparse Fenwick uses N30,000 initialization adds, Q/4 further adds, Q point gets and Q/8 range sums in separate stages. Its logical signed-coordinate domain is 2^40-1. Broad coordinates, a 4096-coordinate hotspot, and positions beside powers of two exercise misses/hits and varied lowbit paths. Three of four point queries select an initial update coordinate; every fourth uses a freshly generated coordinate. Uint64 and mod998244353 values are separate cases. The candidate accumulates removed blocks using existing +=, then performs one binary subtraction. It does not require assignment of T or subtract-assignment and does not insert map entries on reads. The removed-block accumulation is an initial subsequence of the original pref(p) walk, so for valid signed arithmetic it introduces no new overflowing partial sum; the final difference is the same representable point value. Floating-point association can differ, and exact arithmetic callback counts are not an API guarantee. Exact value_at call counts are derived from the queries: baseline popcount(p)+popcount(p+1), candidate 1+ctz(p+1).

Persistent SegmentTree uses N100,000/Q200,000; vector build or implicit identity build; Q/4 version-creating set/apply operations; Q point reads; Q/8 range queries; then Q/4 interleaved operations with one set for every four gets. Dense sequential, dense branching, and implicit-identity branching version histories are separate. Uint64 sum and noncommutative affine composition over uint64 wrapping arithmetic exercise generic monoids. Queries may read any earlier version. The candidate descends only the selected child to its leaf or implicit identity node; no prod/set/apply change is made. The mixed phase includes new-version allocation and is not a pure point-query result.

Query timings include checksum accumulation (and scalar/affine value hashing in candidate profiles), with identical work in each before/after pair; they are measured stage costs rather than isolated accessor latency. Construction/initialization/update/get/range/mixed timings exclude input generation, process startup, checks, output and destruction. These are data-structure microbenchmarks; no parsing/formatting or end-to-end I/O speed claim is made. Linux process peak RSS includes input vectors, allocator retention, runtime, and all phases. Separate instrumented GCC runs count requested allocation calls/bytes and arithmetic or monoid operations, with their timings discarded. Those byte totals are cumulative, not peak-live memory, and hash call counts are not hash-bucket collision/probe measurements.

## Decision policy

Retain all negatives. An ACL difference is a survey observation, not authorization to rewrite a dense implementation. Adopt neither point-access candidate solely from theory or allocation counts: compare compiler/assert modes and point-read benefit against build/update/range/mixed controls. Keep production headers unchanged until the parent reviews measurements and correctness evidence. No Fastest/AOJ network retries, uploads or credentials changes are part of this survey.
