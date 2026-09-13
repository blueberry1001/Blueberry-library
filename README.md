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

## ローカルでの検証

Python 3.8以上とC++20対応のGCCまたはClangが必要です。初回だけ `make setup` を実行すると、
verification-helperとACLを準備します。

```console
make setup
make check
make verify
make docs
```

各コマンドの役割は次の通りです。

| コマンド | 内容 |
| --- | --- |
| `make check` | Pythonテスト、ドキュメント例、compile test、random testをまとめて実行 |
| `make include-test` | 公開カタログの全ヘッダと互換入口を、1個ずつ単独includeしてコンパイル |
| `make verify-compile` | `verify/**/*.test.cpp` を公式ケースの取得なしで全件コンパイル |
| `make random-test` | `tests/random/**/*.cpp` をコンパイルし、既定でseed 1から20回実行 |
| `make verify` | Library Checker公式ケースをchecker付きで実行し、3回の中央値を記録 |

コンパイラと標準は、例えば
`make compile-test CXX=clang++ CXX_STANDARD=gnu++23` のように切り替えられます。
ACLを別の場所に置く場合は `ACL_ROOT=/path/to/ac-library` を指定してください。

random testはseedを `argv[1]` と `BLUEBERRY_RANDOM_SEED` の両方で受け取れます。
失敗時に表示されたseedは
`RANDOM_SEED=123 RANDOM_RUNS=1 make random-test` で再現できます。配置規約と最小例は
[`tests/random/README.md`](tests/random/README.md)にあります。

`make verify` は `verify/**/*.test.cpp` を検出し、指定されたLibrary Checkerの公式テストケースを
ダウンロードしてローカル実行します。verification-helperの標準仕様はYosupoへ自動提出する方式ではなく、
同じテストデータとcheckerを使ってACかどうかを判定する方式です。

## CIで行う検証

pushとpull requestでは、GCC/Clang × GNU C++20/23の4環境で、全公開ヘッダの単体include、
全verifyコードのコンパイル、共通random testを実行します。これにすべて通った後、GCC・GNU C++20で
Pythonテスト、ドキュメント例、Library Checker公式ケース、生成サイトを検証します。
各compile jobの結果はGitHub ActionsのSummaryに件数付きで表示され、Library Checkerの詳細な
実行結果と計測ログは `verification-metrics` artifactに保存されます。

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
