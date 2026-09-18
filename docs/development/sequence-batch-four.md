# Sequence batch four: candidate research and measurements

2026-09-18. Three non-ACL operations were implemented independently; no external implementation was copied.

## Primary implementation research

| Operation | Candidates examined | Decision |
| --- | --- | --- |
| Range inversions | [Nyaan Mo + BIT](https://github.com/NyaanNyaan/library/blob/master/verify/verify-yosupo-ds/yosupo-static-range-inversions-query.test.cpp), [maspypy Mo + FenwickTree_01](https://github.com/maspypy/library/blob/main/test/2_library_checker/data_structure/static_range_inversions_mo_3.test.cpp) | Use Mo and the supported ACL Fenwick tree; retain ordinary coordinate compression for duplicate values. Specialized bitset Fenwick is a potential later optimization. |
| Range mode | [Nyaan Mo ordering](https://github.com/NyaanNyaan/library/blob/master/misc/mo.hpp), [suisen static range mode](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/range_query/static_range_mode_query.hpp) | suisen's block-pair preprocessing supports online queries in O(sqrt N) after O(N sqrt N) construction. The new batch API chooses Mo with O(1) intrusive frequency-bucket updates and O(N+Q) memory. |
| Longest common substring | [ei1333 suffix-array approach](https://github.com/ei1333/library/blob/master/string/longest-common-substring.hpp), [suisen suffix-automaton approach](https://github.com/suisen-cp/cp-library-cpp/blob/main/test/src/string/suffix_automaton/longest_common_substring.test.cpp), [ACL string](https://atcoder.github.io/ac-library/master/document_en/string.html) | Reuse ACL SA-IS and LCP. Encode bytes as 1..256 and reserve 0 for the separator, including empty and binary inputs. Avoid adding an entire automaton only for this operation. |

Primary sources above were read directly from GitHub raw content. The external implementations were researched for algorithm and API comparison, not benchmarked as binary-equivalent competitors. The local measurements below compare independently implemented baseline candidates under identical inputs and build settings. They do not establish superiority over the referenced libraries.

## Reproducible candidate benchmark

Source: [benchmark/sequence-batch-four.cpp](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/sequence-batch-four.cpp).
Raw output including OS, CPU and compiler: [sequence-batch-four-benchmark.log](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/sequence-batch-four-benchmark.log).

WSL Ubuntu, Intel Core i7-14650HX, GCC 13.3.0, `-std=gnu++20 -O2`, fixed seed 20260918, 3 serial runs.
Range inputs: N=Q=1000, values 0..99, uniformly sampled endpoints, including empty ranges.
String inputs: two length-4000 strings over four symbols. Identical checksums were required for both candidates.

| Candidate | Median ms | Minimum ms |
| --- | ---: | ---: |
| Inversions: Mo + ACL Fenwick | 0.348 | 0.321 |
| Inversions: direct pair enumeration | 24.402 | 23.783 |
| Mode: Mo frequency buckets | 0.292 | 0.290 |
| Mode: direct per-query dense counts | 0.152 | 0.150 |
| Common substring: ACL SA/LCP | 0.436 | 0.436 |
| Common substring: one-row dynamic programming | 21.975 | 21.678 |

The direct mode baseline wins on this small, bounded-alphabet workload. Mo was selected for its general comparable-value API and better asymptotic batch bound, not a claim of universal speed. For small inputs the direct approach remains appropriate. No missing or failed measurements were replaced with zero. Parallel work on the host may affect timings; comparisons only concern these measured candidates and inputs.

## Validation

`tests/random/sequence-batch-four.cpp` checks every interval of small random arrays against direct inversion and frequency oracles, and common substrings against direct matching. It includes empty arrays, empty ranges, equal values, negative values, NUL and high bytes.
GCC and Clang with both gnu++20 and gnu++23 passed. Standalone and duplicate header includes passed the same four compiler/standard combinations with `-DNDEBUG -Wall -Wextra -Werror` and ACL as a system include. The combined random executable also passed GCC release warnings.

The initial official run used an incorrect range-mode problem ID and failed its download; its failure excerpt is retained as `sequence-batch-four-official-initial.log`. The corrected ID is `static_range_mode_query`. The resume log is retained separately; the initial failure is not treated as success.

Corrected official run: longest common substring 28/28 cases AC (slowest 0.204199 s, max memory 28.284 MB); range inversions 13/13 AC (0.821410 s, 6.628 MB); range mode 11/11 AC (0.707972 s, 7.324 MB). These are single official verification runs, not median benchmark measurements. See the retained resume summary.
