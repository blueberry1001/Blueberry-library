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
ACLとの役割分担や実装・採用基準は[実装方針](IMPLEMENTATION_POLICY.md)にまとめています。

## ローカルでの検証

Python 3.9以上とC++20対応のGCCが必要です（CIはPython 3.12）。

```console
python3 -m pip install -r requirements-dev.txt
make test
make verify
make docs
```

`make verify` は `verify/**/*.test.cpp` を検出し、指定されたLibrary Checkerの
公式テストケースをダウンロードしてローカル実行します。verification-helperの標準仕様は
Yosupoへ自動提出する方式ではなく、同じテストデータとcheckerを使ってACかどうかを判定する方式です。

計測はコンパイル時間と解答の実行時間を分け、直列3回の実行合計の中央値を記録します。
毎回全verifyを再実行し、同じ測定条件の基準があれば増減率を表示します。
結果は `.build/verification/report.md`、ActionsのSummary・artifact、
公開サイトの[実行時間](https://blueberry1001.github.io/Blueberry-library/benchmarks.html)に掲載します。
ローカルの基準は `make benchmark-baseline` で保存・置換できます。
詳細な導入方法や注意点は[使い方ガイド](https://blueberry1001.github.io/Blueberry-library/guide.html)を参照してください。

## Yosupoへの提出

`online-judge-tools` のLibrary Checker向けアダプタは、現在テストケースの取得とローカル検証を
提供している一方、提出APIには対応していません。そのためGitHub Actionsからの自動提出は行わず、
必要な場合は `oj-bundle` で生成した単一ファイルを [Library Checker](https://judge.yosupo.jp/) の
提出画面から提出してください。

## ライブラリを追加する流れ

1. `blueberry/<category>/name.hpp` に自己完結したヘッダを追加する。
2. `templates/library.md.example` を使い、`docs/<category>/name.md` に全公開操作の
   呼び出し方・計算量と、開閉できる使用例・注意点を書く。
3. `verify/<category>/name.test.cpp` に `PROBLEM` と検証コードを書く。
4. `.verify-helper/docs/static/_data/libraries.yml` に登録し、
   `make test`（使用例をコンパイル・実行）、`make verify`、`make docs` を実行する。
5. pushするとGitHub Actionsが再検証し、`main` ならGitHub Pagesも更新する。

既存のルート直下ヘッダは旧構成との互換性のため残しており、順次、新構成への移行と
検証コードの追加を進めます。
