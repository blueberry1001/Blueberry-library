# Weighted Wavelet Matrix の採用比較

2026-09-18。対象は値列を固定した一点重み加算と添字区間・値域の総和。

## 設計調査

- [Luzhiled](https://ei1333.github.io/library/structure/wavelet/wavelet-matrix-rectangle-sum.hpp.html) は各段に静的累積和を保持し、矩形和を値の bit 数に比例する時間で処理する。重み更新は扱わない。
- [maspypy](https://maspypy.github.io/library/ds/wavelet_matrix/wavelet_matrix_2d_range.hpp) は x 順に並べた点群を WM と可換モノイドの補助構造へ渡し、点の重み更新も行う。
- Blueberry の `OfflineFenwickTree2D` は各外側 Fenwick ノードで座標二分探索を行う。座標で直接更新できる一方、本用途では元の添字から各 WM 段へ rank だけで移動できる。

外部コードは移植せず、既存 WaveletMatrix の圧縮・bit rank と FenwickTree の線形構築を組み合わせた。
動的な値の変更は対象外。更新しない場合にも利用できるが、静的累積和 WM に比べ総和クエリに log N の追加係数を要する。
各段 O(N) 個の重みと約 N bit の rank 情報を持つため O(N log S) メモリ。
OfflineFenwickTree2D は O(N log N) 個の重みと内側座標を持つ。定数や重複に依存するため実測メモリ比較は未実施。

## 同条件の実測

ソースは `benchmarks/weighted-wavelet-matrix.cpp`、生ログは [benchmark.txt](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/weighted-wavelet-matrix/benchmark.txt)。
WSL2、GCC 13.3、`-std=gnu++20 -O2`、N=40000、Q=40000、seed=20260918、3 回。
点の x は元添字、y は乱数。初期重み 1、クエリは一点加算と値域・添字区間和を半数ずつ。
同一プロセスで同じ事前生成入力を使い、初期重み投入も構築時間へ含めた。

| 実装 | 構築 median / min (ms) | 操作 median / min (ms) | checksum |
| --- | --- | --- | --- |
| WeightedWaveletMatrix | 8.703 / 8.486 | 20.318 / 19.258 | 111607015 |
| OfflineFenwickTree2D | 28.669 / 28.217 | 44.759 / 44.739 | 111607015 |

この入力では構築・操作とも短縮した。単一分布の測定であり、全用途での優位を主張しない。
公開ライブラリ同士の実測比較ではなく、参考設計を踏まえた新実装と既存の実用上の代替手段の比較。

## 検証

GCC 13.3 / Clang、gnu++20 / gnu++23 の 4 組合せでランダム比較を実行。
空列、全同値、63/64/65/128/129 要素、signed long long 極値、負重み、空範囲を含む。
Library Checker の Point Add Rectangle Sum 向け verifier を追加し GCC20 でコンパイル確認。
公式ケースの結果は統合時の verify ログを参照する。
