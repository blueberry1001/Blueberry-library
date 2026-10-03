# Dominator tree implementation and performance

Measured on 2026-10-03 on BlueberryPC, Intel Core i7-14650HX, Ubuntu under WSL2.
The full environment and raw seven-run measurements are in
[dominator-tree-results.log](dominator-tree-results.log).
The measured header SHA256 is
`4f1c0081d915d708c375b5507215b7d3945f84ffd425ee4fb0baa3e2e68a9582`.

## Algorithm and reference research

The implementation uses the simple Lengauer–Tarjan algorithm, with iterative DFS
and iterative path compression. Its worst-case time is O((N+M) log(N+1)) for
nonempty graphs and its auxiliary memory is O(N+M). Balanced linking would be
needed to claim the inverse-Ackermann bound. The public API accepts arbitrary
valid directed adjacency lists, preserves vertex IDs, returns the root itself at
the root and -1 at unreachable vertices, and handles loops and parallel edges.

Sources inspected for algorithm, API, and storage choices:

- [Lengauer and Tarjan, 1979](https://doi.org/10.1145/357062.357071), original algorithm.
- [ei1333](https://ei1333.github.io/library/graph/others/dominator-tree.hpp.html),
  Unlicense: simple LT, recursive DFS/compression, nested predecessor and bucket vectors.
- [Library Checker official solution](https://github.com/yosupo06/library-checker-problems/blob/master/graph/dominatortree/sol/correct.cpp),
  Apache-2.0: simple LT, recursive DFS/compression, caller-supplied reverse adjacency.
- [LLVM's evaluation discussion](https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/GenericDomTreeConstruction.h):
  iterative evaluation and simple compression's logarithmic bound. LLVM's overall
  Semi-NCA algorithm has a different worst-case bound; its final phase is not used.

The public Fastest API was read on the same date. Submission
[341145](https://judge.yosupo.jp/submission/341145) reported 29 ms and 15,458,304 bytes;
[87379](https://judge.yosupo.jp/submission/87379) reported 30 ms and 16,470,016 bytes.
Both use recursive simple LT. The first uses static CSR, flat linked buckets,
unsigned indices, and AVX2/mmap I/O. The second uses fixed-size adjacency arrays,
`basic_string<int>` buckets, and unlocked buffered I/O. Their source licenses
were not established, so their code was not copied. Public judge times are not
compared numerically with local timings.

## Storage comparison

Reproduce from the repository root:

```sh
python3 benchmark/dominator-tree-benchmark.py > benchmark/dominator-tree-results.log 2>&1
```

The runner generates a temporary variant of our own implementation with nested
predecessor and bucket vectors. DFS, compression, API, input generation, and the
dominator algorithm remain the same. The selected implementation stores incoming
edges in CSR and stores each bucket as an intrusive list in two integer arrays.
No fixed problem limit, ISA requirement, or restricted query order is introduced.

Each configuration uses C++20 and `-O2`, GCC 13.3 or Clang 18.1, with assertions
enabled and with `-DNDEBUG`. Each case has one warmup and seven measured runs.
The benchmark first compares the two complete result vectors. Generation uses
seed 20261003. Inputs are immutable and identical for both layouts.

`input_build_ms` measures caller-owned adjacency generation. `solve_ms` includes
reverse adjacency construction, LT, result allocation and temporary destruction;
formatting, input generation and checksum computation are outside that timer.
Peak RSS is for the whole process across all five cases, including its input and
output. No claim about parsing or formatting speed is made.

| Case | N | M | Shape |
| --- | ---: | ---: | --- |
| chain | 200,000 | 199,999 | A directed path from 0 |
| backedge | 200,000 | 200,000 | Path plus last vertex to 1, exercising deep compression |
| random | 100,000 | 200,000 | Random rooted tree plus random directed edges |
| dense | 20,000 | 200,000 | Random rooted tree plus random directed edges |
| unreachable | 200,000 | 200,000 | 2,000 reachable path vertices; remaining edges start outside |

Release measurements below are median / minimum milliseconds:

| Case | GCC vectors | GCC flat | Clang vectors | Clang flat |
| --- | ---: | ---: | ---: | ---: |
| chain | 9.191 / 8.843 | 5.920 / 4.635 | 9.977 / 9.767 | 4.847 / 4.607 |
| backedge | 10.103 / 9.943 | 6.326 / 5.046 | 11.301 / 11.052 | 5.564 / 5.280 |
| random | 23.431 / 18.552 | 11.313 / 10.797 | 19.185 / 18.867 | 11.015 / 10.722 |
| dense | 7.675 / 7.350 | 4.069 / 3.947 | 7.370 / 7.167 | 4.535 / 4.434 |
| unreachable | 1.165 / 1.114 | 0.873 / 0.857 | 1.360 / 1.329 | 0.963 / 0.944 |

| Configuration | Vectors peak RSS, KiB | Flat peak RSS, KiB |
| --- | ---: | ---: |
| GCC assertions | 43,412 | 25,492 |
| GCC release | 43,208 | 25,348 |
| Clang assertions | 43,288 | 25,372 |
| Clang release | 43,240 | 25,376 |

The flat layout has a lower median on all measured configurations, including
assertion-enabled builds. It avoids many small allocations and reduces process
peak RSS by about 17 MiB. This is a substantial improvement over the nested-vector
candidate, so flat storage is selected. Some runs vary noticeably, especially GCC
release, and these results do not establish a universal speed ranking. Recursive
traversal, fixed-size arrays, and specialized AVX2 I/O are not adopted because they
would narrow the safety or portability of the public interface.

## Official reference comparison

The cached official solution was measured separately using the same generated
inputs, timer boundaries, warmup, repetitions, and GCC 13.3 C++20 `-O2 -DNDEBUG`.
The temporary adapter replaces the reference's one-integer edge struct with `int`,
constructs its reverse adjacency inside the timer, and normalizes the root result.
The result vectors match on all five cases. Both processes receive a 256 MiB
stack limit so the recursive reference can complete the deep cases.

The reference source SHA256 is
`03a7726b3af101b24ada458dc1e98e800b718a1cc5e1a85eb0f0d3b6b10bef19`.
The source is not stored in this repository; the adapter reads a user-supplied
cached official solution and generates a temporary header. Reproduce with:

```sh
python3 benchmark/dominator-tree-reference.py \
  /path/to/library-checker-problems/graph/dominatortree/sol/correct.cpp \
  > benchmark/dominator-tree-reference.log 2>&1
```

Raw measurements are in [dominator-tree-reference.log](dominator-tree-reference.log).
These measurements form a separate run; compare values within this table.

| Case | Official median / minimum, ms | Blueberry median / minimum, ms |
| --- | ---: | ---: |
| chain | 10.954 / 10.869 | 5.622 / 4.971 |
| backedge | 12.131 / 11.782 | 5.826 / 5.649 |
| random | 26.006 / 25.338 | 12.871 / 12.282 |
| dense | 8.292 / 8.155 | 4.644 / 4.585 |
| unreachable | 10.714 / 10.393 | 0.962 / 0.950 |

Process peak RSS was 48,712 KiB for the reference and 25,296 KiB for Blueberry.
The unreachable case benefits from sizing the LT arrays to reached vertices and
storing reverse edges only from reachable vertices. Both implementations inspect
the original input and return one result for every original vertex. The measured
cases show no unresolved slowdown against this comparable general reference.

## Correctness coverage

The focused GCC 13.3 C++20 randomized runner passed 20 seeds (1 through 20).
Each seed includes every loop-free directed graph on up to four vertices with
every root, 500 random multigraphs with every root, shuffled adjacency, explicit
unreachable incoming edges, and 200,000-vertex path/backedge cases.
The small oracle removes each potential dominator and checks reachability.
The first local run exposed an incorrect expected result in the deep shortcut
test; that assertion was corrected, and the library implementation was unchanged.

The integration run of the Library Checker driver passed all 13 official cases
in three repeats. Compiler/header/example checks and sanitizer results are
recorded with the complete batch's integration evidence.
