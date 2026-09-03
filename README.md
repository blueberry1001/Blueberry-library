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
| Graph | Dijkstra, Lowest Common Ancestor |
| Math | Prime Sieve |
| String | Z Algorithm |

詳細な説明・計算量・検証コードは[ドキュメント](https://blueberry1001.github.io/Blueberry-library/)に掲載します。

## ローカルでの検証

Python 3.8以上とC++20対応のGCCが必要です。

```console
python3 -m pip install -r requirements-dev.txt
make verify
make docs
```

`make verify` は `verify/**/*.test.cpp` を検出し、指定されたLibrary Checkerの
公式テストケースをダウンロードしてローカル実行します。これは
`judge.yosupo.jp` へコードを実際に提出する方式ではありませんが、同じテストデータと
checkerを用いてACかどうかを判定します。

## ライブラリを追加する流れ

1. `blueberry/<category>/name.hpp` に自己完結したヘッダを追加する。
2. `docs/<category>/name.md` に仕様と計算量を書く。
3. `verify/<category>/name.test.cpp` に `PROBLEM` と検証コードを書く。
4. `make verify` と `make docs` を実行する。
5. pushするとGitHub Actionsが再検証し、`main` ならGitHub Pagesも更新する。

既存のルート直下ヘッダは旧構成との互換性のため残しており、順次、新構成への移行と
検証コードの追加を進めます。
