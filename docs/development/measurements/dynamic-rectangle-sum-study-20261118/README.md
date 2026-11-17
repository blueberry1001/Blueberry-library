# Online 2D Fenwick rectangle-sum study (2026-11-18)

`DynamicFenwickTree2D::sum` calls four 2D prefixes. For each x endpoint the two y bounds therefore repeat the same outer Fenwick traversal and hash lookups. This experiment compares four candidates while preserving unknown-coordinate online updates. **No candidate was adopted and the public header was not changed.**

The original public header is retained as `baseline-original.hpp.txt`. It matches the file at revision `179d99a` (Git blob `a854af1c6a4d6fb77098b122bbbfc67873e04596`). The generated snapshots use separate namespaces so the candidates can be compared in a single executable. `prepare.py.txt`, `study.cpp.txt`, and `run.py.txt` are the exact sources used; `build-winclang.json` records their hashes and the hashes of all generated headers. Each `*.hpp.txt` snapshot is byte-identical to the named generated `.hpp` in that build. The binary is intentionally omitted.

| Mode | Candidate | Arithmetic order | Measured |
| --- | --- | --- | --- |
| 0 | Old public implementation | Original | Yes |
| 1 | Share hash lookups and subtract the two y prefixes for each x cell | Changes | Correctness only |
| 2 | Mode 1 plus cancellation of common outer Fenwick cells | Changes | Correctness only |
| 3 | Mode 2 plus one split traversal of each y AVL tree | Changes | Correctness only |
| 4 | Share hash lookups but retain four separate prefix accumulators and the original final expression | Preserved for ordinary value types | Yes |

Modes 1–3 can overflow a signed intermediate even on inputs where the old implementation is valid. For example, in a `3 × 3` tree, insert `(2,1,LLONG_MAX)`, `(2,0,-1)`, `(2,2,1)`, `(0,1,-1)` in this order; `sum(0,1,3,3)` is `LLONG_MAX`, and the original prefix arithmetic stays in range. Modes 1 and 2 subtract the two y prefixes inside the x=2 cell and compute `LLONG_MAX+1`; mode 3's `range_node` instead overflows when it adds the y=1 value `LLONG_MAX` and the y=2 value `1`. These candidates therefore do not preserve the old arithmetic preconditions. Mode 4 retains the per-prefix accumulation and final subtraction/addition order, at the expense of separate y traversals; operations on distinct accumulators become interleaved. User-defined arithmetic with observable external side effects is not assessed.

## Validation

All five generated implementations passed fixed-seed tests 1, 17 and 731 with signed integers, ACL modint, and a two-component commutative additive group that deliberately has no `-=` operator. Tests compare with a naive grid and cover random updates, duplicate points, negative values, empty rectangles and dimensions at `LLONG_MAX`. Mode 4 additionally matches the baseline bit-for-bit for seeded floating-point operations and passes the integer boundary example above. `build-winclang.json` contains the Windows Clang 18 commands and outputs. The same three seeds also passed WSL GCC 13.3 with `-O1 -fsanitize=undefined -fno-sanitize-recover=undefined`; `ubsan-validation.json` and `ubsan.py.txt` preserve the commands, outputs and source hashes. These checks do not replace official-problem verification.

The shell sandbox rejected an initial `wsl.exe -d Ubuntu -- /usr/bin/true` availability probe with `Wsl/E_ACCESSDENIED`. The raw stdout bytes, decoded output and exit code are retained in `wsl-probe.json`. This was a failed environment probe, not a failed or zero-time WSL benchmark. WSL became available for the sanitizer test above; no direct WSL/GCC candidate timing was obtained.

## Performance

Windows 11 on Intel64 Family 6 Model 183 Stepping 1, Clang 18.0.3 targeting x86_64-pc-windows-msvc, `-std=gnu++20 -O2 -pipe -Wall -Wextra -Wshadow -D_GLIBCXX_ASSERTIONS`, CPU affinity `0x1`. The MSVC standard library does not use the `_GLIBCXX_ASSERTIONS` macro; `NDEBUG` was not set. Each process generates the same seed-20261118 workload before timing. A separate construction timer includes all point updates; the query timer includes rectangle lookups and checksum accumulation. Input generation, I/O and destruction are excluded. Results are query milliseconds, one warm-up followed by five samples, alternating the two execution orders. The full inputs are reproducible from the archived driver and seed; no independent input-file hash was recorded. Full stdout, timestamps, per-sample construction time, query time, exit status, checksum and process peak memory are retained in the three result JSON files.

| Workload | N | Q | Old query median ms | Mode 4 query median ms |
| --- | ---: | ---: | ---: | ---: |
| Broad random rectangles | 20000 | 30000 | 276.859 | 227.110 |
| Concentrated x, narrow rectangles | 20000 | 30000 | 152.620 | 147.608 |
| Full domain | 20000 | 30000 | 12.4251 | 11.8515 |
| Duplicate points in a small domain | 20000 | 30000 | 4.7188 | 5.5770 |
| Empty rectangles | 20000 | 30000 | 0.0675 | 0.0751 |
| Tiny rectangles | 20000 | 30000 | 128.601 | 130.299 |
| No points | 0 | 30000 | 3.0506 | 1.9339 |
| Updates only | 20000 | 0 | 0 | 0 |
| Broad random, official maximum N and Q | 200000 | 200000 | 5377.970 | 5287.480 |

In the final row the old query samples were 5527.04, 5590.53, 5227.01, 5377.97 and 5004.35 ms; mode 4 samples were 5517.88, 5287.48, 5335.76, 5040.71 and 5015.99 ms. The broad spread makes the approximately 1.7% median improvement inconclusive. The duplicate-point query samples were 4.6638–4.9626 ms for the old implementation and 5.2879–6.3584 ms for mode 4, a clear regression in this workload. The updates-only row has no queries, so a zero query duration is the actual timer reading and is not a missing measurement. Process-wide peak memory in the 200000-point workload was approximately 269 MB for both implementations, including generated inputs and the runtime.

The previously reported WSL GCC 13.3 official-case result, worst-case three-run median 4.867 s against a 5 s limit, used a different operating system, compiler, driver and dataset (`fc30ff14ff46358b69d772613533ce5933753b0a38fda80d0ab2823948127eb4`). It cannot be numerically compared with the Windows query-only results above. The user-visible judge also runs on different hardware, so these local figures do not prove a submitted AC or TLE.

The static [Rectangle Sum submission 269578](https://judge.yosupo.jp/submission/269578) uses a sweep with coordinate compression and a Fenwick tree after reading all points and queries. It does not offer unknown-coordinate online updates. The previously archived [submission 206958](https://judge.yosupo.jp/submission/206958) addresses the different Point Add Rectangle Sum problem and also assumes pre-registration. Neither historical submission is claimed to be the present fastest, and neither is timed under the conditions above. An earlier local dynamic-Fenwick benchmark, documented in `benchmark/dynamic-fenwick-tree.md`, also showed an advantage for the offline structure when coordinates were available in advance; it does not establish an equivalent online-contract speedup.

## Reproduction

Use a checkout whose public `blueberry/data-structure/dynamic-fenwick-tree-2d.hpp` matches `baseline-original.hpp.txt`; `prepare.py` verifies the Git blob before generating any candidate header. Copy `prepare.py.txt`, `run.py.txt`, and `study.cpp.txt` to `.build/dynamic-rectangle-sum-study/` without the `.txt` suffix, then run from the checkout root:

```sh
python3 .build/dynamic-rectangle-sum-study/run.py --build --tag local
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-short --build-tag local --only random-wide,concentrated-narrow,full
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-edge --build-tag local --only duplicate,empty-range,tiny-range,no-points,add-only
python3 .build/dynamic-rectangle-sum-study/run.py --tag local-large --build-tag local --only official-size-random
```

The Windows compiler, MSVC and SDK paths are configured in `run.py`; adjust them for another machine. On Linux it uses `g++` unless `--compiler` is provided. The default timed modes are 0 and 4; `--modes 0,1,2,3,4` also times the arithmetic-reordering candidates. Every run needs a new tag and preserves its build/error log and raw samples. Different compilers, source revisions, machines and datasets must be treated as separate measurements.
