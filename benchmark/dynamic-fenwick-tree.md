# Online sparse Fenwick migration

The replacement targets unknown online update positions in very large bounded
integer domains. It does not replace the compressed offline 2D implementation.

## Design and sources

- Legacy revision: `03ee56684d0448db62b72a5b542861cc6fc35c28`,
  `blueberry/DynamicFenwickTree2D.hpp`. Its outer structure eagerly allocates
  `n+1` inner trees, owns them through raw pointers without a destructor, and has
  no standalone includes. Copying shares inner trees.
- [Nyaan dynamic 2D Fenwick](https://nyaannyaan.github.io/library/data-structure-2d/dynamic-binary-indexed-tree-2d.hpp.html)
  was read as an algorithm reference. A dense outer array suits compressed x
  coordinates; nesting sparse Fenwick trees adds a log(m) storage factor.
- The new online structure stores only reached outer Fenwick cells in an
  unordered map, each pointing into one vector of augmented AVL nodes. Every
  distinct point needs O(log n) nodes; inner height is deterministically
  logarithmic. Hash lookup bounds still depend on collision distribution.
- [Fastest C++ submission 206958](https://judge.yosupo.jp/submission/206958)
  pre-registers all future update coordinates and uses wavelet-style point
  mapping plus mmap I/O. Its 151 ms public snapshot is not a comparable local
  timing and its offline contract cannot replace unknown online coordinates.
  Raw fetched metadata and sources remain in `.build/legacy-fenwick/fastest`;
  no submitted implementation was copied into the new header.

## Reproduction

Run on Linux/WSL with `/usr/bin/time`, Python 3 and GCC:

```sh
python3 benchmark/run_dynamic_fenwick.py
```

The runner extracts the frozen legacy header from Git into `.build`, compiles
all three candidates with the same compiler and flags, and executes each in a
separate process. Five measured runs follow one warmup; the order alternates.
Seed 1729 produces identical 60,000 update/query pairs for each candidate.

- `bounded`: uniform updates in a 20,000 × 20,000 domain.
- `repeated`: updates restricted to a 100 × 100 subset of that domain.
- `huge`: uniform 10^9 × 10^9 coordinates; the dense-in-x legacy constructor is
  omitted, not reported as a failed/zero-time measurement.

Timed work includes construction, offline coordinate collection/preparation,
updates, rectangle queries and destruction. Input generation is excluded.
The harness explicitly deletes legacy raw pointers to include equivalent
cleanup. Peak RSS includes the process and input vector. The offline candidate
gets all future update positions, a capability difference to retain when
interpreting results. Checksums must agree. Output contains individual runs,
median/min times, RSS, compiler/CPU/OS/flags and source hashes.

## Local measurement (2026-09-17)

[Raw runs and environment](results/dynamic-fenwick.json): Intel Core i7-14650HX,
WSL2 Ubuntu, GCC 13.3.0, `-std=gnu++20 -O2 -DNDEBUG`. Other migration compile,
verify and benchmark jobs were paused during this measurement. All candidate
checksums agreed. Values below are medians of five measured runs.

| Shape | Legacy ms / peak KiB | Online AVL ms / peak KiB | Offline ms / peak KiB |
| --- | ---: | ---: | ---: |
| bounded | 328.24 / 44,220 | 356.47 / 26,908 | 185.46 / 12,508 |
| repeated | 69.49 / 11,524 | 54.22 / 5,760 | 20.15 / 5,576 |
| huge | not measured: dense outer allocation | 973.45 / 61,368 | 242.86 / 16,996 |

The online replacement reduced measured peak RSS versus legacy on both shared
workloads. It was about 9% slower on bounded uniform updates and about 22% faster
on repeated updates. This is not a universal speed improvement: its reason for
adoption is online unknown-coordinate support, sparse initialization, bounded
inner-tree height and safe value ownership. Pre-registration remains the faster
and smaller choice on all three workloads when that contract is available.

## Correctness evidence before integration

- Official checker runs, one repetition, isolated in
  `.build/legacy-fenwick/official`: Point Add Range Sum 21 cases, Point Add
  Rectangle Sum 18 cases, Rectangle Sum 14 cases, all passed. These checks used
  the repository runner's 60-second per-case timeout; no online submission or
  cross-machine performance comparison is claimed.
- GCC 13 and Clang 18, gnu++20/gnu++23: randomized tests with seeds 1, 2, 3, 17
  passed; Clang ASan/UBSan seed 5 passed. Added self-assignment and all-zero
  search boundaries passed GCC seed 19.
- Both docs' minimal examples, operation markup, standalone and repeated
  release includes passed GCC and Clang.
- Tests cover independent brute-force enumeration, arbitrary signed weights,
  modint sums, empty dimensions/ranges, duplicate and sorted insertions,
  `LLONG_MAX` dimensions, weighted prefix search and independent copy/move.
