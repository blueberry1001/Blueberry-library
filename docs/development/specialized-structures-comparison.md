# Specialized data structures: comparison method

Issue #25 adds dense predecessor search, range-action/point-query storage, a double-ended heap, and an owning meldable heap. Issue #26 separates two convex-function representations. The implementations use their specific operation sets; the measurements below are finite workload comparisons, not universal rankings.

## Design references

- [maspypy FastSet](https://maspypy.github.io/library/ds/fastset.hpp): a 64-ary bitmap hierarchy avoids comparison-tree allocation, at the cost of memory proportional to the integer universe. `std::set` accepts sparse arbitrary ordered keys and provides iterators; `FastSet` does not.
- [maspypy dual segment tree](https://maspypy.github.io/library/ds/segtree/dual_segtree.hpp) and [ACL lazy segment tree](https://github.com/atcoder/ac-library/blob/master/atcoder/lazysegtree.hpp): the dual tree stores only point values and pending actions. Chronological boundary propagation supports noncommutative actions; no aggregate monoid is required. ACL retains range aggregate queries that the dual tree cannot answer.
- [Sahni's double-ended priority queue discussion](https://www.cise.ufl.edu/~sahni/dsaaj/enrich/c13/double.htm): a contiguous min-max heap provides both extrema with O(log N) deletion and linear bulk construction. `std::multiset` additionally supports ordered iteration and arbitrary erasure. Vector growth makes heap insertion amortized.
- [Luzhiled skew heap](https://ei1333.github.io/library/structure/heap/skew-heap.hpp.html): skew heaps obtain amortized logarithmic meld without rank metadata. Our implementation owns its nodes, is move-only, and uses iterative meld/destruction. It omits lazy offsets and external node handles.
- [Luzhiled unit slope trick](https://ei1333.github.io/library/structure/others/slope-trick.hpp.html), [Nyaan weighted priority-queue slope trick](https://nyaannyaan.github.io/library/data-structure/slope-trick.hpp.html), and [maspypy finite-domain splay formulation](https://maspypy.github.io/library/convex/slope_trick/slope_super.hpp): the unit heap version is short and supports whole-real lower-bounded functions. The splay version stores arbitrary positive changes of slope, provides logarithmic point evaluation and segment convolution, and supports finite closed domains and affine terms. Their domains and supported operations differ; neither replaces the other. These sources informed algorithm/API comparisons; source code was not copied.

## Reproduction and interpretation

Run `python3 docs/development/measurements/specialized-structures/run.py --output .benchmark/specialized-new` from a Linux checkout with GCC and ACL available. The runner records compiler, flags, CPU, platform, repository revision, and SHA-256 hashes for the benchmark, six headers, and ACL files. Each candidate receives the same generated input, seed 20260919, and 100,000 operations. Five fresh processes per candidate alternate order. Input hashes and final checksums must agree within each pair. Generation is outside the timer; construction, operations, and destruction are included. Peak memory is `/proc/self/status` VmHWM for the whole process, including the common generated input.

The workloads are:

| Group | Candidates | Identical requested work |
| --- | --- | --- |
| Dense predecessor | FastSet / std::set | Uniform insert, erase, and successor operations in universe 4,096 or 65,536 |
| Range actions | DualSegmentTree / ACL lazy_segtree | Alternating range-affine updates and point reads; defined unsigned arithmetic modulo 2^64 |
| Both extrema | DoubleEndedPriorityQueue / std::multiset | Initial inserts followed by mixed pushes and min/max deletion, with duplicates |
| Meld | SkewHeap / std::priority_queue | 32 heaps, inserts, deletion, and one meld per 32 iterations; STL transfers by repeated pop/push |
| Convex functions | SlopeTrick / SplaySlopeTrick | Absolute-value addition, periodic horizontal shifts, and minimum query; unit weights only |

The slope comparison uses an initial splay domain [-1,000,000, 1,000,000], large enough to contain every minimizer throughout this particular input. It does not benchmark arbitrary weights, restriction, merge, or convolution; its result cannot be generalized to those operations. The meld comparison tests a capability absent from `std::priority_queue`; the transfer cost is part of the requested work. SkewHeap may lose to a binary heap when meld is not needed.

Official checker results validate their corresponding workloads separately. Priority-queue SlopeTrick currently has no official verifier; randomized brute comparisons cover its operations. The shortest-path SkewHeap verifier exercises batched meld indirectly, while direct randomized multiheap comparisons cover general meld sequences.

## Measurements

Measured on Intel Core i7-14650HX, WSL2 Linux, GCC 13.3.0, `-std=gnu++20 -O2 -DNDEBUG`, with the other coordinated CPU-heavy jobs stopped. All 90 samples completed, input hashes and paired checksums matched, and source hashes remained unchanged during the run. The raw [samples](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/specialized-structures/samples.jsonl), [environment and source hashes](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/specialized-structures/environment.json), [summary](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/specialized-structures/summary.json), and [compile log](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/specialized-structures/compile.log) preserve the measurement.

Times below are milliseconds; RSS is whole-process peak KiB. N means universe size for predecessor, array length for dual, initial insertion count for DEPQ, and coordinate range for slope. The meld workload always uses 32 heaps (N is unused).

| Group | N | Candidate | Median ms | Min ms | Median peak RSS KiB |
| --- | ---: | --- | ---: | ---: | ---: |
| depq | 4096 | DoubleEndedPriorityQueue | 1.396 | 1.332 | 4860 |
| depq | 4096 | std::multiset | 2.071 | 1.977 | 4968 |
| depq | 65536 | DoubleEndedPriorityQueue | 3.538 | 3.436 | 5276 |
| depq | 65536 | std::multiset | 9.697 | 9.595 | 7864 |
| dual | 4096 | DualSegmentTree | 15.588 | 15.309 | 4680 |
| dual | 4096 | ACL lazy_segtree | 9.783 | 9.647 | 5100 |
| dual | 65536 | DualSegmentTree | 23.507 | 23.302 | 6812 |
| dual | 65536 | ACL lazy_segtree | 14.749 | 14.485 | 8692 |
| fast | 4096 | FastSet | 0.930 | 0.919 | 4680 |
| fast | 4096 | std::set | 5.977 | 5.764 | 4688 |
| fast | 65536 | FastSet | 0.836 | 0.808 | 4652 |
| fast | 65536 | std::set | 8.987 | 8.504 | 5776 |
| meld | — | SkewHeap | 7.364 | 7.263 | 6320 |
| meld | — | std::priority_queue transfer | 114.123 | 112.474 | 9492 |
| slope | 4096 | SlopeTrick | 6.109 | 6.004 | 6588 |
| slope | 4096 | SplaySlopeTrick | 24.321 | 24.145 | 6032 |
| slope | 65536 | SlopeTrick | 7.677 | 7.519 | 6572 |
| slope | 65536 | SplaySlopeTrick | 37.300 | 36.942 | 8920 |

On these inputs, the bitmap set, min-max heap, and skew meld workload beat their STL counterparts. Unit-weight heap SlopeTrick is faster than the richer splay model. Memory measurements include allocator and executable overhead; at small sizes those effects can dominate differences.

**The dual tree is slower here:** at N=65,536 it takes 23.507 ms versus ACL's 14.749 ms, despite lower process peak memory (6,812 versus 8,692 KiB). Its pending flags, conditional propagation and two boundary paths add work, while ACL's simpler unconditional arithmetic benefits this cheap affine action. These timings establish the slowdown but do not isolate each cause through a separate profile. The reason to choose this API is its smaller state and lack of aggregate-monoid requirements, not an unqualified speed claim. ACL remains a strong default when its aggregate API fits.

The measurements cover the listed operation distributions only. Heavy arbitrary weights, point evaluation, finite-domain restriction, and min-plus convolution can favor the splay representation for capabilities or asymptotics that are absent from the unit heap workload. No missing or failed measurement has been replaced by zero.



測定後に runner へ main guard と --output の上書き防止を追加しました。import 時には測定を開始せず、既存の測定ファイルがある出力先は拒否します。既存の環境記録は C++ benchmark・header・ACL のハッシュを対象としており runner 自体は含みません。測定処理と保存済み結果は変更していません。
