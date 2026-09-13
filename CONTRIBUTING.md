# ライブラリ追加ガイド

## 方針

- ヘッダ単体でコンパイルできるよう、必要な標準ヘッダをすべてincludeする。
- 公開APIは原則として `blueberry` 名前空間に置く。
- 区間は原則として半開区間 `[l, r)` とする。
- 計算量、前提条件、境界条件をドキュメントに明記する。
- 追加した機能を実際に使用する `*.test.cpp` を必ず用意する。

## ファイルの対応

```text
blueberry/data-structure/example.hpp
docs/data-structure/example.md
verify/data-structure/example.test.cpp
```

ドキュメントのfront matterでは、リポジトリルートからの絶対パスを指定します。

```yaml
---
title: Example
documentation_of: //blueberry/data-structure/example.hpp
---
```

検証コードの先頭では対応する問題を指定します。

```cpp
#define PROBLEM "https://judge.yosupo.jp/problem/..."
```

## 確認コマンド

```console
make check
make verify
make docs
```

`make check` は公開ヘッダの単体include、全verifyコードのコンパイル、Pythonテスト、
ドキュメント例、`tests/random/**/*.cpp` のrandom testを実行します。愚直解との比較が有効な
ライブラリではrandom testも追加してください。seedは `argv[1]` または
`BLUEBERRY_RANDOM_SEED` から取得すると、CIで失敗したケースをそのまま再実行できます。

適切な公開問題が存在しない場合は、既存の問題をダミーとして独自テストを走らせる方法も
あります。ただし、可能な限り実際の制約を満たすLibrary Checkerの問題を使用します。
