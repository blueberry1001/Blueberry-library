---
title: Linear LCA 構築時の確保量の比較
---

# Linear LCA 構築時の確保量の比較

連結無向木の Euler tour は `2N-1` 要素になる。構築中に増える頂点列と深さ列にこの容量を確保し、再確保を減らす。公開 API、線形空間、問い合わせ時の処理は変わらない。スタックには木の形によって最大使用量に差があるため、今回の変更では容量を指定していない。

2026-11-17、Windows x86-64、MSVC 14.43.34808、`/std:c++20 /EHsc /O2 /DNDEBUG`。N=500,000、seed=1729、各200,000クエリ。各条件で1回の warmup と7回の測定を行い、旧版と変更候補の実行順を交互にした。入力生成は計時外。各サンプルで結果 checksum の一致を確認した。以下は中央値 / 最小値、単位 ms。

| 木 | 版 | 構築 | クエリ |
| --- | --- | ---: | ---: |
| random-parent | 旧 | 109.568 / 95.668 | 21.394 / 17.148 |
| random-parent | 確保 | 104.638 / 80.777 | 18.409 / 15.389 |
| path | 旧 | 27.284 / 24.259 | 15.601 / 11.514 |
| path | 確保 | 21.805 / 19.419 | 16.649 / 12.466 |
| star | 旧 | 29.381 / 21.347 | 25.184 / 20.945 |
| star | 確保 | 21.220 / 16.981 | 25.586 / 22.285 |

実行時間のばらつきは大きく、特に random-parent の差を精密な改善率とは扱わない。RSS、I/O、GCC、Clang は未測定。クエリ処理は同一であり、時間差をクエリ高速化とはみなさない。公開最速提出との同一条件の比較ではなく、旧実装と確保候補の比較である。

[全サンプル・コンパイラ引数・ハッシュ](measurements/linear-lca-reserve-20261117/msvc.json)、[変更前ヘッダ](measurements/linear-lca-reserve-20261117/linear-lca-before.hpp.txt)、[測定時スクリプト](measurements/linear-lca-reserve-20261117/run-snapshot.py.txt)。再測定には `python benchmark/run_linear_lca_reserve.py --output <新しい出力先>` を使用する。変更前の保存ヘッダから旧版と候補を生成する。

記録 JSON のハッシュは実際に読み込んだ Windows 上のバイト列を対象とする。元ヘッダは CRLF で SHA-256 `b3a3f69a3eabced4f05a14450f8636a5c7ee4e812580d44ee2e9ead44deb8a2b`、改行を LF に正規化すると `378e0914a727f09325d9770ef3bce28f09b1f608634835bfeb67fd6249fdbb99`。保存ヘッダはこの LF 正規化版で、内容の差は改行形式だけである。checkout 設定で改行が変わる場合は LF 正規化後に照合する。

変更後の `tests/random/static-tree-expansion.cpp` は Windows Clang 18 の `gnu++20` / `gnu++23`、assert 有効 / `-DNDEBUG` の4構成で、seed 1・2・17 の全12実行に成功した。[コンパイルと実行結果](measurements/linear-lca-reserve-20261117/validation.json)。Library Checker の公式ケースと GCC はこの測定環境では未実行。
