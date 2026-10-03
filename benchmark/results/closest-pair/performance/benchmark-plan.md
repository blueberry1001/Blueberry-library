# Closest-pair bounded comparison

Scope: exact production `blueberry::closest_pair`, plus independent per-node-index-vector divide/conquer and balanced-tree sweep controls. All measured coordinates are signed `long long`, |coordinate|<=2^62-1, distance is exact signed128, and the result is the lexicographically smallest normalized original-index pair. Empty/singleton return {-1,-1}; debug builds validate even a singleton. Public signed-type acceptance/rejection is validated separately by the library's tests; performance results do not claim every type is measured.

Production header is snapshotted directly and every measurement rechecks live/header/harness/runner/binary/input hashes. Public calls remain intact. Independent controls copy records, sort once by x/y/index, and scan all duplicate groups before their positive-distance algorithms. This duplicates scan is necessary to preserve the lexicographic tie contract without destroying the packing bound. The vector control reserves per-node output/strip vectors; it is not an intentionally unreserved strawman. Sweep uses a balanced tree and corrects an approximate square root with exact unsigned128 comparisons. Inclusive x/y bounds preserve equal-distance index ties. Both are deterministic O(N log(N+1)), O(N) peak auxiliary storage.

Primary studies (no source copied into the production header or independent controls):

- Library Checker pinned1814c4e5205517e368bb57a8d1127eb961cfeaae geo/closest_pair/sol/correct.cpp: divide/conquer, per-node returned y-index vectors. Reference contract N>=2, |coordinate|<=1e9, signed64 distances, any minimizing pair. https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/sol/correct.cpp
- KACTL ClosestPair.h, CC0, Simon Lindholm: balanced-tree sweep, approximate sqrt window, signed64 distance, coordinate-pair return. Retrieved2026-10-03 from https://raw.githubusercontent.com/kth-competitive-programming/kactl/main/content/geometry/ClosestPair.h ; local source SHA256919f6d8e07d2f700c58259a05ff98ab4374aa6fb548f71b46eacbaa463f34ed9. This mutable URL is bound by saved content hash, not a claimed pinned upstream revision.
- maspypy closest_pair.hpp: per-node-vector DC and randomized hashing/grid variant. Retrieved2026-10-03 from https://raw.githubusercontent.com/maspypy/library/main/geo/closest_pair.hpp ; SHA256f6d6d20218818f127e4d146b13c41174bf630dd3a02e39ee949b1f317323eddf. Study only; no license-dependent copying. Randomized expected behavior and narrower arithmetic/tie guarantees differ, so no measured direct replacement comparison is claimed.

Fastest/leaderboard research remains blocked by the previously recorded API403; no denied API/AOJ path was retried. No fastest ranking claim.

## Fixed experiment

GCC14 and Clang19, -std=gnu++20 -O2 -Wall -Wextra, release adds -DNDEBUG. One compiler at a time during preparation. Timing starts only in the root-approved quiet window after all other tests/builds finish.

Six cells: randomN20000, randomN100000, duplicatesN100000, gridN100000, nearlineN100000, extremeN100000. Generation seed0x43504c4f53455354, with shuffled original indices. Grid provides abundant positive-distance ties; duplicate input provides zero-distance ties; wide input includes four coordinate corners. Small correctness uses all512 subsets of3x3 grid in forward/reverse order,45distribution/size cases,2explicit adversarial cases,1000random cases(seed0x43504f5241434c45); each of2071cases compares all3exact returned pairs against independently accumulated unsigned128 brute force. Debug negative singleton must terminate by SIGABRT.

Three measured repetitions plus one warmup per compiler/mode/cell/variant, rotating order each repetition:288kernel profiles,72warmups. Raw JSONL preserves every successful profile immediately; rejected commands retain stdout/stderr separately, earlier rows survive. All variants/compiler/modes must match input digest and exact pair. Summary retains median/min/max, never pooling compiler/mode/distribution. Three repetitions and shared unpinned host limit inference; min/max are observed sample ranges, not confidence intervals.

`full_ns` calls the intact public header or complete independent control including validation/copy/sort/search/destruction. `prepare_ns` and `search_ns` only exist for controls and are recorded in a separate instrumented call: preparation includes validation, record copy, sort, duplicate scan; search includes the actual DC/sweep or duplicate early return. These phases are diagnostic, nonadditive to `full_ns`; public phases are null. Comparators/layout differ as well as allocation strategy, so a full-call difference cannot be assigned solely to allocations. Input generation/hashing is outside algorithm timers.

I/O: exact public header, independently switch FastInput/FastOutput versus unsynchronizediostream. Random100000 and10000tiny four-point testcases,4compiler/modes,3repeats+warmup,64profiles(16warmups). Parse includes input-container construction, solve includes output-vector creation and public calls, format includes explicitflush. End-to-end is parse+solve+format within one process and excludes process startup. Output bytes must match exactly. No cross-family I/O/algorithm change conflation.

Separate diagnostics use GCCrelease/assert,3variants,random/duplicates/grid100000. Eighteen allocation runs count calls/requested bytes only while the full algorithm runs; eighteen uninstrumented fresh-process RSS runs exclude the separate staged calls. RSS remains whole-process high-water including input generation/storage/runtime/allocator. A small shell parent avoids inheriting Python's high-water RSS. Allocation-instrumented timings are excluded from runtime summaries. No cost attribution from RSS alone.

Reproduce after `source /tmp/blueberry-setup/env.sh`:

    python3 benchmark/closest-pair.py prepare --experiment NEW_NAME --production-sha256 <frozen_sha256>
    python3 benchmark/closest-pair.py measure --experiment NEW_NAME
    python3 benchmark/closest-pair.py diagnostics --experiment NEW_NAME
    python3 benchmark/closest-pair.py summarize --experiment NEW_NAME

Existing experiment directories/raw outputs are refused. Official29case preparation is independent at .verification/closest-pair/datasets/report.json; those official references accept any minimizer, so independent exact-tie brute tests remain necessary.
