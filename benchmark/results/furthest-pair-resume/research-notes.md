# Cached primary-source research

This resume used existing cached sources; no Fastest/AOJ/network retry was made.
The original `../furthest-pair/research.json` records source URLs/SHA256 and
`../furthest-pair/fastest-fetch.log` preserves the single HTTP tunnel403 failure.
Raw third-party sources remain outside tracked benchmark artifacts. None is
copied into the independent candidates.

- [KACTL HullDiameter.h](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/HullDiameter.h): cached file explicitly declares Boost Software License. Its input is an already strict CCW hull, passed by value; result coordinates and O(H) work exclude hull construction and original-index recovery. This is not an equivalent end-to-end API. The historical filename-level license is used rather than assuming a repository-wide license.
- [maspypy furthest_pair.hpp](https://github.com/maspypy/library/blob/main/geo/furthest_pair.hpp): cached source builds an indexed hull, duplicates hull IDs and coordinates for unwrapped support traversal, and stores squared distance in `long long`. No source-local license declaration was present; study-only treatment remains. Copying points and duplicating hull storage can avoid original-index recovery but adds copies/allocation/indirection.
- [Pinned official solution](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/furthest_pair/sol/correct.cpp): local checkout source hash matches the original research record. Indexed/duplicated-hull layout is similar; problem assumes N>=2, |coordinate|<=10^9 and total N<=500,000. Its narrower arithmetic fits that problem. Cached repository declares Apache2.0, but this experiment still uses it only for study.

Blueberry accepts empty/singleton input, normalized distinct original indices,
all-equal input and coordinate magnitudes through2^62-1. Direct timings of these
upstream routines without contract/arithmetic adapters would be misleading.
Consequently the benchmark compares independent coordinate, index and record
layouts with the same wide arithmetic, valid-input objective and copy accounting.

The caliper scan can syntactically reuse terminal `change(j)` for its plateau
check; compilers may already eliminate that repetition. This is deferred unless
measurements establish a relevant caliper bottleneck. Endpoint/plateau checks
are retained; the upstream one-pair update cannot simply replace this different
progression without a correctness argument. No fastest-source rank/time was
retrieved, so there is no claim to match or beat the leaderboard.
