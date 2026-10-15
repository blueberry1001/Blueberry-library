---
layout: page
title: 使い方
---

# 使い方

## 必要な機能を探す

| 探し方 | 入口 |
| --- | --- |
| 名前・用途が分かる | [ライブラリ一覧]({{ '/' | relative_url }})で検索 |
| 更新・問い合わせから選びたい | [操作から探す]({{ '/operations.html' | relative_url }}) |
| ACL用の演算を組みたい | [モノイド集]({{ '/monoids.html' | relative_url }}) |
| 問題に対応する実装を知りたい | [LC対応表]({{ '/library-checker.html' | relative_url }}) |

個別ページの「使用例」で最小の使い方を確認できます。操作名を開くと、引数・返り値・計算量・注意点が読めます。

## 共通の約束

| 項目 | 約束 |
| --- | --- |
| 名前空間 | `blueberry`。ACLは `atcoder` |
| 添字・頂点 | 0-indexed |
| 区間 | 原則として半開区間 `[l, r)` |
| 計算量 | N, V, Eなどの意味は各ページの概要に記載 |
| 型の演算 | 特記がなければ比較・加算・結合演算を O(1) と仮定 |
| 償却計算量 | 1回の最悪値でなく、操作列全体で平均した上界 |
| assert | 前提違反を検出する補助。不正な入力を許す仕組みではない |

空区間・空配列が使えるかは各操作の説明を確認してください。
`long long` でもオーバーフローする入力は扱えません。

## ローカルで使用したい場合

{% raw %}
```console
git clone https://github.com/blueberry1001/Blueberry-library.git
g++ -std=gnu++20 -O2 -Wall -Wextra -I ./Blueberry-library main.cpp -o main
```
{% endraw %}

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/rollback-union-find.hpp"

int main() {
  blueberry::RollbackUnionFind uf(3);
  int saved = uf.state();
  uf.merge(0, 1);
  assert(uf.same(0, 1));
  uf.rollback(saved);
  assert(!uf.same(0, 1));
}
```
{% endraw %}

全体を読み込む `blueberry/all.hpp` や、FPSなどACLに依存するヘッダには、別途ACLが必要です。
ACLのルートも `-I` に追加してください。ACL本体は同梱していません。

提出先で外部ヘッダを使えないときは、`oj-bundle`で単一ファイルにします。

{% raw %}
```console
oj-bundle main.cpp -I ./Blueberry-library > submission.cpp
g++ -std=gnu++20 -O2 submission.cpp -o submission
```
{% endraw %}

ACLも展開するときはACLのルートも `-I` で指定します。生成後のコードもコンパイルしてください。
