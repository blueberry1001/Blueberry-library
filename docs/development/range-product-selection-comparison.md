# DST, Sqrt Tree, and Segment Tree: choosing by update/query counts

This comparison addresses range-product selection directly. Static data does not exclude dynamic structures: ACL Segment Tree and Dynamic Sqrt Tree answer unchanged-array queries correctly, and lower preprocessing cost may be useful when the query count is small.

| Structure | Construction | Range product | Point replacement | Storage |
| --- | --- | --- | --- | --- |
| Disjoint Sparse Table | O(N log N) | O(1), nonempty range | Not supported | O(N log N) |
| Static Sqrt Tree | O(N log log N) | O(1) | Not supported | O(N log log N) |
| Dynamic Sqrt Tree | O(N log log N) | O(1) | O(sqrt N) | O(N log log N) |
| ACL Segment Tree | O(N) | O(log N) | O(log N) | O(N) |

All four handle associative, noncommutative products; this benchmark uses inexpensive signed 64-bit sums. DST does not require an identity but excludes empty intervals, so all measured queries are nonempty. DST and Static Sqrt Tree are **excluded** from update workloads. Rebuilding after each replacement is neither their API nor a reasonable default competitor here.

## Reproduce

Run `python3 docs/development/measurements/range-product-selection/run.py --output .benchmark/range-product-new` on Linux with GCC and `.deps/ac-library` installed. This is independent of earlier measurements and does not overwrite them.

- N = 4,096 and 65,536; 総操作数 M = 100 and 100,000.
- Updates = 0%, 1%, and 50%, exactly periodic in blocks of 100 operations; the rest are range queries. 本レポートでは総操作数を M、更新回数を U、区間クエリ回数を Q とし、M = U + Q とします。UI の Q も区間クエリ回数です。benchmark 内部の引数 q と生ログの queries は総操作数 M に対応します（生ログは変更していません）。
- Static workloads compare all four candidates. Dynamic workloads compare Dynamic Sqrt Tree and ACL Segment Tree.
- Five fresh-process runs per candidate, rotating candidate order: 160 samples total. Seed 20260919; identical generated values/operations within each comparison group.
- Values and replacements lie in [0,1000]. Query endpoints are two uniform positions, sorted into a nonempty inclusive interval and passed as a half-open range. The short workload is the prefix of the long workload with the same N/update rate.
- Record construction and operation times separately, both median and minimum. Also record per-run total time and whole-process VmHWM memory. Input generation and destruction are outside the timer; query-checksum mixing and one final full-range query are inside operation time.
- Save compiler/version/flags, CPU/platform, repository/ACL revisions, source SHA-256, every input hash/checksum, and all raw timings. Reject mismatching checksums or changed source hashes; never substitute missing measurements with zero.

Evidence files after measurement: [runner](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/range-product-selection/run.py), [raw samples](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/range-product-selection/samples.jsonl), [environment and hashes](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/range-product-selection/environment.json), [summary](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/range-product-selection/summary.json), [compile log](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/range-product-selection/compile.log).

## Results

Measured with GCC 13.3.0, `-std=gnu++20 -O2 -DNDEBUG`, on Intel Core i7-14650HX / WSL2, during the coordinated idle window. All 160 samples passed equal-input/result and unchanged-source checks. Times are milliseconds; RSS is whole-process peak KiB. Each timing cell is **median / minimum** from five runs. Total is computed per run before taking its median, so it need not equal the sum of separately computed medians. Tiny 100-operation timings are sensitive to startup/cache/timer noise; compare total costs, not small query-time differences.

### N = 4,096

| 総操作数 M | Updates | Candidate | Build ms | Operations ms | Total ms | Peak RSS KiB (median) |
| ---: | ---: | --- | ---: | ---: | ---: | ---: |
| 100 | 0% | ACL Segment Tree | 0.018800 / 0.017034 | 0.005975 / 0.005789 | 0.024725 / 0.023009 | 2020 |
| 100 | 0% | DST | 0.128199 / 0.114500 | 0.000789 / 0.000781 | 0.128985 / 0.115289 | 3668 |
| 100 | 0% | Dynamic Sqrt | 0.128536 / 0.117357 | 0.001680 / 0.001500 | 0.130216 / 0.118965 | 3680 |
| 100 | 0% | Static Sqrt | 0.138969 / 0.117517 | 0.000891 / 0.000836 | 0.139937 / 0.118353 | 3688 |
| 100 | 1% | ACL Segment Tree | 0.020084 / 0.016509 | 0.005934 / 0.005490 | 0.026101 / 0.021999 | 2064 |
| 100 | 1% | Dynamic Sqrt | 0.118911 / 0.116554 | 0.001979 / 0.001825 | 0.120994 / 0.118379 | 3640 |
| 100 | 50% | ACL Segment Tree | 0.016727 / 0.015910 | 0.003568 / 0.003346 | 0.020394 / 0.019256 | 2060 |
| 100 | 50% | Dynamic Sqrt | 0.122279 / 0.103975 | 0.008568 / 0.007501 | 0.129780 / 0.114543 | 3668 |
| 100,000 | 0% | ACL Segment Tree | 0.023093 / 0.019812 | 5.547099 / 5.352721 | 5.570434 / 5.372533 | 5476 |
| 100,000 | 0% | DST | 0.119348 / 0.104617 | 0.691051 / 0.649425 | 0.810399 / 0.754042 | 5956 |
| 100,000 | 0% | Dynamic Sqrt | 0.127088 / 0.110743 | 1.336749 / 1.221676 | 1.463837 / 1.332419 | 5792 |
| 100,000 | 0% | Static Sqrt | 0.138764 / 0.131572 | 0.717134 / 0.665632 | 0.871518 / 0.797204 | 6064 |
| 100,000 | 1% | ACL Segment Tree | 0.021898 / 0.018948 | 5.246844 / 5.148872 | 5.286795 / 5.170770 | 5472 |
| 100,000 | 1% | Dynamic Sqrt | 0.120959 / 0.107533 | 1.378989 / 1.334038 | 1.540273 / 1.454997 | 5796 |
| 100,000 | 50% | ACL Segment Tree | 0.021172 / 0.018199 | 3.019692 / 2.987886 | 3.040864 / 3.006085 | 5460 |
| 100,000 | 50% | Dynamic Sqrt | 0.118117 / 0.111224 | 4.922376 / 4.764378 | 5.036137 / 4.875602 | 5752 |

### N = 65,536

| 総操作数 M | Updates | Candidate | Build ms | Operations ms | Total ms | Peak RSS KiB (median) |
| ---: | ---: | --- | ---: | ---: | ---: | ---: |
| 100 | 0% | ACL Segment Tree | 0.298995 / 0.288793 | 0.007933 / 0.007810 | 0.306805 / 0.296726 | 4720 |
| 100 | 0% | DST | 2.600427 / 2.332677 | 0.002953 / 0.002247 | 2.603380 / 2.334924 | 12400 |
| 100 | 0% | Dynamic Sqrt | 1.760587 / 1.637555 | 0.004089 / 0.002514 | 1.764676 / 1.640069 | 10104 |
| 100 | 0% | Static Sqrt | 2.153779 / 2.021124 | 0.003822 / 0.003008 | 2.157240 / 2.024946 | 10812 |
| 100 | 1% | ACL Segment Tree | 0.290745 / 0.246407 | 0.007967 / 0.007560 | 0.298725 / 0.254450 | 4700 |
| 100 | 1% | Dynamic Sqrt | 1.781017 / 1.666444 | 0.004890 / 0.003655 | 1.784672 / 1.670154 | 9760 |
| 100 | 50% | ACL Segment Tree | 0.292401 / 0.270460 | 0.004940 / 0.004362 | 0.297278 / 0.274822 | 4740 |
| 100 | 50% | Dynamic Sqrt | 1.739755 / 1.689051 | 0.025028 / 0.022172 | 1.764814 / 1.711223 | 9768 |
| 100,000 | 0% | ACL Segment Tree | 0.312413 / 0.290216 | 7.564932 / 7.540016 | 7.877345 / 7.842659 | 7048 |
| 100,000 | 0% | DST | 2.546593 / 2.215553 | 1.222193 / 1.027106 | 3.785836 / 3.242659 | 14736 |
| 100,000 | 0% | Dynamic Sqrt | 1.703624 / 1.648064 | 1.168038 / 1.155529 | 2.871444 / 2.816102 | 12092 |
| 100,000 | 0% | Static Sqrt | 2.026655 / 1.784993 | 0.829876 / 0.762401 | 2.854890 / 2.547394 | 13144 |
| 100,000 | 1% | ACL Segment Tree | 0.291075 / 0.287857 | 7.631554 / 7.585210 | 7.920185 / 7.873067 | 7116 |
| 100,000 | 1% | Dynamic Sqrt | 1.834073 / 1.653911 | 1.641180 / 1.551283 | 3.480878 / 3.342693 | 12144 |
| 100,000 | 50% | ACL Segment Tree | 0.311862 / 0.292493 | 4.656604 / 4.505287 | 5.010541 / 4.817149 | 7072 |
| 100,000 | 50% | Dynamic Sqrt | 1.777020 / 1.604675 | 14.534732 / 14.104272 | 16.139407 / 15.715563 | 12132 |

### What changes the choice

For 100 operations, ACL has the smallest median total time at both sizes and every update rate, including the fully static case. At N=65,536 with no updates, its total is 0.307 ms versus DST 2.603 ms, Static Sqrt 2.157 ms, and Dynamic Sqrt 1.765 ms. Construction dominates; static-only filtering would hide a useful choice.

For 100,000 static queries, preprocessing is amortized. At N=4,096, DST has the lowest median total (0.810 ms versus Static Sqrt 0.872 ms and ACL 5.570 ms). At N=65,536, Static Sqrt and Dynamic Sqrt totals are close (2.855 / 2.871 ms), versus DST 3.786 ms and ACL 7.877 ms. This close pair is not evidence of a universal ordering.

At N=65,536 with 100,000 operations and 1% updates, Dynamic Sqrt totals 3.481 ms versus ACL 7.920 ms. At 50% updates, ACL totals 5.011 ms versus Dynamic Sqrt 16.139 ms. The measured reversal supports distinguishing infrequent and frequent updates instead of grouping all dynamic tasks together.

ACL uses less process memory in these groups. For N=65,536 and 100,000 static queries, median peak RSS is 7,048 KiB for ACL, 14,736 KiB for DST, 13,144 KiB for Static Sqrt, and 12,092 KiB for Dynamic Sqrt. These include the common generated inputs and executable; they are not isolated allocation sizes.

Short workloads require considering preprocessing together with query time. Query-heavy static workloads amortize preprocessing, while update-heavy workloads expose Dynamic Sqrt Tree's square-root replacement cost. The measurements apply to these sums and interval distributions; expensive monoids, different ranges, and memory limits can change the choice. No universal crossover count is inferred from two operation-count levels.


測定後に runner へ main guard と --output の上書き防止を追加しました。import 時には測定を開始せず、既存の測定ファイルがある出力先は拒否します。既存の環境記録は C++ benchmark・header・ACL のハッシュを対象としており runner 自体は含みません。測定処理と保存済み結果は変更していません。
