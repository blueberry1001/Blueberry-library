# Closest pair — bounded performance investigation

公開実ヘッダと、独立実装のper-node-index-vector分割統治・balanced-tree sweepを同条件で比較した。
全候補は符号付き64bit座標、|c|≤2^62−1、正確な128bit距離、元添字対の辞書順最小tie、debug座標検査を揃える。
元の公開資料は型・距離・tie契約が異なるため、そのままの提出時間とは比較しない。Fastest API403は既知の制約として維持し、再試行していない。

Production SHA256: `d949453afd1f3695a4f63586e15785cac518188c20f3fe5d6b49d9d06de64214`。

GCC14/Clang19、GNU++20、-O2。releaseは-DNDEBUG、assertは有効。生成seed0x43504c4f53455354。
各条件warmup1回＋測定3回、候補順を循環。kernel288profile＋I/O64profile（計352、warmup88）。
別途allocation18＋RSS18profile。6binary各2071caseの全対比較oracle、108small profile、9件の期待SIGABRTを準備ゲートで確認した。
共有ホスト・CPU非固定・cgroup4CPU。min/maxは3標本の範囲であり、統計的信頼区間ではない。

## 完全なアルゴリズム呼出し

単位ms、中央値[min,max]。制御候補の%はpublic比（正は遅い）。入力生成・hashは時間外。

| compiler/mode | 入力/N | public ms [min,max] | vectors % | sweep % |
|---|---|---:|---:|---:|
| g++/release | random/20000 | 4.896 [4.525, 5.245] | +24.3% (slower) | -35.1% (faster) |
| g++/release | random/100000 | 26.669 [25.803, 27.189] | +17.3% (slower) | -44.7% (faster) |
| g++/release | duplicates/100000 | 12.971 [12.235, 13.444] | -4.5% (overlap) | +0.9% (overlap) |
| g++/release | grid/100000 | 32.662 [32.572, 34.326] | -7.7% (faster) | -22.4% (faster) |
| g++/release | nearline/100000 | 18.995 [18.065, 20.237] | -4.7% (overlap) | -23.9% (faster) |
| g++/release | extreme/100000 | 29.834 [28.001, 30.905] | +6.7% (slower) | -44.6% (faster) |
| g++/assert | random/20000 | 4.558 [4.544, 4.740] | +19.1% (slower) | -21.5% (faster) |
| g++/assert | random/100000 | 25.737 [25.412, 26.639] | +18.0% (slower) | -41.8% (faster) |
| g++/assert | duplicates/100000 | 12.676 [12.261, 13.062] | +3.2% (overlap) | +4.5% (overlap) |
| g++/assert | grid/100000 | 33.407 [32.922, 34.071] | -14.2% (faster) | -14.5% (faster) |
| g++/assert | nearline/100000 | 18.465 [18.359, 19.834] | -3.8% (faster) | -20.8% (faster) |
| g++/assert | extreme/100000 | 27.624 [26.441, 29.223] | +12.5% (slower) | -39.3% (faster) |
| clang++/release | random/20000 | 6.313 [4.866, 18.319] | -11.7% (overlap) | -44.2% (faster) |
| clang++/release | random/100000 | 27.173 [25.881, 27.316] | +23.2% (slower) | -40.3% (faster) |
| clang++/release | duplicates/100000 | 13.404 [12.954, 13.858] | -0.3% (overlap) | -5.8% (overlap) |
| clang++/release | grid/100000 | 28.009 [26.963, 29.295] | +0.7% (overlap) | +7.9% (slower) |
| clang++/release | nearline/100000 | 17.220 [16.783, 17.315] | +6.9% (slower) | -3.9% (overlap) |
| clang++/release | extreme/100000 | 27.539 [27.311, 28.284] | +13.7% (slower) | -35.0% (faster) |
| clang++/assert | random/20000 | 4.718 [4.702, 4.823] | +20.8% (slower) | -22.2% (faster) |
| clang++/assert | random/100000 | 27.075 [25.460, 27.121] | +15.1% (slower) | -40.6% (faster) |
| clang++/assert | duplicates/100000 | 13.833 [13.324, 14.305] | -3.7% (overlap) | +3.3% (overlap) |
| clang++/assert | grid/100000 | 28.969 [27.474, 29.040] | +0.8% (overlap) | +2.2% (slower) |
| clang++/assert | nearline/100000 | 17.728 [17.342, 17.880] | +5.7% (slower) | -8.2% (faster) |
| clang++/assert | extreme/100000 | 27.857 [27.349, 31.023] | +11.8% (overlap) | -31.2% (faster) |

prepare/searchは制御候補だけの別の計測呼出しであり、fullに足し合わせない。prepareには検査・record copy・sort・duplicate scan、searchにはDC/sweep本体を含む。publicの内訳はnull。
vectorsはallocationだけでなくrecords/indices配置と比較式も異なる。allocation回数だけを時間差の唯一の原因とはしない。

## I/O

publicアルゴリズムを固定し、FastIOとunsynchronizediostreamを独立に変更。単位ms。parseは入力container構築、solveは回答vector作成、formatはflushを含む。
end-to-endは同一実行内のparse＋solve＋formatで、process startupを含まない。

| compiler/mode | 入力 | FastIO parse/solve/format/end-to-end中央値 | iostream parse/solve/format/end-to-end中央値 |
|---|---|---:|---:|
| g++/release | random | 5.063 / 25.346 / 0.033 / 30.725 | 11.319 / 25.777 / 0.039 / 37.139 |
| g++/release | tiny | 1.187 / 1.135 / 0.214 / 2.509 | 3.260 / 1.143 / 0.713 / 5.233 |
| g++/assert | random | 5.047 / 27.108 / 0.031 / 31.929 | 11.986 / 26.198 / 0.045 / 38.213 |
| g++/assert | tiny | 1.114 / 1.131 / 0.193 / 2.448 | 3.336 / 1.138 / 0.719 / 5.181 |
| clang++/release | random | 7.092 / 26.056 / 0.029 / 33.277 | 11.969 / 27.062 / 0.045 / 39.048 |
| clang++/release | tiny | 1.805 / 1.271 / 0.215 / 3.256 | 3.505 / 1.273 / 0.720 / 5.497 |
| clang++/assert | random | 5.839 / 27.322 / 0.054 / 33.046 | 12.256 / 26.502 / 0.068 / 38.882 |
| clang++/assert | tiny | 1.686 / 1.223 / 0.224 / 3.133 | 3.490 / 1.283 / 0.740 / 5.547 |

## メモリ診断

別のfresh processでGCCを使用。allocationはfull呼出し中の回数/requested bytesで、計測用newを使った時間は性能集計から除外。
RSSは別のuninstrumented診断のwhole-process high-waterで入力生成・runtime・allocatorを含む。staged callを実行しない。通常sampleのRSSは比較に使わない。

| mode | 入力 | variant | allocation calls | requested bytes | peak RSS KiB |
|---|---|---|---:|---:|---:|
| release | random | blueberry | 2 | 4800000 | 7808 |
| release | random | vectors | 103391 | 14854272 | 6504 |
| release | random | sweep | 100001 | 8000000 | 5504 |
| release | duplicates | blueberry | 1 | 2400000 | 5504 |
| release | duplicates | vectors | 1 | 2400000 | 5504 |
| release | duplicates | sweep | 1 | 2400000 | 5504 |
| release | grid | blueberry | 2 | 4800000 | 7808 |
| release | grid | vectors | 103391 | 14854272 | 6504 |
| release | grid | sweep | 100001 | 8000000 | 5504 |
| assert | random | blueberry | 2 | 4800000 | 7808 |
| assert | random | vectors | 103391 | 14854272 | 6508 |
| assert | random | sweep | 100001 | 8000000 | 5504 |
| assert | duplicates | blueberry | 1 | 2400000 | 5504 |
| assert | duplicates | vectors | 1 | 2400000 | 5504 |
| assert | duplicates | sweep | 1 | 2400000 | 5504 |
| assert | grid | blueberry | 2 | 4800000 | 7808 |
| assert | grid | vectors | 103391 | 14854272 | 6516 |
| assert | grid | sweep | 100001 | 8000000 | 5504 |

## 再現・調査資料

`benchmark/closest-pair.py`のprepare/measure/diagnostics/summarizeを、新しいexperiment名で順に実行する。source /tmp/blueberry-setup/env.shが必要。既存experimentは上書きしない。
入力・source・binary・compiler flags・環境はprepared.json、全生値はraw.jsonl/diagnostics.jsonl、phaseのmedian/min/maxはsummary.json、全48対比較はcomparison.json/csv。
詳しい事前計画は../../benchmark-plan.md。公式29caseの生成・58hash照合は../../datasets/report.json。公式checkerはany-tieを認めるため、lexicographic契約は独立oracleで確認する。
公開資料: [LC pinned DC](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/sol/correct.cpp), [KACTL CC0 sweep](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ClosestPair.h), [maspypy DC/randomized grid](https://github.com/maspypy/library/blob/main/geo/closest_pair.hpp)。
KACTL/maspypyは取得2026-10-03のsource hashを../../research/metadata.jsonに保存。アルゴリズム研究のみでソースコードをコピーしていない。ランダムgridは期待計算量や契約が異なるため測定代替案には採用していない。
