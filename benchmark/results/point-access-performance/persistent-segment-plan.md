# PersistentSegmentTree final production batch

This is the second implementation batch. DynamicFenwickTree was already validated and committed as `5182af3cdb4f05a24ae8978b0599d67c38f1eb70`; none of its evidence is overwritten. The common benchmark C++ harness, preparation/measurement runner, and shared summary generator remain byte-identical to that commit.

The comparison baseline is exact `ca8a4df35269a6980cf0b9612878e420744fc860`. The candidate PersistentSegmentTree header is copied directly from the frozen production file with SHA256 `880c5340010c78de2e96490213f561ea46589fc92e886386637ae56756025366`. The checkout HEAD at preparation is the Dynamic commit `5182af3cdb4f05a24ae8978b0599d67c38f1eb70`; the uncommitted Persistent change is identified by its separate production-header SHA. All four companion headers remain the exact ca8 baseline in both include roots, including the older DynamicFenwick header, which is unused by Persistent profiles. These experiments therefore isolate the Persistent change.

The only production-code difference is point `get`: preserve index/version checks, descend one child per level, return the leaf or implicit identity. Range queries, version creation, ownership/value return, arbitrary version access, and noncommutative monoid support are unchanged. The new operation skips identity combinations; an exact callback invocation count is not an API guarantee.

## Reproduction

```sh
source /tmp/blueberry-setup/env.sh
python3 benchmark/point-access-performance.py prepare --experiment persistent-segment --production-sha256 880c5340010c78de2e96490213f561ea46589fc92e886386637ae56756025366
python3 benchmark/point-access-performance.py check --experiment persistent-segment
# Reserve a quiet window after production validation/compilation completes:
python3 benchmark/point-access-performance.py run --experiment persistent-segment
python3 benchmark/point-access-performance.py diagnostics --experiment persistent-segment
python3 benchmark/persistent-segment-performance-summary.py
```

Preserve/move the corresponding existing Persistent result directories before reproducing; immutable preparations and raw timings are never overwritten. `--mode release|assert` restricts one configuration, default both. No old Dynamic or representative-survey files are changed by these commands. The Persistent-only summary wrapper calls the committed summary generator for Persistent and adds this plan link and Persistent-specific context to its new output only.

The runner verifies its hash, C++ source and header snapshots, ACL headers, binary hashes, the live production target SHA, and current successful small-case gates before measurement. It retains process failures, timeouts, malformed/missing JSON fields, and checksum disagreements. Exact commands, GCC14/Clang19 versions and dependencies are recorded in prepared.json. Flags are `-std=gnu++20 -O2 -Wall -Wextra`; release additionally uses `-DNDEBUG`, assert does not. No architecture-specific flags are used.

## Workloads and stage scopes

The deterministic RNG seed is `0x50455253495354`, with exact mixing and generation order in [the frozen shared harness](../../point-access-performance.cpp). N=100,000 and Q=200,000. Each profile separates:

1. Construction from a dense vector or an implicit identity array.
2. Q/4=50,000 new versions, alternating `set` and `apply`.
3. Q=200,000 point `get` calls over earlier versions.
4. Q/8=25,000 range `prod` calls.
5. Q/4=50,000 mixed operations: 10,000 new-version `set` operations interleaved with 40,000 `get` calls (20%/80%).

Histories are dense sequential, dense branching, and implicit-identity branching. Values are uint64 addition or noncommutative affine composition over uint64 wrapping arithmetic. Earlier versions remain queryable after later updates. The mixed phase creates additional versions and includes allocation costs; it is not isolated getter latency. The generator chooses only versions already available at each operation.

Small profiles use N=127/Q=211. An independent vector for every version is reconstructed, every point and all_prod are checked for every version, and range results are checked against direct ordered folding. Mixed-created versions are replayed in order and checked too. These 72 profiles cover both compilers, assertion modes, both variants and separate GCC diagnostic builds. Their timings are discarded.

The main matrix has three histories × two monoids × two compilers × two assertion modes × two variants × (one warmup + five alternating measured runs) = 288 profiles, including 48 warmups. Baseline/candidate and compiler order alternate by repetition. Separate instrumented GCC diagnostics add 24 profiles, with durations discarded. Every input/result checksum must match across implementations.

Input generation, startup, oracle work, output and destruction are outside recorded stages. Query checksum and affine value hashing are inside the same stage in both variants. Parsing/formatting and end-to-end I/O are not measured, so no I/O improvement is claimed. Allocation counters represent ordinary new/new[] calls and cumulative requested bytes, not peak-live capacity. Monoid combine counters identify removed point-read work. Process RSS includes inputs, allocator retention, runtime and all phases; RSS differences are not attributed to getter memory savings.

Build/update/range implementations are unchanged controls. Retain their regressions and ranges; do not credit their improvements to the getter change. Results are from a coordinated team quiet window on a shared, unpinned host; quota/affinity are captured separately, and other tenants or frequency scaling cannot be ruled out. Baseline and candidate must use the same input/flags/host conditions. Final-code results remain separate from prior survey or Dynamic results.

## Status and report

The shared runner's historical conditions field points at the original common plan; this new file supplies the finalized Persistent scope and commands. Final report and full stage CSV are new `persistent-segment-final-code-report.md` and `persistent-segment-stage-comparisons.csv`. Production integration validation and final adoption/commit remain the parent agent's responsibility. No upload, remote-access changes, new optimization candidates or network retries are part of this measurement batch.
