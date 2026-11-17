# AggregateQueue transfer reservation

2026-11-17, WSL2, Intel Core i7-14650HX; GCC 13.3 / Clang 18.1.3, `-O2 -DNDEBUG -std=c++20`. Each measurement executes 1 warmup and 7 recorded runs, with baseline then candidate sequentially. `measurement.log` retains each sample, checksums, environment and source SHA-256. The first attempted header hash used shell brace expansion under `sh` and failed; explicit hashes were appended later. The driver has no I/O in the timed region and times construction, push, prod, pop, and destruction together; it does not isolate transfer or memory usage. Results do not compare end-to-end official input or equal-contract fastest submissions.

`benchmark/aggregate-queue-transfer.cpp` includes `queue.hpp`: place either `baseline.hpp.txt` or `candidate.hpp.txt` beside a copy of the driver as `queue.hpp` to reproduce. The candidate snapshot is the exact measured implementation; the final header additionally removes precondition assertions and comments per repository policy, with assertions already disabled in this measurement.

| Workload | GCC baseline / candidate ms | Clang baseline / candidate ms |
| --- | ---: | ---: |
| 70 fresh queues, 100,000 pushes then pops each | 156.270 / 122.080 | 190.457 / 148.195 |
| one reused queue, batches of 1 to 5,000 | 125.344 / 118.622 | 102.575 / 104.237 |
| one reused queue, one million batches of 8 | 74.121 / 74.211 | 58.252 / 58.185 |

The reserve allocates enough for the entire transfer while preserving geometric growth for repeated increasing batches. The queue still owns its elements and preserves noncommutative aggregation order. The large-batch result is useful evidence of reduced reallocation, not a universal speed claim. Existing Fastest analysis: `benchmark/results/operation-structures/report.md`, especially submission 403012, which assumes a capacity budget and uses different I/O. Compare a matching official workload and memory usage before drawing conclusions about its remaining gap.

Reservation can keep up to twice the previous output capacity when it grows; both stacks retain capacity until `clear` or destruction. Peak and resident memory were not measured, so the speed result does not quantify this memory tradeoff.
