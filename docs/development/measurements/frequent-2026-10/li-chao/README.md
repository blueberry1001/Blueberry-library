# Li Chao Treeの高速化調査

基準revisionは`d6af3e9`。2026-10-17に、座標圧縮版と動的整数領域版を調べた。
公開API、対応型、直線の追加順、半開区間、overflow条件、optionalで空集合を表す契約は維持する。

## 候補と採否

線分を追加する再帰の途中で、既存直線が新しい線分を両端で支配していれば、その部分木への追加を省略する。
動的版では不要なノードの作成もなくなる。比較点はノード領域と線分の共通部分の両端に限る。
線分の有効範囲外では積がoverflowしてもよいという既存契約を維持するためである。
この変更は両実装で採用する。最悪計算量とメモリの上界は変わらない。

圧縮版queryは、座標のlower_boundと木の下降を別々に行っていた。
最初に座標から葉を特定し、その親を順に評価する案を測定したが、Clangで回帰したため棄却した。
比較用の関数を`bottom-up-query.txt`に保存している。
次に、座標との比較で木を下りながら同時に直線を評価する案を測定し、こちらを採用した。
assert有効時の未登録座標チェックは評価前に行い、これまでどおりassertで失敗させる。
assert有効時はチェックで得た添字を下降に再利用し、同じ座標を再比較しない。

部分木の非空フラグを追加する案は未採用・未測定である。
直線が少数だけ残る分布では、空の部分木でも葉まで下降することが圧縮版の支配項となる。
動的版は存在するノードだけをたどるため、この分布で大きな差が残る。
フラグ配列の追加は構築時間・メモリ・全更新への書き込みを増やすため、今回の変更には含めない。
query専用の添字APIや全操作の先読みも既存APIの高速化には含めない。

## 測定条件と再現

- Intel Core i7-14650HX、Ubuntu 24.04 / WSL2、Linux 5.15.167.4。
- GCC 13.3.0、Clang 18.1.3、`-std=gnu++20 -O2 -Wall -Wextra -Werror`。
  releaseは`-DNDEBUG`を追加し、GCCではassert有効時も分離した。
- seed 1729、100,000操作。通常は追加とqueryを各100,000回、query-heavyは追加10,000回とquery100,000回。
- 入力生成は測定外。構築、追加とqueryを交互に行う処理、全追加だけの処理、追加後のqueryだけの処理をそれぞれ計測する。
- 構築はアダプター呼び出しから計測する。動的版アダプターにも座標vectorの値渡しコピーが含まれるので、
  `construction_ms`は動的木そのものの構築時間ではない。圧縮版との純構築時間比較には使わない。
- I/Oは計測外。外部の`/usr/bin/time`によるRSSは入力配列と2個の木を含むプロセス全体のピークである。
  構造単体のメモリではない。動的版は2個の木の合計確保量も変わる。
- 各候補を1回ウォームアップし、5回ずつ直列実行する。全8分布で混在操作とバッチqueryのhashが候補間で一致することを確認する。
- 他担当のcompile/test/benchmarkを止めて測定した。初回の葉から親への案はCPU affinity固定なし、
  追加の下降中評価案は`taskset -c 2`でCPU2に固定した。追加JSONには実際のaffinityも保存した。
  初回と追加を混ぜず、各設定の同じログ内のbaselineと候補を比較する。
  OS全体の排他は行っていない。候補ごとに5回をまとめて実行しており、温度・クロック等の時間変動は残る。
- 全反復、最小値、中央値、コンパイラ、フラグ、候補ソースのSHA-256をJSONに保存する。
  失敗時も取得済みの試行を保存し、欠測を0にしない。

{% raw %}
```sh
python3 docs/development/measurements/frequent-2026-10/li-chao/run.py
python3 docs/development/measurements/frequent-2026-10/li-chao/run.py --assertions
python3 docs/development/measurements/frequent-2026-10/li-chao/run.py --compiler clang++
taskset -c 2 python3 docs/development/measurements/frequent-2026-10/li-chao/run.py --query-variant top-down
taskset -c 2 python3 docs/development/measurements/frequent-2026-10/li-chao/run.py --query-variant top-down --assertions
taskset -c 2 python3 docs/development/measurements/frequent-2026-10/li-chao/run.py --query-variant top-down --compiler clang++
```
{% endraw %}

`run.py`はgitから基準ヘッダーを取り出し、queryのみ・剪定のみ・両方の変更を分離する。
基準revisionが読めるcheckoutで実行する。`--query-variant bottom-up`が初回試行、`top-down`が追加試行である。
`envelope-*`は`a=-2c, b=c*c`の下包絡線を作る。`random-*`は傾きと切片を独立に生成する。
`narrow-segment`は幅0～31、`duplicate-line`はquery座標を31種類以下へ縮め、`ordered-line`は昇順のqueryとする。
全操作は生成済みだが、ライブラリには順番に追加し、各queryをその場で実行する。
動的版に座標の先読みを要求していない。

## 初回測定と回帰

GCC release、混在処理の中央値ms。query候補は棄却した葉から親への方式である。

| 入力 | 圧縮版基準 | queryのみ | 剪定のみ | 両方 | 動的版基準 | 動的版剪定 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| envelope-line | 34.934 | 29.126 | 36.264 | 28.583 | 24.462 | 25.166 |
| random-line | 24.060 | 15.876 | 23.457 | 15.938 | 2.329 | 2.303 |
| query-heavy-line | 18.974 | 13.780 | 18.754 | 14.150 | 7.687 | 7.670 |
| duplicate-line | 2.914 | 2.715 | 2.922 | 3.038 | 16.359 | 16.357 |
| ordered-line | 22.475 | 21.826 | 22.877 | 20.818 | 18.819 | 19.264 |
| envelope-segment | 62.525 | 59.602 | 43.011 | 36.906 | 99.855 | 17.860 |
| random-segment | 56.593 | 56.136 | 34.403 | 32.632 | 94.994 | 9.544 |
| narrow-segment | 33.184 | 29.589 | 34.080 | 30.329 | 59.220 | 60.924 |

剪定は広い線分で大きく改善する一方、狭い線分では比較の追加が負担になる。
GCC releaseのnarrow-segmentは、圧縮版の剪定単独が約2.7%、動的版が約2.9%遅くなった。
この回帰を全入力で高速になったとは扱わない。全試行幅と追加測定はJSONに残す。
Clangの葉から親へのqueryはduplicate-lineで2.039→2.888ms、query-heavyで15.397→16.785msとなった。
GCCだけの改善を根拠にこのquery方式を採用しない。

## 採用版の追加測定

CPU2固定、混在処理の中央値ms。各セルは同じログ内の基準→採用候補である。
圧縮版はqueryと剪定の両方、動的版は剪定だけを変更した。
初回のCPU固定なしの値から改善率を計算しない。

| 入力 | 圧縮版 GCC release | 圧縮版 Clang release | 動的版 GCC release | 動的版 Clang release |
| --- | ---: | ---: | ---: | ---: |
| envelope-line | 31.801→24.062 | 26.479→26.128 | 22.788→22.777 | 17.256→17.941 |
| random-line | 21.743→13.944 | 20.749→18.522 | 2.211→2.186 | 2.391→2.450 |
| query-heavy-line | 17.868→11.808 | 13.783→14.292 | 7.619→7.522 | 4.064→4.167 |
| duplicate-line | 2.766→2.625 | 1.928→1.672 | 16.342→16.288 | 15.651→16.130 |
| ordered-line | 21.482→18.277 | 22.934→22.162 | 19.069→19.443 | 16.670→15.883 |
| envelope-segment | 60.573→34.067 | 48.090→27.949 | 92.876→17.921 | 82.969→12.879 |
| random-segment | 55.434→27.226 | 47.902→23.866 | 88.511→9.298 | 74.784→8.009 |
| narrow-segment | 33.074→27.961 | 23.194→22.784 | 53.117→51.958 | 48.372→46.843 |

広い線分追加は両コンパイラで大幅に改善した。動的版のGCC releaseでプロセス全体のpeak RSS中央値は、
envelope-segmentが86,932→12,884KiB、random-segmentが87,008→10,004KiBだった。
不要ノードの作成を省くため、更新後のqueryも浅い部分木で終了するようになる。

上表は構築を除く操作時間である。構築込みの各試行の合計を取り直した中央値では、GCC圧縮版の
envelope-lineが37.389→29.312ms、random-segmentが61.025→32.926ms、Clang圧縮版の
random-segmentが54.232→29.636msだった。入力生成・I/O・破棄はこの合計にも含まない。

回帰も残る。Clang圧縮版query-heavyの混在処理は中央値で約3.7%増えた。
全5試行の範囲は基準13.198～15.275ms、採用候補13.618～15.192msで重なっている。
queryだけの中央値は12.024→11.860msで、すべての入力で高速化したとは主張しない。
狭い線分の圧縮版更新だけの中央値は、GCC releaseで18.770→19.621ms、
Clang releaseで12.187→12.944msとなり、剪定用比較を増やした負担が残る。
queryを含む全体では表のとおり改善したが、狭い線分の更新が大半の用途には注意が必要である。

GCC assert有効ではqueryの下降を基準と同じにしているため、query単独の改善は目的にしていない。
広い線分の混在処理はenvelope-segmentが60.693→41.901ms、random-segmentが55.665→34.875msとなった。
追加測定の全144組（3設定×6候補×8分布）について、5回ずつの混在・バッチqueryのhash一致を確認した。
初回の144組も同様に確認済みで、棄却候補を含む計1,440回の計測値を6個のJSONに保持している。

## 境界・ランダム検証

`tests/random/li-chao-tree.cpp`に、線分外の評価ではoverflowするケースと、1～65点の非完全二分木・疎座標を追加した。
`tests/random/dynamic-li-chao-tree.cpp`には、共通部分の外で交差する直線と、端点で同値になる線分を追加した。
既存の乱択テストは線分と全域直線を混在させ、全座標を愚直解と比較する。

採用queryの圧縮版をGCC `gnu++20`とClang `gnu++23`、`-O2 -Wall -Wextra -Werror`でコンパイルし、
両コンパイラでseed `1, 2, 42, 1729, 4294967295`をそれぞれ実行した。
両実装をGCC `gnu++20 -O1 -g -fsanitize=undefined -fno-sanitize-recover=undefined`でもコンパイルし、
同じ5seedを実行した。これら計20実行は正常終了した。初回候補もGCC/Clangの境界・乱択比較で確認した。
リポジトリ全体の`make check`、公式verify、docs公開確認は統合担当が実行する。

## 一次ソースの調査

2026-10-17にLibrary Checkerの公開REST APIから、AC提出を時間順・ユーザー重複除去で上位3件取得した。
最初の`--lang C++23`では0件だったため、言語指定を外して再取得した。実際のAPIの言語IDは`cpp`だった。
取得した公開メタデータは`fastest-line.json`と`fastest-segment.json`に保存した。
提出ソースは調査用のignoredな`.build/li-chao-fastest/`にだけ保存し、転載しない。
Fastestの最大ケース時間は今回のローカル混在処理時間と比較しない。

- [Line Add Get Min 362191](https://judge.yosupo.jp/submission/362191): 座標を先読みし、圧縮添字でqueryする。
  葉の値を別配列に置き、親への走査と端点値の再利用を行う。mmap/freadのI/Oも含む。
- [Line Add Get Min 393904](https://judge.yosupo.jp/submission/393904): 座標・傾きを32bitに絞る配置、
  固定INF、添字query、radix sort、SIMDを含むI/Oを使う。
- [Line Add Get Min 401383](https://judge.yosupo.jp/submission/401383): 圧縮添字queryと端点値の再利用、
  AVX2指定とSIMDを含むI/Oを確認した。
- [Segment Add Get Min 357121](https://judge.yosupo.jp/submission/357121)、
  [401386](https://judge.yosupo.jp/submission/401386)と[401384](https://judge.yosupo.jp/submission/401384)では、
  圧縮添字による線分追加、ビット操作で区間を分解する方法、葉の値と親の評価、端点値の再利用を確認した。
  AVX2や固定INF・padding座標を含み、既存の任意型・全値域・オンライン動的版の契約とは異なる。
- [Nyaanの座標圧縮Li Chao Tree](https://nyaannyaan.github.io/library/segment-tree/li-chao-tree.hpp.html)は
  完全二分木の添字queryと親への走査を使う。座標末尾を増やすpaddingやINFの仮定は採用しない。
- [LuzhiledのDynamic Li Chao Tree](https://ei1333.github.io/library/structure/convex-hull-trick/dynamic-li-chao-tree.hpp.html)は
  線分の再帰で既存直線による剪定を行う。今回の剪定はこの性質を使い、評価点を共通部分へ制限して独立に実装した。

固定INF・32bit専用化・ISA必須化・動的版の全座標先読みは採用しない。
端点値のキャッシュは挿入内の候補であるが、今回の採用変更には含めず、未測定とする。
