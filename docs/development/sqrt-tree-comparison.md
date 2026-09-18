---
title: Sqrt Tree と Disjoint Sparse Table の比較
---

## 設計

静的モノイド積を対象に、既存の Disjoint Sparse Table と Sqrt Tree を比較した。
両方とも非可換演算に対応し、クエリは O(1)。前計算・メモリはそれぞれ
O(N log N) と O(N log log N)。Sqrt Tree は ACL にない静的構造として追加する。

[CP-Algorithms の実装・解説](https://cp-algorithms.com/data_structures/sqrt-tree.html)と
[AtCoderInfo の解説](https://info.atcoder.jp/entry/algorithm_lectures/sqrt_tree)を調査した。
prefix/suffix と全ブロック間積を層ごとに作る方針を採り、更新を行うための補助構造は省いた。
既存 DST も比較候補とし、公開コードはコピーせずに実装した。

各層の親の長さを `2^k`、子を `2^ceil(k/2)` とする。
子の個数の二乗は親の長さ以下になるため、ブロック間積も各層 O(N) 個で収まる。
左右端の XOR の最上位ビットから層を直接引き、suffix・between・prefix を最大 2 回の
`op` で結合する。単位元による 2 冪へのパディングは最大で元の長さの約 2 倍になる。

## 同一環境の測定

2026-09-18、WSL2 / Intel Core i7-14650HX / GCC 13.3.0、
`-std=gnu++20 -O2 -Wall -Wextra -I.`。
`benchmarks/sqrt-tree.cpp` で unsigned 64-bit sum、各 N に対し同一の乱数配列・
同一の非空区間 500,000 個を使用。seed=20260918、5 回、実行順を交互にした。
明示的な warmup はせず初回を含めた。コンパイル時間は含まない。
すべての対応する checksum は一致。並列作業と共有 OS の負荷による変動は残る。

単位はミリ秒、各セルは **中央値 / 最小値**。

| N | 構造 | 構築 | 500,000 クエリ |
| --- | --- | --- | --- |
| 65,536 | Sqrt Tree | 0.514 / 0.472 | 1.698 / 1.615 |
| 65,536 | DST | 1.550 / 1.481 | 2.490 / 1.632 |
| 65,537 | Sqrt Tree | 3.822 / 3.126 | 2.165 / 2.033 |
| 65,537 | DST | 2.468 / 1.588 | 2.003 / 1.653 |
| 262,144 | Sqrt Tree | 10.095 / 9.520 | 4.794 / 4.620 |
| 262,144 | DST | 14.640 / 14.391 | 4.864 / 4.381 |

Sqrt Tree の構築は 2 冪ちょうどでは有利だったが、その直後ではパディングと層数の増加に
より不利になった。クエリの差も一貫しないため、DST の置換ではなく選択肢として提供する。
メモリ量の実測は未実施。式からの比較でも、N=65,537 では Sqrt Tree の
パディングのため、漸近計算量だけから DST より省メモリとは判断できない。
冪等な RMQ に専用化した Sparse Table と sum を直接比較することはしない。

生ログと環境・ソースハッシュ:

- [results.csv](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/sqrt-tree/results.csv)
- [environment.txt](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/sqrt-tree/environment.txt)
- [sources.sha256](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/sqrt-tree/sources.sha256)

再現:

{% raw %}
```sh
g++ -std=gnu++20 -O2 -Wall -Wextra -I. benchmarks/sqrt-tree.cpp -o /tmp/sqrt-bench
/tmp/sqrt-bench
```
{% endraw %}

## 検証

`tests/random/sqrt-tree.cpp` で空配列、1/2 要素、各層の 2 冪境界の前後、
非可換 affine 合成を愚直解と比較した。GCC の gnu++20/23、Clang の gnu++20/23、
Clang ASan/UBSan の focused test を実行。既定コンストラクタのない要素型も検証する。
公式 Static Range Sum / Static RMQ 向け verify driver を追加し、コンパイル確認した。
この測定レポートだけでは公式テストの AC を主張しない。
