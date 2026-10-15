---
title: Complement Graph Components
documentation_of: //blueberry/graph/complement-components.hpp
---

[カテゴリへ戻る]({{ "/categories/graph.html" | relative_url }})

## 概要

無向グラフの補グラフを明示的に構築せず連結成分を列挙します。隣接リストは両方向に登録します。自己ループ・重複辺は無視します。

Nを頂点数、Mを隣接リストの総長（重複を含む）とすると、最悪時間O(N+M)、入力を除くメモリO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`complement_components`](#components) | O(N+M) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 各頂点番号は`[0,N)`、`N < INT_MAX`が必要です。空グラフでは空の成分配列を返します。
- 入力を変更しません。ACLに補グラフ専用の探索はありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/complement-components.hpp"
int main() {
  std::vector<std::vector<int>> g{{1,2},{0},{0}};
  auto groups = blueberry::complement_components(g);
  assert(groups.size() == 2 && groups[0] == std::vector<int>{0});
  assert(groups[1] == (std::vector<int>{1,2}));
}
```
{% endraw %}

<details class="api-operation" id="components" markdown="1">
<summary><code>vector&lt;vector&lt;int&gt;&gt; complement_components(const vector&lt;vector&lt;int&gt;&gt;& graph)</code> — O(N+M)</summary>

各頂点をちょうど一度含む連結成分配列を返します。成分内部の頂点順は探索順です。未訪問の連結リストを走査し、削除された頂点は一回、残る頂点は元グラフの辺に対応づけることで線形時間になります。

{% raw %}
```cpp
auto groups = blueberry::complement_components(std::vector<std::vector<int>>(3));
```
{% endraw %}

注意点: 有向入力には対応しません。返り値は所有する値です。隣接リストの対称性は検査しません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

[Library Checker](https://judge.yosupo.jp/problem/connected_components_of_complement_graph) の公式ケースと固定 seed を指定できる全列挙比較で検証します。
候補比較・一次資料・測定条件は [調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md) を参照してください。

</details>
