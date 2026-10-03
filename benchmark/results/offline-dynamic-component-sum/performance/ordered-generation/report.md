# OfflineDynamicComponentSum storage comparison

CSRを維持する。churnでは全4compiler/modeでfull中央値が短縮して標本範囲も分離し、全4入力でRSSが小さい。一方、longではsolve中央値が全4条件で増加し、GCC/assertのfull遅延は標本範囲も分離した。一律の高速化は主張しない。

実ヘッダSHA256: `7d54e1694d42da12503677a76f853197fc592ae351e60d6cf916cbb2694e6be9`。対照は同じヘッダからbucket格納部分とclass名だけを変更。control.diffで全差分を確認できる。
同一long long入力、同一event ID/order、query-time compression、集約undo、RollbackUnionFindを使用。GCC14/Clang19、GNU++20、-O2、release/assert。
各条件warmup1＋測定3。kernel128＋release I/O16profile中warmup36。別allocation8＋uninstrumented RSS8profile。共有ホスト・CPU非固定・4CPU quota。標本範囲は信頼区間ではない。

## Kernel

単位ms、中央値[min,max]。%はCSR/publicのnested buckets比、正は遅い。
| compiler/mode | 入力 | public full | buckets full | full %/範囲 | solve % | registration % |
|---|---|---:|---:|---:|---:|---:|
| g++/release | long | 92.053 [89.319,95.200] | 85.930 [80.450,91.581] | +7.13%/overlap | +8.90% | +4.76% |
| g++/release | churn | 39.061 [38.075,39.728] | 44.273 [40.826,52.399] | -11.77%/faster | -16.77% | +17.07% |
| g++/release | updates | 79.418 [79.391,84.003] | 82.323 [81.878,84.757] | -3.53%/overlap | -4.56% | +0.07% |
| g++/release | cyclic | 80.334 [69.849,92.278] | 77.760 [74.072,166.105] | +3.31%/overlap | -10.54% | +21.20% |
| g++/assert | long | 114.785 [101.496,123.906] | 92.473 [85.657,95.423] | +24.13%/slower | +25.77% | +16.72% |
| g++/assert | churn | 39.303 [38.441,44.895] | 52.776 [45.667,214.315] | -25.53%/faster | -18.22% | -6.62% |
| g++/assert | updates | 82.208 [82.095,86.161] | 93.063 [85.821,99.783] | -11.66%/overlap | -11.07% | -3.37% |
| g++/assert | cyclic | 70.899 [67.183,71.200] | 82.603 [79.296,88.987] | -14.17%/faster | -21.55% | -11.96% |
| clang++/release | long | 102.240 [93.914,114.334] | 101.833 [98.897,103.148] | +0.40%/overlap | +4.78% | +4.73% |
| clang++/release | churn | 34.728 [34.681,37.215] | 40.970 [39.665,42.781] | -15.24%/faster | -17.75% | +1.84% |
| clang++/release | updates | 75.306 [66.199,82.743] | 78.700 [73.855,93.007] | -4.31%/overlap | -6.68% | -3.00% |
| clang++/release | cyclic | 58.463 [57.447,59.254] | 64.990 [64.265,102.552] | -10.04%/faster | -16.43% | -0.34% |
| clang++/assert | long | 104.216 [102.369,127.326] | 104.750 [92.547,118.832] | -0.51%/overlap | +3.99% | +4.71% |
| clang++/assert | churn | 36.817 [34.641,39.669] | 44.107 [41.054,48.941] | -16.53%/faster | -20.53% | +3.13% |
| clang++/assert | updates | 74.594 [67.755,75.096] | 77.332 [71.534,97.905] | -3.54%/overlap | -4.40% | -2.02% |
| clang++/assert | cyclic | 61.142 [58.182,71.117] | 66.276 [64.450,74.013] | -7.75%/overlap | -17.17% | +7.57% |

constructorは初期値copy、registrationは全操作登録、solveは区間抽出・bucket構築・DFS・回答作成を含む。fullはrecorder破棄まで。すべて同じ呼出し内の計測で、内部工程の独立分離を主張しない。
registrationは実装差のないcontrol。GCC/release cyclicのregistration増加がfull中央値を逆転させた事実も保存する。constructor/registrationを含む全64phase比較と範囲はcomparison.jsonにあり、変動を都合よく除外しない。

## I/O

公開APIを固定し、FastIOとunsynchronizediostreamだけを変更。churn N=Q=300000、releaseのみ。parseは入力vector作成、algorithmはconstructor/registration/solve/destruction、formatはflushを含む。process起動は時間外。

| compiler | FastIO parse/algorithm/format/end-to-end ms | iostream parse/algorithm/format/end-to-end ms | FastIO end-to-end %/範囲 |
|---|---:|---:|---:|
| g++ | 15.451 / 39.284 / 2.679 / 56.998 | 40.820 / 43.510 / 6.816 / 91.355 | -37.61%/overlap |
| clang++ | 18.524 / 36.021 / 2.647 / 57.293 | 41.044 / 35.839 / 6.229 / 85.594 | -33.06%/faster |

## Memory

各条件1回の独立したGCC/release診断。allocationはfull API lifetimeの回数/requested bytesで、instrumented時間は集計しない。RSSは入力・runtime・allocatorを含むfresh processのhigh-water。bucketのみの値ではない。

| 入力 | public allocations / bytes / RSS KiB | buckets allocations / bytes / RSS KiB |
|---|---:|---:|
| long | 100063 / 37395396 / 43364 | 300100 / 49486940 / 48220 |
| churn | 60062 / 25621436 / 27444 | 300074 / 30798192 / 36276 |
| updates | 44 / 41537336 / 38808 | 180056 / 73369004 / 46104 |
| cyclic | 28532 / 13249352 / 18528 | 204371 / 22898976 / 22964 |

5binary各217 BFS program、非default/非inverse型、4large入力の全回答要素一致、40small cross-compiler profilesを準備ゲートで確認。
初回prepareはRNG引数の評価順によりGCC/Clang入力hashが異なり中止。元の失敗証拠を維持し、明示的local変数で生成順を固定したordered-generation実験だけを測定した。API本体の不具合ではなく、失敗回の性能値は存在しない。
再現はbenchmark/offline-dynamic-component-sum.pyのprepare/measure/diagnostics/summarize。計画、source/header/control/binary hashとflagsはplan.md/prepared.json、全生値はraw.jsonl/diagnostics.jsonl。元のdatasetと公式API検証は別記録であり、この性能比較を公式ACとして数えない。
