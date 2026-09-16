# 追加ライブラリの設計・測定記録（2026-09-14）

今回の対象は ACL にない Wavelet Matrix、Li Chao Tree、Manacher。
既存実装の置換ではなく、公式問題に対応する公開ライブラリを追加した。
研究対象のコードの転載は行わず、アルゴリズムから独立に実装した。

## Wavelet Matrix

[Luzhiled](https://ei1333.github.io/library/structure/wavelet/wavelet-matrix.hpp.html) の
固定幅・座標圧縮の設計と、[Nyaan](https://nyaannyaan.github.io/library/data-structure-2d/wavelet-matrix.hpp.html)
の64 bit rank構造を調査した。座標圧縮を自動化し、bit幅を指定せず負数・整数極値も扱う。
本体は標準C++20の `std::popcount` を利用し、CPU固有intrinsicを要求しない。

比較候補 `PrefixMatrix` は各要素ごとの整数prefix配列でrankを求める独立実装。
両候補とも同じ座標圧縮・段数・最悪 O(N log(N+1)) 構築 / O(log(σ+1)) 取得であり、
rankの保持方法を比較する。σ は異なる値の個数。

ソース: `benchmark/wavelet_matrix_benchmark.cpp`。
生ログ: `benchmark/results/wavelet-wsl-gcc13.txt`（全反復、checksum、CPU、OS、compiler）。
Intel Core i7-14650HX / WSL2 Ubuntu / GCC 13.3.0、
`-std=gnu++20 -O2 -Wall -Wextra -Werror -I.`。
seed 20260914、N=200000、Q=200000、各候補5回、実行順を交互にする。
明示的なウォームアップなし。入力・クエリの生成は計測外。
構築は入力コピー・sort・rank配列の生成まで、取得は全k番目クエリの実行まで。
各回で全取得値のchecksum一致を確認し、失敗なら非ゼロ終了する。

| 入力の値域 | 候補 | 構築 中央値 / 最小 ms | 取得 中央値 / 最小 ms |
| --- | --- | ---: | ---: |
| [0,16) | packed（採用） | 11.0291 / 10.7092 | 9.31962 / 9.06929 |
| [0,16) | integer prefix | 9.57768 / 9.36326 | 6.46282 / 6.42355 |
| [0,200000) | packed（採用） | 43.0327 / 42.7162 | 43.3443 / 42.1199 |
| [0,200000) | integer prefix | 38.4445 / 37.1304 | 81.7125 / 72.0935 |

rankのデータ量は1段あたりpackedが `12*(N/64+1)` bytes、integer prefixが
`4*(N+1)` bytes（いずれもvector管理領域を除く理論値、RSSではない）。
値域が大きい入力での取得速度とrankメモリ量からpackedを採用する。
小さい値域ではinteger prefixが速く、構築も両分布でinteger prefixが速かった。
小値域や構築のみの用途での一律な高速性は主張しない。
測定中は環境構築や他のテストも進行しており、CPU専有状態ではない。
同一マシン内の候補比較であり、微小な差やLibrary Checkerの表示時間とは比較しない。

再現（WSL/Linux、リポジトリルート）:

```bash
mkdir -p .build
g++ -std=gnu++20 -O2 -Wall -Wextra -Werror -I. benchmark/wavelet_matrix_benchmark.cpp -o .build/wavelet-benchmark
.build/wavelet-benchmark
```

## Li Chao Tree

[詳細な比較・再現方法](li-chao-tree.md)、
[全反復ログ](results/li-chao-tree-2026-09-14.json)。
座標圧縮配列版と動的整数領域版を比較した。線分追加では配列版が有利だが、
直線のみのworkloadでは動的版が速い結果も保持している。

## Manacher

[API・比較・再現方法](../docs/string/manacher.md)、
[全反復ログ](results/manacher-wsl-gcc13.txt)。
奇数・偶数の2回走査と区切り配列を構築する方式、愚直展開を比較した。
入力に区切り値を要求せず、最悪 O(N)、結果以外 O(1) メモリの2回走査を採用した。

## 検証記録

- GCC 13.3.0 / Clang 18.1.3、GNU C++20で公開・互換入口16ヘッダとverify25本をコンパイル。
- 各コンパイラで `tests/random/` の4プログラムをseed 1〜20で実行。新規アルゴリズム3件とrunnerの確認が成功。
- 新規3件のASan/UBSanによる境界・乱択比較が成功。
- `make test`: Pythonテスト23件、ライブラリ14ページの形式検査、実行可能な最小例15件が成功。
- 公式verifyは25本・534ケースを各3回、合計1,602回のACを確認。
  最初の `make verify` は19本成功後に既定1500秒の全体上限で終了し、残り6本は `not_run`。
  残りだけを同じ環境・flags・3反復で `scripts/verify_with_metrics.py --timeout 3600` により検証した。
  単一の `make verify` が正常終了したという意味ではない。
- 初回の結果は `.verification/first-batch.json` と `make-verify.log`、残りは
  `.verification/remaining/current.json` と `remaining.log` に保持した。
  `<cstddef>` の明示と見出しコメント確定後は依存する5本を再実行し、
  `.verification/final-headers/` に保存した最終コードの実測値を採用した。
  `.verification/combine_results.py` で環境の一致、対象集合の一致、全生ログのAC、3反復を検査し、
  実測値を変更せず `.verification/current.json` と `report.md` にまとめた。
  集計JSONには元の3レポートへの参照を含めている。計測はcommit前の作業ツリーで実行した。
- 実行時間は基準なしの初回計測として扱い、既存ライブラリの高速化を主張しない。
