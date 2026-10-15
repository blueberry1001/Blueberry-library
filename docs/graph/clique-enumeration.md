---
title: Clique Enumeration
documentation_of: //blueberry/graph/clique-enumeration.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要

単純無向グラフの空でないクリークをすべて列挙します。自己ループ・重複辺は禁止です。

Nを頂点数、Mを辺数、Cをクリーク数とします。Dは次数と頂点番号の組で小さい側から辺を向けたときの最大出次数です。時間O((N+M)log(N+1) + C(D+1)² log(N+1))にcallbackの処理時間を加え、追加メモリO(N+M+D²)を使います。DはO(√M)ですが、Cは指数的に増加し得ます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`enumerate_cliques`](#enumerate) | O((N+M)log(N+1) + C(D+1)² log(N+1)) + callback |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- N,Mは非負のintの範囲内、頂点番号は`[0,N)`とします。
- 数値集計は行いません。集計時のオーバーフローはcallback側で管理してください。
- ACLにはありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/clique-enumeration.hpp"
int main() {
  int count = 0;
  blueberry::enumerate_cliques(3, {{0, 1}, {1, 2}, {0, 2}},
      [&](const std::vector<int>& vertices) { assert(!vertices.empty()); ++count; });
  assert(count == 7);
}
```
{% endraw %}

<details class="api-operation" id="enumerate" markdown="1">
<summary><code>enumerate_cliques(n, edges, emit)</code> — O((N+M)log(N+1) + C(D+1)² log(N+1)) + callback</summary>

`emit(const vector<int>& vertices)` を各非空クリークにつき一度呼びます。
単独頂点も含み、`n=0` は呼び出し0回、辺なしなら各単独頂点のみです。
列挙順とクリーク内の頂点順は保証しません。入力は変更しません。
callback に渡す参照・要素への参照はその呼び出し中だけ有効です。保存する場合はコピーしてください。
callback はグラフ入力を変更しないでください。例外は呼び出し元へ伝播します。
再帰深さは O(D+1)、計算量は償却ではなく最悪上界です。

{% raw %}
```cpp
std::vector<std::vector<int>> result;
blueberry::enumerate_cliques(2, {{0, 1}}, [&](const auto& c) { result.push_back(c); });
assert(result.size() == 3);
```
{% endraw %}

注意点: 上記の入力条件と寿命の制約を満たしてください。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

[Luzhiled](https://ei1333.github.io/library/graph/others/enumerate-cliques.hpp.html) の近傍部分集合列挙と
[maspypy](https://maspypy.github.io/library/enumerate/clique.hpp) の最小次数除去・DFSを比較調査。
行列を確保せず、次数による向き付けと候補集合の絞り込みを独立実装しました。外部コードの転載はありません。
[Library Checker](https://judge.yosupo.jp/problem/enumerate_cliques) と部分集合全探索比較で検証します。
候補の同一環境測定は [追加調査](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/graph-batch-four.md) に記載します。

</details>
