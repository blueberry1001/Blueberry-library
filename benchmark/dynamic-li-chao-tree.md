# Dynamic Li Chao Tree の所有方式と座標圧縮の比較

## 調査と採用理由

2026-09-17 に [Luzhiled の動的整数座標方式](https://ei1333.github.io/library/structure/convex-hull-trick/dynamic-li-chao-tree.hpp.html) と
[Nyaan の圧縮配列方式](https://nyaannyaan.github.io/library/segment-tree/li-chao-tree.hpp.html)、Library Checker Fastest を調査した。
外部ソースは調査専用として `.build/legacy-li-chao/` に保存し、コードは転載していない。

| 方式 | 前提 | 直線 / 線分追加 | 取得 | メモリ |
| --- | --- | --- | --- | --- |
| 座標圧縮 `LiChaoTree` | query 座標を先読み | O(log N) / O(log² N) | O(log N) | O(N) |
| 比較用 `DynamicCandidate` | 整数領域を指定 | O(log V) / O(log² V) | O(log V) | O(K)、ノードごとに unique_ptr で所有 |
| 採用 `DynamicLiChaoTree` | 整数領域を指定 | 償却 O(log V) / O(log² V) | O(log V) | O(K)、vector と整数 index で所有 |

N は query 座標の異なる個数、V は整数領域幅、K は動的方式の確保ノード数。
表は非空領域についての表記で、空・単点を含む厳密な定義はライブラリ文書を参照。
新実装はオンラインの未知座標を扱うために追加した。生ポインタでの解放漏れと浅いコピーを避け、
INF番兵に依存せず、半開区間と符号なし幅を使う中点計算で端点演算の overflow を回避する。
arena によりノード別 allocation を減らす一方、vector 拡張時にノード全体の移動が発生する。
追加は償却計算量で、単発では O(K) の再確保がありうる。

## Fastest の観察

公開 REST API の AC を `+time` で取得した。2026-09-17 時点の最大ケース時間は次のとおり。
時間はその提出の環境での表示値であり、以下のローカル測定との比率は計算しない。

| 問題 | 提出 | 時間 | 観察 |
| --- | --- | ---: | --- |
| Line Add Get Min | [362191](https://judge.yosupo.jp/submission/362191) | 57 ms | 事前に圧縮した座標、端点評価の持ち回り、葉の答えの分離、radix sort、高速I/O |
| Line Add Get Min | [357336](https://judge.yosupo.jp/submission/357336) / [357321](https://judge.yosupo.jp/submission/357321) | 各58 ms | 圧縮方式、AVX2指定と高速I/Oを含む |
| Segment Add Get Min | [357121](https://judge.yosupo.jp/submission/357121) | 62 ms | 圧縮方式、区間分解のbit操作、端点評価の持ち回り、radix sort、高速I/O |
| Segment Add Get Min | [361262](https://judge.yosupo.jp/submission/361262) / [361261](https://judge.yosupo.jp/submission/361261) | 各63 ms | 圧縮方式と高速I/O |

ユーザー重複を除く結果も調べ、Line Add Get Min の
[393904](https://judge.yosupo.jp/submission/393904)（60 ms）は `IndexedLiChaoTree` を main から利用していた。
同ソースに動的実装が含まれていても、その提出時間をオンライン実装の性能とは扱わない。
SIMDの機械的検出はI/Oなどにも反応するので、本体がSIMD化されている証拠にはしない。

既存の座標圧縮版は端点での支配判定をすでに採用している。新しい動的版にもこの性質を使う。
圧縮・radix sort・事前indexは未知のquery座標へのオンライン処理という今回の要件を満たさないため、
Fastestを模倣してAPIを変更しない。CPU依存命令や独自I/Oはライブラリ本体に導入しない。

## 再現方法と測定条件

Linux / WSL のリポジトリルートで `python3 benchmark/run_dynamic_li_chao.py` を実行する。
ソースは `benchmark/li-chao-tree.cpp`。結果は `benchmark/results/dynamic-li-chao-tree-2026-09-17.json`。
JSON に compiler・flags・CPU・OS・revision・各ソース SHA-256・全反復の stdout/stderr・終了コードを保存する。
初回版と反復化のみの版は同JSONの `source_snapshots` に保存し、runnerが比較用headerを再生成する。
初回測定のログは `benchmark/results/dynamic-li-chao-tree-before-2026-09-17.json` に保持する。
失敗時も途中ログを保持し、欠測や失敗を0に置き換えない。

- 全候補とも GCC、`-std=gnu++20 -O2 -DNDEBUG -Wall -Wextra -Werror -I.`。
- seed 1729、100,000 回の追加と直後の取得、計200,000操作を候補間で共有する。
- 座標コピー・ソート・確保を含む構築と操作を計測。入力生成とデストラクタは時間測定外。
- 各候補のウォームアップ1回の後、5回を直列に実行し、中央値・最小値を報告する。
- 同じ workload の全取得値のハッシュを全候補・全反復で照合する。
- peak RSS は入力配列とプロセス全体を含み、構造単体のメモリ量ではない。
- `line` / `segment`: 整数領域 `[-10^6, 10^6+1)`。中心cから `a=-2c, b=c²` を生成し、多数の直線が下包絡線に現れる。
- `sparse-line`: 領域を `[-10^12, 10^12+1)` に広げ、同数のqueryを疎に分布させる。係数の生成範囲は変えず、積・和はlong longに収まる。
- `narrow-segment`: 線分長を0〜31にする。残りの生成条件は通常領域と同じ。

旧2026-09-14の数値は今回の比較表に混在させない。計測前に他の担当のbuild/verify/benchmarkを止め、
この比較を直列に実施する。WSLホストの他プロセスまではCPU専有にしていないため、微差だけで判断しない。

## 初回結果（再帰挿入）

Intel Core i7-14650HX、Ubuntu 24.04 / WSL2、GCC 13.3.0 で測定した。
他担当の重い処理はこの測定枠では停止した。全候補・全反復の checksum は workload ごとに一致した。

| workload | 候補 | 中央値 ms | 最小 ms | 中央 peak RSS KiB |
| --- | --- | ---: | ---: | ---: |
| line | 座標圧縮 | 43.8580 | 41.9984 | 18876 |
| line | unique_ptr 比較候補 | 29.2576 | 27.1612 | 13348 |
| line | arena 初回版 | 41.9281 | 41.3736 | 16088 |
| segment | 座標圧縮 | 78.5070 | 74.6023 | 19100 |
| segment | unique_ptr 比較候補 | 125.6760 | 114.5170 | 49108 |
| segment | arena 初回版 | 108.8600 | 107.1560 | 46140 |
| sparse-line | 座標圧縮 | 34.5250 | 34.0311 | 19208 |
| sparse-line | unique_ptr 比較候補 | 24.1048 | 23.6637 | 12812 |
| sparse-line | arena 初回版 | 67.4855 | 66.1790 | 15784 |
| narrow-segment | 座標圧縮 | 44.1576 | 42.6639 | 19396 |
| narrow-segment | unique_ptr 比較候補 | 72.8772 | 70.2465 | 44140 |
| narrow-segment | arena 初回版 | 64.6537 | 59.2046 | 46428 |

この初回の分布では arena は unique_ptr 比較候補より線分追加で速かったが、直線追加では遅かった。
特に広い整数領域の sparse-line で差があるため、動的arenaの一律な高速性は主張しない。
また、座標圧縮版は通常・狭区間の線分 workload で動的2候補より速かった。
座標を事前収集できる場合は引き続き座標圧縮版が候補になる。

比較用の unique_ptr 版は既存ベンチマークの独立実装で、測定領域に対して安全な
`l + (r-l)/2` を使う。arena版は符号付き整数の全幅に近い領域、値の独立コピー、移動元の再利用を
保証するため、両者はAPIの保証範囲まで同一ではない。再帰・indexアクセス・安全な中点計算・
再確保などの影響をこの集計だけでは分離できない。RSSも入力を含むため単独のノードサイズの比較ではない。

## 要因を分けた改善比較と採用結果

初回結果の大きな差を調べるため、同じ compiler・flags・入力・反復条件で次の5候補を
別の静かな測定枠で同一binaryに組み込み、直列比較した。次表の候補間だけで比率を比較する。
初回表と2回目表は別の実行であり、同じ測定として集計しない。

- `compressed`: 既存の座標圧縮版。
- `dynamic`: 独立した unique_ptr 比較候補。
- `arena-recursive`: 初回の再帰挿入と `std::midpoint`。
- `arena-iterative`: 挿入だけを反復化し、`std::midpoint` を保持。
- `arena`: 反復挿入と、`l < r` を利用して順序分岐を省いた中点計算（採用）。

最後の中点計算は `U = make_unsigned_t<T>` として、`U width = U(r) - U(l)` の後に
`l + T(width / 2)` を返す。width は領域幅を正確に保持し、半分は T の最大値以下、
結果も `[l,r)` 内なので符号付き overflow は起きない。小型型は差の式が int に昇格しても、
width への代入で U の剰余値を復元する。signed char の全領域を含む sanitizer テストで確認した。

| workload | 候補 | 中央値 ms | 最小 ms | 中央 peak RSS KiB |
| --- | --- | ---: | ---: | ---: |
| line | compressed | 43.8112 | 42.7357 | 19132 |
| line | dynamic | 28.5770 | 27.9030 | 12544 |
| line | arena-recursive | 42.2101 | 41.0363 | 15348 |
| line | arena-iterative | 30.6153 | 30.4356 | 15552 |
| line | arena | 27.1133 | 26.1276 | 15844 |
| segment | compressed | 77.1731 | 74.7030 | 19692 |
| segment | dynamic | 154.1060 | 147.8490 | 49644 |
| segment | arena-recursive | 120.4510 | 111.7620 | 46092 |
| segment | arena-iterative | 127.4620 | 124.7360 | 46364 |
| segment | arena | 125.7940 | 118.2590 | 46372 |
| sparse-line | compressed | 34.7241 | 34.5607 | 19220 |
| sparse-line | dynamic | 23.4345 | 22.6594 | 13376 |
| sparse-line | arena-recursive | 69.4980 | 69.1129 | 15672 |
| sparse-line | arena-iterative | 28.0432 | 27.0632 | 15608 |
| sparse-line | arena | 22.8168 | 21.3209 | 15464 |
| narrow-segment | compressed | 45.3029 | 43.7274 | 19532 |
| narrow-segment | dynamic | 82.6867 | 80.3304 | 44348 |
| narrow-segment | arena-recursive | 61.8143 | 57.4152 | 45908 |
| narrow-segment | arena-iterative | 75.5954 | 74.3276 | 46124 |
| narrow-segment | arena | 66.8170 | 59.5167 | 46188 |

全候補・全反復で workload ごとの checksum は一致した。
再帰arena比で、採用版は line が約36%、sparse-line が約67%短縮した。
反復化だけで差の多くが減るため、この比較では再帰挿入側の処理が広域直線の差の主因と判断する。
中点の順序分岐省略にも追加の効果があった。命令別のprofileではないため、再帰とcompiler最適化の
どの命令が寄与したかまでは断定しない。

一方、再帰arena比で segment は約4%、narrow-segment は約8%遅くなった。
直線での大幅改善と単純な内部ループへの変更を重視して採用するが、全workloadの高速化は主張しない。
測定した全workloadで unique_ptr 比較候補以下の中央値となったものの、数%の差を安定した優位とは扱わない。
線分中心で座標先読みが可能なら、今回も最速だった座標圧縮版を優先できる。
