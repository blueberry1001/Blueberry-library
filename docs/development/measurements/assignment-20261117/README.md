> 測定原本・各試行・失敗ログ・再現スクリプトは、バイト単位で `reproducibility.tar.gz` に収録しています。このディレクトリで `tar -xzf reproducibility.tar.gz` を実行すると展開でき、README も元の内容に戻ります。

# Assignment の高速化調査

既存 API の符号付き整数・負費用・矩形・空入力・所有する返却値を維持し、幅広整数で安全に計算できる場合に限り高速経路を使う。正方は列初期化と最短増加路による LAPJV、矩形は各行の最小値による初期マッチと最短増加路を使う。正方の Monge / 逆 Monge 行列は O(N²) で対角 / 逆対角を返す。同点解の選択は従来どおり未規定。

## 調査対象と契約

- [Library Checker 提出 #410724](https://judge.yosupo.jp/submission/410724) は調査時に 4 ms の最速提出。BSD 2-Clause の gatagat/lap 派生で、正方行列・32-bit 入力・64-bit 中間値に限定し、Monge 判定、列ごとの初期化、2回の行 reduction と LAPJV を用いる。比較用の該当関数を `fastest-410724.hpp` にライセンス全文付きで保存した。I/O や pragma は比較用 driver に合わせている。
- 正方高速経路はこの提出を幅広型と行ベクトル入力に合わせて変更。矩形高速経路は [KACTL WeightedMatching](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/WeightedMatching.h)（CC0、保存原本 `WeightedMatching.reference.txt`）の最短増加路を参考にし、同じ最小費用の未使用列を選ぶ初期マッチを追加した。最速提出そのものは矩形や任意型を扱わない。
- 旧版と同じ `T` で新方式を計算する案は overflow 条件を維持できないため棄却。例えば矩形の `[[82,6,28,57,33],[6,-46,14,51,23],[61,-44,-64,1,49]] * 16000000` は旧式の全中間値が int32 に収まるが、旧候補の新式では `2000000000+448000000` が overflow する。後述の実装は幅広型を使い、値域に合わない場合は旧版へ戻す。
- 符号付き組み込み整数かつ N≤4096 のとき、32-bit 以下は int64 中間値、64-bit 型は入力がすべて int32 範囲内なら int64、そうでなければ `__int128` があれば同型を使う。該当しない場合、unsigned・浮動小数・独自型は旧 Hungarian 実装を使う。戻り値の総和も幅広型で計算してから T に戻す。

幅の根拠として、B=max|cost| とする。列価格の初期値は [-B,B] で、処理中は非増加。未完了時の未使用列は初期価格のままであり、マッチ行の双対実行可能性から他列の価格は通常 [-3B,B] に収まる。残余距離の増加は定数倍 B、単純増加路の長さは高々 N。中間和を保守的に `16(N+1)²(B+1)` と見ても、N≤4096 で int32 全域は 2^60 未満、int64 全域は 2^92 未満で、番兵 2^62 / 2^126 より十分小さい。初期列の処理だけで全行がマッチした場合は列最小値和の下界を達成しているため、価格調整より前に返す。この上界は標準的な primal-dual 不変条件に基づく評価であり、形式証明ではない。

## 操作単体の比較

Windows 11、Intel Family 6 Model 183、Windows Clang 18.0.3 / MSVC 19.43、C++20、`-O2 -DNDEBUG` 相当。入力生成と I/O は計時外、各 solver 呼び出し内の初期化・配列確保は計時内。子プロセスを同じ CPU 論理コアへ固定し、warmup 1回＋5回、順番を入れ替えた中央値。値は ms、各ケースで2回ずつ解いている。`results-v10.json` の raw sample を参照。

| 入力・Clang | 旧版 | 新版 | 最速提出 |
| --- | ---: | ---: | ---: |
| 250×250 一様乱数 | 8.540 | 0.676 | 0.595 |
| 250×250 値域 {-1,0,1} | 27.319 | 0.232 | 0.191 |
| 250×250 行列和型 | 25.734 | 0.223 | 0.143 |
| 125×500 値域 {-1,0,1} | 11.392 | 0.562 | 対象外 |

最速提出の driver は平坦な int32 を予め用意し、旧版・新版は元の `vector<vector<long long>>` を使用する。値は同じでも型・配置・契約は異なるため、各版の実装全体の比較であり演算自体の完全同条件比較ではない。CPU固定は他プロセスの干渉を排除しない。実際に同形状の別実験では旧版の中央値が約4～9 msと変動しており、小差は確定的な優劣と扱わない。

非 Monge の対角優位入力では列ごとの最小をすべて異なる行に割り当てられる。完備マッチを早期 return する版は、250×250・2回の中央値で Clang 0.159→0.107 ms、MSVC 0.245→0.158 ms。`results-v10.json` の kind=5。kind=6 は対角が一意に最小の乱数行列。

## 公式入力の実行全体

問題 repo commit `73876f1cbcf082d2b22722924b924d3d65a2f00e` から生成した全14入力は、それぞれ公式 `hash.json` の入力 SHA-256 と一致。公式 `sol/correct.cpp` の最小費用と、旧版・新版・最速提出の出力について置換と費用を検算した。Windows Clang、起動・I/O・構築・操作・出力を含む warmup 1回＋3回中央値。最速提出にも共通入力から int32 平坦配列への変換を含めた。単位 ms。

| 入力 | 旧版 | 新版 | 最速提出 |
| --- | ---: | ---: | ---: |
| max_random_00 | 149.48 | 124.24 | 131.69 |
| max_random_03 | 122.92 | 113.92 | 114.30 |
| hand_plus_00 | 180.71 | 90.55 | 91.36 |
| multiplication_table_00 | 187.31 | 87.94 | 92.02 |

起動・I/O・同時負荷の影響がある。例として random_00 は旧 21.24、新 24.34、最速 20.88 ms で、回帰と誤差を切り分けられていない。全ケース、コマンド、各回、hash、失敗は `official-results-v2.json`。競技環境の実行時間を保証しない。

## 正しさと未測定

固定 seed の小さい正方・矩形について全列挙し、複数の同点、負値、Monge / 逆 Monge、元版との整数 overflow 反例、int64 の大きな値、浮動小数・独自型・空入力を検証。公開版テストは Windows Clang / MSVC × C++20 / C++23 × assert 有効・無効 × seed 1,2,17 で成功。Clang は `-Wall -Wextra -Wshadow -Werror` でも成功。幅広型・浮動小数・独自型の追加比較も assert 有効・無効の計8構成で成功。`publish-test-results-v5.json` と `validation-early-results.json` を参照。公開ヘッダでは、測定原本から namespace を変更し、利用者の前提を繰り返すだけの assert を除去し、`move` を修飾した。`release-header.hpp.txt` に公開ヘッダの保存原本を置く。測定は `-DNDEBUG` であり、削除した assert は測定に含まれない。

独立レビューは `jv.hpp` SHA-256 `9528ffe4641db85632b1d00dd212e3b84022aef319802741a02b60ce2c3f6870` について 3×3三値・4×4二値・3×4三値の全列挙と符号・同値・64bit極値の乱択、debug/release 各645,573件を brute force 比較。早期 return 追加後の `jv-early.hpp` SHA-256 `0f127dd1cca297d05655612d0a9cc1630cd4e7e5b8a21ebec7722a6e14092beb` も 3×3三値19,683件・4×4二値65,536件と境界を debug/release で再照合。ログと driver は `independent/` に保存。Linux GCC・sanitizer・メモリ使用量はこの測定では未実施。

`results-v9.json` は失敗記録で、試作 driver の乱数生成で unsigned の減算が巻き戻り、int32 専用の比較対象へ範囲外の値を渡したために費用が不一致になった。型変換を修正した `driver-v10.cpp` と `results-v10.json` で再測定した。`results-v7.json` の `safe.hpp` hash は実験当時の `safe-v1.hpp.txt` と一致する。棄却案・失敗ログも保存している。

再現時はこのフォルダを別の書き込み可能な作業ディレクトリにコピーし、`python run-v10.py`、`python run-validation-early.py` を実行する。`run-official-v2.py` は固定版の問題 repo と Windows の Visual Studio / SDK / Clang パスを必要とし、環境に合わせて変更する。元ログは上書きせず、別ディレクトリで再実行する。
