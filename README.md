# Blueberry Library

[![verify](https://github.com/blueberry1001/Blueberry-library/actions/workflows/verify.yml/badge.svg)](https://github.com/blueberry1001/Blueberry-library/actions/workflows/verify.yml)
[![documentation](https://img.shields.io/badge/docs-GitHub%20Pages-0969da?logo=github)](https://blueberry1001.github.io/Blueberry-library/)

C++20向けの競技プログラミング用ライブラリです。`blueberry/` 以下のヘッダは、
[Library Checker](https://judge.yosupo.jp/) のテストケースを利用して
[verification-helper](https://github.com/online-judge-tools/verification-helper) で継続的に検証します。

## 使い方

リポジトリのルートをインクルードパスに追加し、必要なヘッダだけを読み込みます。

```cpp
#include "blueberry/data-structure/disjoint-set-union.hpp"

blueberry::DisjointSetUnion dsu(n);
dsu.merge(u, v);
```

まとめて読み込む場合は `#include "blueberry/all.hpp"` を使用できます。
提出用の単一ファイルは `oj-bundle main.cpp -I .` で生成できます。

## 収録ライブラリ

| 分類 | ライブラリ |
| --- | --- |
| Data Structure | Disjoint Set Union, Fenwick Tree, Segment Tree, Sparse Table, Rollback Union Find |
| Graph | Dijkstra, Lowest Common Ancestor, Heavy-Light Decomposition |
| Math | Formal Power Series, Prime Sieve |
| String | Z Algorithm |

詳細な説明・計算量・検証コードは[ドキュメント](https://blueberry1001.github.io/Blueberry-library/)に掲載します。
ACLとの役割分担や実装・採用基準は[実装方針](IMPLEMENTATION_POLICY.md)にまとめています。

形式的冪級数は `#include "blueberry/math/formal-power-series.hpp"` で読み込みます。
`atcoder::modint998244353` などのACL `static_modint`を係数型に指定し、`inv`・`log`・
`exp`・`sqrt`・`pow`を利用できます。一般的な多項式積はACLの`convolution`を使い、
FPS固有のNewton反復だけをBlueberry側で提供します。
FPSを使う場合は、ACLの `atcoder/` ヘッダをインクルードパスに配置してください。
GitHub Actionsでは公式ACLを取得してから検証します。

LCA が大量に必要な場合は `LowestCommonAncestorRMQ`（Euler tour + Sparse Table、クエリ O(1)）を、
部分木・パスクエリには `HeavyLightDecomposition` を利用できます。後者は ACL の `segtree` / `lazy_segtree`
を分解配列の区間に適用する設計です。

区間積の新しい呼び出しは `SparseTable::prod` / `SegmentTree::prod`（全体は `all_prod`）を推奨します。
既存の `product` / `all_product` も互換性のため残しています。Fenwick Tree のprefix取得には短い
`pref`、Union Find の成分サイズには `comp_size` も使えます。

## ローカルでの検証

Python 3.8以上とC++20対応のGCCが必要です。

```console
python3 -m pip install -r requirements-dev.txt
make verify
make docs
```

Fastest 提出との比較を行うときは、公開 REST API を読み取るスクリプトも利用できます。

```console
python3 scripts/fetch_lc_fastest.py exp_of_formal_power_series --limit 10 \
  --out-dir .verification/lc/exp --save-source
```

`make verify` は `verify/**/*.test.cpp` を検出し、指定されたLibrary Checkerの
公式テストケースをダウンロードしてローカル実行します。verification-helperの標準仕様は
Yosupoへ自動提出する方式ではなく、同じテストデータとcheckerを使ってACかどうかを判定する方式です。

## Yosupoへの提出

`online-judge-tools` のLibrary Checker向けアダプタは、現在テストケースの取得とローカル検証を
提供している一方、提出APIには対応していません。そのためGitHub Actionsからの自動提出は行わず、
必要な場合は `oj-bundle` で生成した単一ファイルを [Library Checker](https://judge.yosupo.jp/) の
提出画面から提出してください。

## ライブラリを追加する流れ

1. `blueberry/<category>/name.hpp` に自己完結したヘッダを追加する。
2. `docs/<category>/name.md` に仕様と計算量を書く。
3. `verify/<category>/name.test.cpp` に `PROBLEM` と検証コードを書く。
4. `make verify` と `make docs` を実行する。
5. pushするとGitHub Actionsが再検証し、`main` ならGitHub Pagesも更新する。

既存のルート直下ヘッダは旧構成との互換性のため残しており、順次、新構成への移行と
検証コードの追加を進めます。
