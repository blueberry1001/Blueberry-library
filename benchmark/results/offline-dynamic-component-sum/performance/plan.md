# OfflineDynamicComponentSum: bounded storage comparison

Frozen production header SHA256: 7d54e1694d42da12503677a76f853197fc592ae351e60d6cf916cbb2694e6be9. Compare actual CSR implementation with an alternate class generated from these exact bytes. The alternate replaces offsets/cursor/events with vector<vector<int>> and the matching iteration loop. Event IDs, visit_events order, query-time compression, interval extraction, aggregate copies/undo, RollbackUnionFind, recording API and lifecycle are identical. control.diff records the complete change. No third-party code is used as a measured API baseline.

Contract: initially empty undirected multigraph, canonical unordered endpoints, multiplicities, self-loops, absent removal returns false without recording an event; add_value persists across edge removal. Query IDs are insertion order, solve is const/repeatable, further calls may be appended. T only needs copies/assignment and associative commutative +; no zero, default construction, subtraction or comparison. Intermediate reordered sums must be representable. Performance uses long long; a nondefault/noninverse/noncomparable value type is also instantiated in smoke checks. Parent validation owns the comprehensive public contract tests and official driver.

## Fixed inputs and comparisons

N/Q call counts:
- long:300000/300000,100000 long-lived edges followed by100000 vertex updates and100000 queries.
- churn:300000/300000, add edge/query/add value/remove reversed edge/query,60000 groups,120000 queries.
- updates:300000/300000,nine updates per query,270000 updates and30000 queries,no edges.
- cyclic:4096/300000,edge multiplicities in a fixed chord pool,40% adds/20% removal attempts replaced with adds when absent/20% vertex updates/20% queries. Actual query count is recorded, not assumed.

All performance removals are successful. Independent BFS smoke also tests absent removals, duplicate/reversed/self edges, mutations between queries, repeated solves and appending after solves. Initial values i%29-14 and deltas[-9,9] fit safely in signed64 even under reordered partial sums. Generation seed0x4f44435f42454e43; oracle seed0x4f44435f4f524143. Input generation/hashing is outside timing.

GCC14/Clang19,gnu++20,-O2,-Wall,-Wextra; release adds-DNDEBUG. Four cells×four compiler/mode combinations×two storage layouts×(one warmup+three measured repetitions)=128kernel profiles. Variant order alternates. Median/min/max retained separately per compiler/mode/workload; no pooling. Shared unpinned host and3sample ranges limit inference; min/max are not confidence intervals.

Five sequentially compiled binaries:four normal compiler/modes plus GCCrelease allocation instrumentation. Each checks217programs against independent BFS, including prefix/repeated solves, plus the custom type contract. First binary compares all output elements between layouts for all4large cases; forty small cross-binary profiles then bind input/result/count hashes. Large timings must match the previously compared outputs. Failed commands retain stdout/stderr; earlier raw rows are flushed and preserved. Live production/dependency/source/runner/binary hashes are checked before each phase.

## Timed boundaries

constructor_ns copies initial values into the recorder; registration_ns records all operations and checks query IDs; solve_ns includes query compression materialization, edge intervals, bucket construction, rollback traversal and returned answers. full_ns includes all these plus recorder destruction. Timed phases are from the same call; no hidden internal stage claim. Input vectors exist before full timing; the result remains alive afterwards. Registration is unchanged between candidates and acts as a control.

Public API fixed, one churn input, FastIO versus unsynchronizediostream independently varied. Two release compilers×two I/O methods×(warmup+three measurements)=16I/O profiles. Parse includes input-vector construction; algorithm includes constructor/registration/solve/destruction; format includes explicit flush. End-to-end excludes process startup. Output bytes and answer hashes must agree. Assert-enabled I/O is not separately measured; both modes are covered in the kernel matrix.

Separate GCCrelease diagnostics on all4cells×two layouts:8allocation and8uninstrumented fresh-process RSS profiles. Allocation counts/requested bytes cover the complete API lifetime; allocation-instrumented times are not compared. RSS includes input, runtime and allocator high-water, not only buckets. Small shell parent avoids inherited Python peak RSS. No extra staged algorithm call is performed in these processes.

## Complexity and lifetime interpretation

K denotes queries, not all recorded calls Q. Tree size is2*bit_ceil(K), and solve with K=0 returns immediately. Let E be nonempty positive-multiplicity edge lifetimes visible to queries and U be updates before the last query. Canonical event references require O((E+U)log(K+1)) storage; DSU leader/merge is O(log(N+1)), without path compression. Include Q-recording/map costs, O(N) copies, O(K) answers/tree traversal, and event processing times; do not report O(QlogQ) while hiding UF logN.

CSR has an offsets array plus a temporary cursor during construction; the cursor is destroyed before traversal/history allocation. Nested buckets have a vector object at every tree node, per-bucket capacity slack and allocations. Both retain recorder data, visible edge intervals, event references, initial sums, result capacity and undo history as required by their respective lifetimes. Peak RSS cannot be inferred from final event storage alone. Vertex update payloads are stored once in the recorder; buckets carry int event IDs, not repeated T copies.

## Sources and reproduction

Primary algorithm studies, not copied code:
- Nyaan offline connectivity: https://github.com/NyaanNyaan/library/blob/master/graph/offline-dynamic-connectivity.hpp (time-segment buckets, multiplicities, callbacks, rollback). Its explicit-time/callback API differs from this class.
- KACTL CC0 rollback UF: https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/UnionFindRollback.h (union-by-size without path compression).
- Official pinned reference: https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/graph/dynamic_graph_vertex_add_component_sum/sol/correct.cpp (online Euler-tour/HDT-style algorithm; stronger online contract, not an apples-to-apples offline speed baseline).

Official19datasets are prepared separately at ../datasets/report.json,38hashes matching the pinned manifest. Dataset self-tests are not API AC claims. No AOJ or blocked Fastest endpoint was retried.

After sourcing /tmp/blueberry-setup/env.sh, use benchmark/offline-dynamic-component-sum.py prepare --experiment NEW --production-sha256 <hash>, then measure, diagnostics and summarize with the same new experiment name. Existing experiments/raw files are refused. Timings require the root-approved quiet window after validation jobs finish. No archives or previous evidence mutation.

Preparation amendment: initial/ failed before performance at cross-compiler input identity because multiple RNG draws in call arguments had unspecified evaluation order. The failed snapshot/logs remain. ordered-generation/ uses explicit vertex-then-delta local draws; no library or candidate algorithm change.
