---
title: 静的木・空間検索の比較測定
---

## 条件と再現方法

2026-09-19、WSL2 Linux x86-64 / Intel Core i7-14650HX、GCC 13.3.0、
`-std=gnu++20 -O2 -DNDEBUG`。同じ入力・同じプロセス内で各組を 5 回測定しました。
時間は構築と全クエリを合わせた壁時計時間であり、公式ジャッジの提出時間ではありません。
乱数 seed は 20260919。各組の checksum は全 5 回一致しました。

- RMQ: N=100000、Q=200000、乱数値と一様な区間。
- LCA: N=50000 の random-parent 木、Q=100000 の頂点対。
- 二次元: N=10000 の登録点、20000 回の一点加算と長方形和。

[測定ソース](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/static-tree-expansion/benchmark.cpp) と
[全サンプル・環境・実装 SHA-256](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/static-tree-expansion/results.log) を保存しています。
リポジトリルートで次のように実行できます。実装が変更された場合は別測定として扱ってください。

{% raw %}
```sh
g++ -std=gnu++20 -O2 -DNDEBUG -I. -I.deps/ac-library docs/development/measurements/static-tree-expansion/benchmark.cpp -o /tmp/static-tree-benchmark
/tmp/static-tree-benchmark
```
{% endraw %}

## 結果

| 実装 | 中央値 ms | 最小 ms |
| --- | ---: | ---: |
| LinearRMQ | 5.148 | 4.796 |
| SparseTable | 1.479 | 1.409 |
| LinearLCA | 5.564 | 4.953 |
| LowestCommonAncestorRMQ | 4.471 | 4.063 |
| KDTree | 53.215 | 52.557 |
| OfflineFenwickTree2D | 32.498 | 32.402 |

この条件では新規の 3 実装はいずれも比較対象より遅く、速度改善としては扱いません。
LinearRMQ / LinearLCA の追加理由は前計算・格納量が O(N) であることです。
KDTree は可換モノイドの一点代入・長方形集約を扱い、逆元が必要な二次元 Fenwick Tree と代数的な前提が異なります。
加算群で登録座標が事前に分かる本測定の用途では OfflineFenwickTree2D が有利でした。
メモリ量は実測しておらず、ここで述べたメモリの違いは理論計算量です。

本測定は一つの入力分布です。偏った長方形、木の形、N/Q 比、演算コストが変われば結果も変わります。
