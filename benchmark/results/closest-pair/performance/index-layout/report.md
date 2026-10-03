# Shared y-index layout follow-up

初回とは別の測定。公開ヘッダd949453aを固定し、x-sorted records＋共有int order/scratch案と比較した。
GCC14/Clang19、GNU++20 -O2、release/assertを分離。同じ6入力・seed、warmup1＋測定3回。192profile中warmup48。I/O/sweepの再測定なし。
中央値/min/maxは3標本の観測範囲。共有ホスト・CPU非固定で統計的信頼区間ではない。layout/比較式も変わるためコピー量だけへの因果帰属はしない。

| compiler/mode | 入力/N | public ms [min,max] | candidate ms [min,max] | candidate比 | 範囲 |
|---|---|---:|---:|---:|---|
| g++/release | random/20000 | 4.693 [4.518,5.705] | 5.301 [5.145,7.056] | +12.96% | overlap |
| g++/release | random/100000 | 26.392 [26.328,28.124] | 32.240 [28.549,42.249] | +22.16% | slower |
| g++/release | duplicates/100000 | 13.609 [12.970,13.850] | 14.166 [13.536,17.215] | +4.09% | overlap |
| g++/release | grid/100000 | 38.504 [33.643,38.985] | 29.312 [28.989,29.448] | -23.87% | faster |
| g++/release | nearline/100000 | 21.091 [17.680,22.485] | 17.051 [16.490,17.806] | -19.15% | overlap |
| g++/release | extreme/100000 | 31.936 [28.508,34.530] | 32.057 [31.354,33.138] | +0.38% | overlap |
| g++/assert | random/20000 | 4.744 [4.741,5.643] | 5.808 [5.039,13.804] | +22.43% | overlap |
| g++/assert | random/100000 | 29.052 [26.814,50.806] | 36.429 [32.421,41.412] | +25.39% | overlap |
| g++/assert | duplicates/100000 | 13.966 [13.631,16.659] | 14.296 [13.147,14.624] | +2.36% | overlap |
| g++/assert | grid/100000 | 33.698 [32.724,34.818] | 30.014 [28.171,42.986] | -10.93% | overlap |
| g++/assert | nearline/100000 | 19.336 [18.830,19.383] | 18.708 [17.650,21.204] | -3.25% | overlap |
| g++/assert | extreme/100000 | 27.771 [26.537,27.918] | 32.242 [30.629,35.986] | +16.10% | slower |
| clang++/release | random/20000 | 5.125 [4.787,5.626] | 5.038 [4.794,5.788] | -1.69% | overlap |
| clang++/release | random/100000 | 29.608 [28.763,29.790] | 32.456 [28.267,34.388] | +9.62% | overlap |
| clang++/release | duplicates/100000 | 14.179 [12.611,19.125] | 17.569 [13.858,24.489] | +23.91% | overlap |
| clang++/release | grid/100000 | 30.863 [27.911,38.741] | 31.664 [31.070,65.487] | +2.59% | overlap |
| clang++/release | nearline/100000 | 18.024 [17.442,26.180] | 17.664 [16.614,22.198] | -1.99% | overlap |
| clang++/release | extreme/100000 | 30.712 [27.273,39.247] | 30.532 [29.431,31.097] | -0.59% | overlap |
| clang++/assert | random/20000 | 4.845 [4.777,9.640] | 12.820 [5.508,13.769] | +164.62% | overlap |
| clang++/assert | random/100000 | 29.134 [28.195,75.248] | 29.763 [28.741,83.923] | +2.16% | overlap |
| clang++/assert | duplicates/100000 | 18.654 [13.457,40.543] | 13.515 [13.163,38.100] | -27.55% | overlap |
| clang++/assert | grid/100000 | 29.471 [28.943,56.767] | 67.719 [27.781,83.991] | +129.79% | overlap |
| clang++/assert | nearline/100000 | 22.991 [20.903,27.714] | 20.016 [17.234,38.166] | -12.94% | overlap |
| clang++/assert | extreme/100000 | 28.357 [28.040,43.478] | 29.644 [28.627,30.772] | +4.54% | overlap |

中央値短縮9/24、短縮の標本範囲分離1/24。遅い中央値15/24、遅い標本範囲分離2/24。

search phaseにはorder/scratchの確保・初期化を含み、prepareは座標検査・record copy・sort・duplicate scan。いずれも独立したinstrumented callで、full中央値と足し合わせない。public phasesはnull。
手順は../../index-layout-plan.md、source/hash/flagsと2071case×6binary oracle＋72small profile＋6SIGABRTゲートはprepared.json。生値raw.jsonl、別診断diagnostics.jsonl、全phase範囲summary.json。
