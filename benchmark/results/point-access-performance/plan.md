# Final production point-access profiles

These experiments follow the representative survey without modifying or replacing any survey evidence. Baseline is exact `ca8a4df35269a6980cf0b9612878e420744fc860`. Each candidate is a direct snapshot of the parent-confirmed frozen production target header, checked against an explicit SHA256; no string-generated implementation is used. The other four DS headers remain the exact baseline in both include roots. Each family and assertion state has a separate immutable preparation/results directory.

Only DynamicFenwick is authorized initially. Persistent final-code preparation/timing requires a later production freeze and parent authorization; the reusable harness already contains its validated workload, but it is not run by default.

## Reproduce

```sh
source /tmp/blueberry-setup/env.sh
python3 benchmark/point-access-performance.py prepare --experiment dynamic-fenwick --production-sha256 34ae81541b179c2dd2bc24e7573d2f5540deb37622f91eb9152ae76c2c3a5126
python3 benchmark/point-access-performance.py check --experiment dynamic-fenwick
# Coordinate a quiet window after all integration/validation jobs finish:
python3 benchmark/point-access-performance.py run --experiment dynamic-fenwick
python3 benchmark/point-access-performance.py diagnostics --experiment dynamic-fenwick
```

Existing completed experiment directories must be preserved/moved before reproduction. The runner refuses to overwrite preparation or timings. `--mode release|assert` selects one state; default is both. Exact compiler commands, compiler versions, source/header/ACL/binary hashes and selected include environment are recorded. GCC14/Clang19 use gnu++20, O2, Wall/Wextra; release alone adds DNDEBUG. No architecture-specific tuning. Runtime checks reject changed sources, headers, binaries or runner, and require the current successful small-case gate. Process failure, timeout, malformed JSON/schema and checksum-rejection evidence are retained.

One warmup plus five measured samples alternate implementation/compiler order, in separate processes. Every row retains input/result checksums. Main Dynamic matrix: broad/hotspot/power-of-two boundary × uint64/mod998 × GCC/Clang × release/assert × baseline/candidate, 288 profiles including48warmups. Separate instrumented GCC diagnostics:24profiles. Small N127/Q211 profiles use full point/range oracles and mixed replay,72profiles across both modes. Durations from small checks and diagnostics are discarded.

## DynamicFenwick stage scopes

The deterministic RNG starts at `0x44594e46454e` and uses the exact mix/generation sequence in [point-access-performance.cpp](../../point-access-performance.cpp). Logical coordinate domain is2^40−1 (1023 for small checks). N30,000 initialization adds, then Q/4=50,000 adds, Q=200,000 gets, Q/8=25,000 range sums are timed separately. These sequences match the earlier survey; the new phase is generated only after them.

A final mixed phase has Q=200,000 interleaved operations: one add followed by four gets (40,000adds/160,000gets). Each first get observes the latest added coordinate; remaining gets mix known initialization/new-update coordinates and fresh coordinates from the same distribution. This exposes updated values, missing coordinates and map growth. The mixed checksum is independently reproduced with a plain-array oracle in small cases; final values are also checked. Mixed timing includes additions, possible map allocations and checksum work. It is not pure getter latency. All old separated-stage timings and new mixed timings remain independent fields.

Input generation, startup, output, oracle work and destruction are excluded. Query checksum/value hashing is included identically. No parsing/formatting or end-to-end I/O claim is made. Process peak RSS includes inputs, allocator retention, runtime and all stages. Separate diagnostics count ordinary new/new[] requests and arithmetic operations, not peak-live bytes. Derived value_at loop counts are exact calls for these coordinates, not hash bucket collision/probe counts. Both original and mixed point-query call counts are recorded.

Construction, initialization, add and range controls remain unchanged implementation paths; regressions and sample overlap must be reported rather than attributed automatically to the get change. The parent decides adoption after final-code results and the independent production validation suite. Shared-host/unpinned and cgroup quota limits are captured with the measurement environment.

## Reusable Persistent scope (held pending its own batch)

Its earlier validated profile remains N100,000,50,000version-creating set/apply operations,200,000gets,25,000ranges and50,000mixed20%set/80%get operations. Dense sequential, dense branching and implicit-identity branching histories use uint64 sums and noncommutative affine values. Its RNG seed is `0x50455253495354`. It will get separate frozen production-header snapshots, small gates and measurements; Dynamic results will not be overwritten or recombined with a changed harness.
