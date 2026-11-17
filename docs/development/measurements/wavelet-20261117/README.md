# Wavelet Matrix の構築と範囲頻度

Library Checker の Range Kth Smallest は、[過去の最速調査](../../fastest-other-structures-study.md)で上位の[提出403008](https://judge.yosupo.jp/submission/403008)を確認した。全クエリを先読みして bit 段ごとにまとめて処理し、AVX2 gather も使うため、任意順にクエリを受ける既存 API と契約が異なる。[提出376681](https://judge.yosupo.jp/submission/376681)は rank の bit と累積値を同じ構造体に配置していた。順位は2026-09-19調査時のもので、今回の最新順位はネットワーク制限のため取得できていない。

Nyaan の [Wavelet Matrix](https://nyaannyaan.github.io/library/data-structure-2d/wavelet-matrix.hpp.html) は bitvector の rank に BMI2 と POPCNT を使う。値域・ビット幅、CPU 命令、API の前提が異なるので、掲載タイムを Blueberry の速度と直接比較しない。以下は Blueberry の同一契約を持つ候補間の測定であり、上記提出を同一環境で再測定した結果ではない。

最初の比較は Windows / MSVC 14.43、`/std:c++20 /O2 /DNDEBUG`、N=200,000、Q=100,000、warmup 1 回と測定 5 回。異なる Codex セッションのテストが並行していたため、CPU負荷の干渉があり、数%の差は有意とは扱えない。入力は全幅64bit乱数、32種類重複、狭い値域、整列済み、符号付き最小値と最大値の重複、先頭だけ乱数かつ残りは両極値の6分布。各候補の checksum は各 run で一致した。メモリ、パース、出力時間は別途測っていない。

例として、最小値・最大値の重複で構築の中央値は baseline 10.89ms、combined 4.44ms。同値域 `range_freq(l,r,x,x)` 10万回は狭い値域の値列で baseline 38.33ms、combined 14.20ms。異なる幅の `range_freq` や kth は各ケースの [`first-results.json`](first-results.json) に分けて記録した。この等値域は答えが常に0という特殊な分布であり、一般的なクエリ全体の速度比ではない。実行対象と同一の4種類のヘッダを baseline / build / query / combined の snapshot に保存した。

再現にはWindowsのBuild ToolsとSDKが必要。作業ディレクトリをリポジトリルートにして `python docs/development/measurements/wavelet-20261117/first-run.py` を実行する。元の測定driverは `first-driver.cpp.txt`、候補のSHA-256と全sampleは `first-results.json` にある。source生成手順はrunnerに含む。記載したsource snapshotは測定時のもの、公開ヘッダは不要な契約assertと説明コメントを削除しており、同じファイルハッシュとは限らない。

## 最終候補

最終版は、辞書の値が連続する場合に先頭値との差で順位を求める。そうでなくても、値域幅が `min(2^20, 8N)` 以下なら、値域内の各位置にそれ未満の値の種類数を記録する。表の生成・初期化も構築時間に含める。表を保持すると最大 `min(2^20, 8N) + 1` 個のintが追加で必要になる。連続値域は表を使わず保持せず、ほかの値域は従来どおり二分探索する。符号付き値の差はunsignedで計算する。構築後の高さや操作の保証は変わらない。

値域の両端が同じ圧縮順位に落ちる場合は0を返し、異なる場合も上位桁が共通なら、その部分の走査と演算を共有する。固定された等値域ではコンパイラが呼び出し自体を最適化することがあり、以下の等値域結果を通常の値域問い合わせの速度とは見なせない。

Windows 11、Intel Family 6 Model 183、MSVC 14.43.34808とWindows向けClang 18.0.3、`-O2` 相当・`NDEBUG`、N=200,000、Q=100,000、warmup 1回・各5回。クエリを測定前に作り、候補順を入れ替えた。CPU固定はしていないため、他の作業による干渉と数%の変動は区別できない。次は中央値msで、構築・kth・値域頻度は別々のループで計測した。

| compiler・値分布 | 構築 旧→最終 | kth 旧→最終 | 任意値域頻度 旧→最終 | 幅1以下の値域頻度 旧→最終 |
| --- | ---: | ---: | ---: | ---: |
| MSVC・全幅乱数 | 18.50→19.04 | 19.65→19.38 | 40.86→40.27 | 37.87→28.56 |
| MSVC・整列済み | 7.23→5.73 | 17.32→16.89 | 37.43→21.05 | 34.65→12.04 |
| Clang・全幅乱数 | 16.38→17.74 | 25.00→23.66 | 45.75→41.63 | 38.27→25.15 |
| Clang・整列済み | 6.37→4.80 | 21.46→21.08 | 39.42→27.42 | 33.99→15.44 |
| Clang・最小値/最大値のみ | 6.19→2.39 | 0.95→0.95 | 1.48→1.34 | 1.31→1.30 |

全幅乱数の構築には回帰が見られる。型が異なる場合・他CPU・負荷下でも同じ傾向とは限らない。rankのbit列と累積値を同じ要素にまとめる案も測ったが、Clang の全幅乱数 kth は25.47ms、最終案23.66ms、整列済み任意値域頻度は29.76ms、最終案27.42msで、不利な分布があるため採用しなかった。全6分布、等値域、各測定値、checksumとソースhashは [`second-results.json`](second-results.json)。

## Nyaan との直接比較

2026-11-17取得の Nyaan のソース（blob `4ed107693cf81df2e2e0fe3e74c5f34e83ea17bf`、[CC0 1.0](https://github.com/NyaanNyaan/library/blob/master/LICENSE)）を [`nyaan-raw.hpp.txt`](nyaan-raw.hpp.txt) に原本として保存した。MSVC標準ライブラリには GNU の `__lg` がないため、ドライバで正の整数に対する等価な補助関数を定義した。Nyaan本体は改変していない。

Windows Clang 18.0.3、両者とも `-O2 -DNDEBUG -mbmi2 -mpopcnt`、同じ生成入力・同じドライバ・N=200,000、Q=100,000、warmup 1回、交互に各5回。Nyaanの契約に合わせ、非負の20bit整数のみとする。BMI2/POPCNTを使えるCPUでの比較であり、Blueberryの符号付き極値や汎用性の検証ではない。全checksumは一致した。

| 分布 | 構築 旧 / 最終 / Nyaan | kth 旧 / 最終 / Nyaan | 任意値域頻度 旧 / 最終 / Nyaan | 幅1頻度 旧 / 最終 / Nyaan |
| --- | ---: | ---: | ---: | ---: |
| 20bit乱数 | 13.98 / 15.09 / 16.54 | 18.84 / 18.84 / 18.18 | 37.16 / 26.51 / 19.46 | 28.31 / 12.96 / 18.16 |
| 32種類重複 | 4.79 / 4.37 / 4.71 | 4.23 / 4.26 / 3.60 | 5.45 / 4.40 / 3.74 | 4.46 / 2.83 / 2.71 |
| 整列済み | 6.38 / 4.87 / 7.48 | 16.34 / 16.30 / 14.09 | 29.61 / 18.28 / 14.67 | 24.91 / 10.24 / 13.99 |

乱数の20bit入力では表を保持し、追加メモリは最大約4 MiB（intを4 byteとした場合）。連続値域の表は保持しない。最終版の構築が旧版より遅い分布と、Nyaanとの差は残る。Nyaanは値を圧縮せず直接bit段に分ける一方、Blueberryは任意の符号・幅に対応し、値から順位への変換を行う。差をすべてその変換費用だと断定する測定ではない。パース、出力、ピークRSS、公式ケースの全体測定は未実施。全sample、source hash、flagsは [`nyaan-expanded-results.json`](nyaan-expanded-results.json) と対応runnerを参照。

## 正しさの確認

MSVC 14.43 のC++20/C++latestとWindows Clang 18のgnu++20/gnu++23で、更新した `tests/random/wavelet-matrix.cpp` を `-O2` 相当、assert有効でコンパイルし、各seed 1・2・17が成功。符号付き最小値と最大値付近、値域の穴、両端の辞書外、連続値域、偏った分布、copyと空配列を含む。結果・全コマンド・ソースhashは [`windows-test-results.json`](windows-test-results.json)。最初のWindows Clang試行では MSVC標準ライブラリが `__int128` を整数型と扱わずコンパイルが失敗し、対象環境だけ該当テストを無効化した。[失敗ログ](windows-test-first-failed.json)も保持する。Linux GCC、Linux Clang、sanitizer、公式verifyはこの結果に含まない。
