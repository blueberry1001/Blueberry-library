---
title: Cartesian Tree
documentation_of: //blueberry/graph/cartesian-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要

配列のinorder順を保つmin Cartesian treeの親配列を構築します。最小値が同値なら左端を根に選びます。Tの`<`はstrict weak orderingを満たす必要があります。

Nを要素数とすると、時間・メモリはO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`cartesian_tree`](#tree) | O(N) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `N <= INT_MAX`を満たしてください。計算量では比較をO(1)とします。
- 再帰・値の算術を使いません。入力を変更・保持せず、返り値は独立した所有配列です。
- ACLにはない線形木構築です。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/cartesian-tree.hpp"
int main() {
  auto p = blueberry::cartesian_tree(std::vector<int>{3, 1, 2});
  assert((p == std::vector<int>{1, 1, 1}));
}
```
{% endraw %}

<details class="api-operation" id="tree" markdown="1">
<summary><code>std::vector&lt;int&gt; cartesian_tree(const std::vector&lt;T&gt;&amp; values)</code> — O(N)</summary>

p[i] は頂点 i の親。根 r は p[r] == r。空入力は空配列、単要素は {0}。
根の自己辺を除けば通常の木の隣接リストへ変換可能。各頂点の push/pop は高々一回で全体最悪 O(N)、個々の挿入は複数 pop しうる。

{% raw %}
```cpp
auto p = blueberry::cartesian_tree(std::vector<int>{2, 2});
assert((p == std::vector<int>{0, 0}));
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

[Nyaan](https://nyaannyaan.github.io/library/tree/cartesian-tree.hpp.html) の単調スタックを調査し独立実装。
公式 Library Checker `cartesian_tree`、固定 seed の再帰最小値 oracle。候補比較は `docs/development/sequence-batch-two.md`。

</details>
