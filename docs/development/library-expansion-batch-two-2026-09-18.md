# ライブラリ追加第2弾 — 2026-09-18

基準は main `2207ff8`。既存APIを維持し、ACLにない12ライブラリを追加する。
カタログは42→54ヘッダ、公式verifyドライバは68→81。
Library Checker対応表の「実装＋verifyあり」は58→70問題（AOJは別枠）。

| 分類 | 追加ライブラリ | 公式問題 |
| --- | --- | --- |
| Data Structure | Disjoint Sparse Table | Static Range Sum（非可換積は乱択比較） |
| Graph | Cartesian Tree | Cartesian Tree |
| Graph | Cycle Detection（有向・無向） | Cycle Detection / Cycle Detection (Undirected) |
| Graph | Topological Sort | 直接対応なし。推移閉包・順序制約との比較 |
| Graph | Tree Diameter | Tree Diameter |
| Graph | Triangle Enumeration | Enumerate Triangles |
| Math | Divisor Convolution（約数・倍数変換、gcd/lcm畳み込み） | GCD Convolution / LCM Convolution |
| Math | Enumerate Quotients | Enumerate Quotients |
| Math | Kth Root Integer | Kth Root (Integer) |
| Math | Discrete Logarithm | Discrete Logarithm |
| Math | Longest Increasing Subsequence | Longest Increasing Subsequence |
| String | Prefix Function / KMP | AOJ ALDS1_14_B |

各ヘッダのAPIページに入力条件・空入力・計算量・返却値を記載した。
新しい公開フィールドはなく、結果は所有権を持つvector・pair・optionalまたは値で返す。
三角形列挙は同期コールバックに渡すため、全出力を内部に保存しない。

## 選択と制約

- Disjoint Sparse Tableは結合則だけを要求し、非可換な空でない区間積をO(1)で返す。
  既存Sparse Tableの冪等演算向けAPIと併存する。構築・メモリはO(N log N)。
- 閉路検出・木の直径は非再帰。閉路検出は自己ループと多重辺を受け付けるが、
  三角形列挙は単純グラフ、木の直径は非負重みの連結木を要求する。
- 離散対数は合成数法を扱い、最小非負指数を返す。法はINT32_MAX以下。
  ACLの逆元を使い、標準のmodint・畳み込みなどは再実装しない。
- 整数k乗根はuint64全域に対応し、比較前の除算で積のoverflowを避ける。
- LISは狭義増加で添字を復元し、Cartesian Treeは同値の最小値を左優先にする。
  KMPは重複一致と空パターン（全境界に一致）を定義する。

## 検証記録

- `make check`: Python 45件、54ライブラリ文書・58実行例、55ヘッダ、81公式ドライバのコンパイル、
  22乱択テスト各20seedが成功。
- 新規13公式ドライバの分野別実行は全件成功（各ケース1回）。KMPのAOJ以外はLibrary Checker。
- GCC/Clang × C++20/23の55ヘッダ・81ドライバのコンパイル、22乱択テスト各20seed、
  releaseヘッダ検査が成功。新規3分野のASan/UBSanもseed=23で成功。
- 担当分野を交換して数学・列・グラフの実装を独立レビューし、統合担当も全12ヘッダと公開APIを確認した。

## 調査・比較測定

- [グラフの選択と生ログ](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/graph-batch-two.md)
- [数学の選択と生ログ](math-batch-two.md)
- [列・文字列の選択と生ログ](sequence-batch-two.md)

比較候補は同一入力・GCC・フラグで独立実装して複数回計測した。
再帰の深さ、漸近計算量、メモリとAPIの条件も合わせて採用を判断している。
共有ホストで他の検証が並走しており、小さい実測差を一般的な優位とは主張しない。

## ログの扱い

作業中のコマンドログは `.build/batch-two-*.log`、分野別公式測定は
`.verification/batch-two-{graph,math,sequence}/` に保存する。
全体の公式測定は `.verification/current.json` と対応するrunディレクトリを使う。

初回の `.build/batch-two-baseline.log` は実装作業と並走し、一括入口へ登録した時点で
まだ作成されていなかった新ヘッダのincludeが1件失敗した。この実行を変更前の
完全なbaseline成功とは扱わない。ヘッダ作成後の統合コンパイルは
`.build/batch-two-compile-preflight.log` に分けて記録した。

公式問題対応表はローカルドライバの存在を表す。AC判定は公式checkerの実行結果を参照し、
未実行や失敗した計測を0秒や成功へ置き換えない。
