# LCA measurement results

Baseline: `d6af3e9`. N=200000, Q=500000, seed=20261017, 1 warmup + 5 recorded runs. These are medians from `full/summary.json`; raw attempts including warmups are in `full/samples.jsonl`. Build and query durations exclude tree/query generation and I/O. Peak RSS includes shared input arrays.

Final selection: `bit-scan.hpp` for Doubling/kth and the identical RMQ portion of `candidate.hpp`. `final.hpp` combines these changes. All checksum comparisons passed.

## gcc-release

| API | Shape | Build ms before -> after | Query ms before -> after | Total ms before -> after | Peak RSS KiB before -> after |
| --- | --- | ---: | ---: | ---: | ---: |
| doubling | path | 6.158 -> 5.558 | 43.505 -> 28.472 | 49.663 -> 33.865 | 35984 -> 35220 |
| doubling | star | 6.075 -> 1.351 | 48.736 -> 1.499 | 54.599 -> 2.850 | 31596 -> 17992 |
| doubling | balanced | 5.561 -> 1.808 | 48.846 -> 17.270 | 54.407 -> 19.080 | 32804 -> 22020 |
| doubling | broom | 6.016 -> 5.083 | 45.477 -> 30.134 | 51.479 -> 35.123 | 33664 -> 32308 |
| doubling | random | 8.528 -> 4.540 | 50.737 -> 18.718 | 60.321 -> 23.767 | 33072 -> 21972 |
| doubling | permuted | 9.860 -> 4.920 | 64.085 -> 19.471 | 73.446 -> 24.355 | 33068 -> 21972 |
| rmq | path | 13.749 -> 4.534 | 4.944 -> 5.464 | 18.672 -> 9.864 | 84240 -> 36408 |
| rmq | star | 14.095 -> 4.266 | 6.181 -> 4.586 | 21.075 -> 8.844 | 78004 -> 31872 |
| rmq | balanced | 13.641 -> 3.815 | 5.789 -> 4.562 | 19.433 -> 8.333 | 80400 -> 33248 |
| rmq | broom | 22.759 -> 4.078 | 6.231 -> 4.767 | 29.075 -> 8.845 | 81140 -> 34312 |
| rmq | random | 23.485 -> 7.147 | 4.936 -> 4.666 | 28.421 -> 12.037 | 80368 -> 33508 |
| rmq | permuted | 23.539 -> 7.803 | 5.588 -> 4.864 | 29.140 -> 12.667 | 80348 -> 33548 |
| kth | path | 6.248 -> 5.644 | 34.205 -> 28.877 | 40.461 -> 34.230 | 35968 -> 35208 |
| kth | star | 6.007 -> 1.458 | 2.154 -> 2.020 | 8.058 -> 3.422 | 31592 -> 17988 |
| kth | balanced | 5.478 -> 1.836 | 5.826 -> 3.289 | 11.397 -> 5.093 | 32748 -> 21992 |
| kth | broom | 6.193 -> 5.341 | 24.941 -> 14.000 | 31.229 -> 19.340 | 33656 -> 32308 |
| kth | random | 8.509 -> 4.835 | 5.398 -> 2.911 | 13.908 -> 7.830 | 33096 -> 21972 |
| kth | permuted | 9.728 -> 5.682 | 5.355 -> 2.947 | 14.985 -> 8.560 | 33100 -> 21980 |

## gcc-assert

| API | Shape | Build ms before -> after | Query ms before -> after | Total ms before -> after | Peak RSS KiB before -> after |
| --- | --- | ---: | ---: | ---: | ---: |
| doubling | path | 6.832 -> 5.479 | 42.285 -> 34.345 | 48.844 -> 39.936 | 35960 -> 35124 |
| doubling | star | 5.950 -> 1.469 | 50.040 -> 1.776 | 55.839 -> 3.245 | 31580 -> 17952 |
| doubling | balanced | 5.792 -> 2.037 | 51.204 -> 17.829 | 56.995 -> 19.741 | 32796 -> 22012 |
| doubling | broom | 6.298 -> 5.336 | 47.315 -> 33.596 | 53.517 -> 38.720 | 33660 -> 32352 |
| doubling | random | 9.608 -> 5.009 | 60.924 -> 18.936 | 70.693 -> 24.090 | 33100 -> 21924 |
| doubling | permuted | 9.969 -> 5.157 | 63.234 -> 19.556 | 73.203 -> 24.712 | 33096 -> 22004 |
| rmq | path | 13.802 -> 4.427 | 5.102 -> 5.925 | 19.160 -> 10.003 | 84328 -> 36452 |
| rmq | star | 13.507 -> 4.370 | 6.396 -> 4.670 | 19.974 -> 8.962 | 78152 -> 31920 |
| rmq | balanced | 14.776 -> 4.135 | 6.555 -> 4.581 | 22.304 -> 8.600 | 80336 -> 33400 |
| rmq | broom | 13.573 -> 4.415 | 6.121 -> 5.170 | 19.768 -> 9.632 | 81244 -> 34364 |
| rmq | random | 25.996 -> 7.243 | 6.083 -> 5.113 | 31.283 -> 12.318 | 80396 -> 33548 |
| rmq | permuted | 24.237 -> 7.474 | 6.247 -> 5.493 | 31.055 -> 12.880 | 80352 -> 33548 |
| kth | path | 6.338 -> 5.693 | 36.047 -> 33.958 | 42.702 -> 39.445 | 35960 -> 35212 |
| kth | star | 6.278 -> 1.474 | 2.436 -> 2.269 | 8.714 -> 3.743 | 31600 -> 17980 |
| kth | balanced | 5.810 -> 1.966 | 6.891 -> 3.578 | 12.510 -> 5.502 | 32752 -> 22004 |
| kth | broom | 6.275 -> 5.318 | 25.649 -> 15.476 | 31.964 -> 20.411 | 33616 -> 32308 |
| kth | random | 9.528 -> 5.111 | 5.766 -> 3.321 | 15.355 -> 8.496 | 33124 -> 21972 |
| kth | permuted | 9.067 -> 5.249 | 5.840 -> 3.308 | 15.015 -> 8.661 | 33104 -> 22016 |

## clang-release

| API | Shape | Build ms before -> after | Query ms before -> after | Total ms before -> after | Peak RSS KiB before -> after |
| --- | --- | ---: | ---: | ---: | ---: |
| doubling | path | 5.839 -> 5.792 | 42.606 -> 33.746 | 48.445 -> 40.303 | 36000 -> 35180 |
| doubling | star | 6.165 -> 1.334 | 149.538 -> 2.324 | 155.081 -> 3.658 | 31600 -> 17996 |
| doubling | balanced | 5.507 -> 1.999 | 152.642 -> 10.280 | 158.914 -> 12.375 | 32832 -> 21936 |
| doubling | broom | 5.980 -> 5.644 | 86.539 -> 50.667 | 92.886 -> 55.696 | 33672 -> 32276 |
| doubling | random | 9.498 -> 5.049 | 187.188 -> 10.903 | 197.152 -> 15.734 | 33060 -> 22012 |
| doubling | permuted | 9.322 -> 5.793 | 160.338 -> 12.329 | 169.660 -> 18.200 | 33096 -> 21940 |
| rmq | path | 13.123 -> 4.375 | 4.821 -> 5.316 | 18.492 -> 9.654 | 84284 -> 36420 |
| rmq | star | 11.744 -> 4.432 | 4.650 -> 4.132 | 16.591 -> 8.564 | 78004 -> 31828 |
| rmq | balanced | 12.156 -> 4.192 | 5.500 -> 3.547 | 17.991 -> 7.739 | 80380 -> 33280 |
| rmq | broom | 12.378 -> 4.702 | 4.925 -> 4.711 | 17.208 -> 9.414 | 81244 -> 34312 |
| rmq | random | 20.451 -> 8.641 | 6.458 -> 4.590 | 26.820 -> 13.029 | 80364 -> 33512 |
| rmq | permuted | 19.577 -> 8.169 | 5.726 -> 4.076 | 25.412 -> 12.246 | 80264 -> 33512 |
| kth | path | 6.523 -> 5.651 | 42.962 -> 24.972 | 49.977 -> 30.741 | 36012 -> 35180 |
| kth | star | 5.734 -> 1.339 | 1.932 -> 1.830 | 7.743 -> 3.135 | 31616 -> 17960 |
| kth | balanced | 5.539 -> 2.079 | 6.040 -> 3.136 | 11.521 -> 5.234 | 32740 -> 22008 |
| kth | broom | 5.947 -> 5.675 | 26.924 -> 15.169 | 32.408 -> 20.418 | 33668 -> 32356 |
| kth | random | 9.710 -> 5.042 | 5.315 -> 2.826 | 15.025 -> 7.918 | 33060 -> 22040 |
| kth | permuted | 9.209 -> 5.594 | 5.167 -> 2.940 | 14.341 -> 8.429 | 33100 -> 22000 |

## Selection and limits

The packed 64-bit parent rank/vertex candidate is preserved in `packed.hpp`. `packed-retry/` contains a separate 5-run same-input comparison against baseline and int. Packing can remove the dependent vertex lookup and improve some query loops, but increases peak RSS by about 13 MiB relative to int and generally increases construction plus query time; it was rejected. The first attempt in `packed/run.log` exited 127 because temporary baseline binaries were no longer present. No failed/missing result receives a timing. Binaries were rebuilt under `.benchmark/frequent-tree-binaries`.

The int RMQ path query loop regressed in the original full run: GCC release 4.94 -> 5.46 ms and Clang release 4.82 -> 5.32 ms. The additional run did not reproduce the GCC regression (5.17 -> 4.89 ms), while Clang retained it (4.97 -> 5.34 ms). Construction and memory savings dominate at this Q/N ratio. Very query-heavy path inputs can prefer the old query layout; no universal speedup is claimed.

`small/` independently measures N=4096, Q=100000 with GCC/Clang release, the same 6 shapes, and 5 repeats after warmup. Every median construction + query time improved. GCC total reductions: Doubling 20.4-82.3%, RMQ 27.8-32.5%, kth 13.9-70.6%. Clang total reductions: Doubling 48.4-87.9%, RMQ 28.8-62.7%, kth 17.5-70.0%. Sub-millisecond measurements are noisy; inspect raw samples before interpreting minor differences.

Focused correctness: `validation.json`/`validation.log` contain 106 successful compile/run commands. GCC/Clang x gnu++20/23 each compile and execute the new boundary/random test with seeds 1-20 plus existing `tests/tree_random.cpp`, compile both official LCA verify drivers, and compile a duplicate standalone include with `-DNDEBUG`. GCC AddressSanitizer+UndefinedBehaviorSanitizer passed seed 20261017. Repository-wide checks and official testcase execution are performed by the integration owner.

The original full-run script is preserved as `full/run-script.py`; its SHA256 matches the run.py hash recorded in that environment. Current `run.py` adds kind/shape filters for later comparisons. Snapshot headers keep SparseTable at the baseline revision, so the LCA effects are isolated.
