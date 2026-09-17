---
title: Cartesian Tree
documentation_of: //blueberry/graph/cartesian-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

配列の inorder 順を保つ min Cartesian tree の親配列。ACL にない線形木構築。
N は要素数で N ≤ INT_MAX。T の `<` は strict weak ordering、比較 O(1)。時間・メモリ O(N)。
最小値が同値なら左端を根に選ぶ。再帰・値の算術を使用しない。入力を変更・保持せず返り値は独立した所有配列。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `std::vector<int> cartesian_tree(const std::vector<T>& values)` | O(N) | [開く](#tree) |

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

## 出典・検証

[Nyaan](https://nyaannyaan.github.io/library/tree/cartesian-tree.hpp.html) の単調スタックを調査し独立実装。
公式 Library Checker `cartesian_tree`、固定 seed の再帰最小値 oracle。候補比較は `docs/development/sequence-batch-two.md`。
