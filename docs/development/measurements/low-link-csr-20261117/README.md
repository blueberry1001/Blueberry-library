# LowLink CSR measurement (2026-11-17)

The old implementation builds one adjacency vector per vertex; the candidate uses contiguous adjacency ranges and preserves each vertex's original edge order. The final public header also handles graphs with no edges directly. Public results and IDs remain unchanged. AtCoder Library has no LowLink equivalent. [maspypy’s graph representation](https://maspypy.github.io/library/graph/base.hpp.html) uses CSR; its [two-edge-component implementation](https://maspypy.github.io/library/graph/two_edge_component.hpp) has a narrower output contract. [ei1333’s LowLink](https://ei1333.github.io/library/graph/others/low-link.hpp) uses recursive traversal. These are structural references, not equivalent-condition benchmark results; a fastest-submission source could not be retrieved, so no claim of matching the fastest code is made.

Windows 11, Intel64 Family 6 Model 183 Stepping 1, Clang 18.0.3 targeting x86_64-pc-windows-msvc, `-std=gnu++20 -O2 -DNDEBUG`, affinity mask `0x1`. Construction plus all vertex component/articulation lookups and iteration of bridge/articulation output is timed; input generation, output and `groups()` allocation are excluded. Each cell is the median of five samples after one warm-up, with execution order alternated and identical results checked. Times are milliseconds for the **total repetitions**, not per construction. Peak working set is process-wide, including input and runtime, rather than an isolated library allocation count.

| Graph | n | m | Repetitions | Old ms | CSR only ms | Public header ms | Old / public peak MiB |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Empty | 0 | 0 | 200000 | 4.655 | 18.685 | 3.153 | 3.58 / 3.58 |
| Empty | 1 | 0 | 100000 | 41.191 | 39.719 | 8.195 | 3.58 / 3.58 |
| Single edge | 2 | 1 | 80000 | 47.223 | 41.708 | 42.139 | 3.59 / 3.59 |
| Path | 8 | 7 | 20000 | 32.491 | 22.108 | 23.143 | 3.59 / 3.59 |
| Empty | 100000 | 0 | 30 | 65.648 | 49.228 | 5.348 | 9.14 / 4.06 |
| Path | 100000 | 99999 | 15 | 384.854 | 100.589 | 94.536 | 15.73 / 12.17 |
| Star | 100000 | 99999 | 15 | 214.106 | 76.984 | 78.349 | 13.83 / 11.26 |
| Random multigraph | 100000 | 300000 | 8 | 725.761 | 215.677 | 225.426 | 20.58 / 15.19 |
| Doubled path and loops | 100000 | 224997 | 8 | 499.068 | 84.764 | 97.805 | 18.59 / 14.68 |
| Complete | 1000 | 499500 | 8 | 177.827 | 94.221 | 90.119 | 21.99 / 16.82 |

`results.json` preserves every warm-up and sample, compiler commands, SHA-256 hashes, child affinity and exit statuses. `initial-results.json`, `driver-v1.cpp.txt` and `run-v1.py.txt` preserve the initial exploratory run. The CSR-only variant exposes the tiny-empty-input regression that motivated the direct empty case. Changes in elapsed time between the CSR-only and public variants on graphs with edges are measurement variation; the public version has no different nonempty algorithm.

The final differential test compares the old code, CSR-only and the actual public header on empty graphs of size 0–9, 1,000 shuffles of a fixed graph with self-loops and parallel edges, and 9,000 seeded random multigraphs. It checks every public operation, exact bridge and articulation order, groups and component IDs. The public header also passed the deletion-based brute-force oracle and large path/empty cases in `tests/random/low-link.cpp`, with seeds 1, 17 and 731, in debug and release builds using Clang gnu++20/gnu++23 and MSVC C++20 (`validation.json`). The earlier `validation-initial.json` is retained: its large-case assertions did not run under `NDEBUG`. They were replaced with unconditional checks before the final validation; the measured public header was unchanged and timing was not rerun. GCC, full make targets and Library Checker execution require the repository's Linux environment and were not run here.

To reproduce in this Windows toolchain, place the file `prepare.py.txt` unchanged at `.build/low-link-csr-study/prepare.py` (it is recorded for hashing; do not execute it against a newer public header), adjust the hard-coded Clang, MSVC and Windows SDK paths in `run.py` and `validate.py` if necessary, then run `python docs/development/measurements/low-link-csr-20261117/validate.py` and `python docs/development/measurements/low-link-csr-20261117/run.py` from the repository root. The baseline and CSR-only namespaces are embedded as `.hpp.txt` snapshots; the final variant includes the actual checked-out public header. A different compiler, machine, workload or source revision is a separate measurement.
