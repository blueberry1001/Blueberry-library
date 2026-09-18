---
title: Complement Graph Components
documentation_of: //blueberry/graph/complement-components.hpp
---

[カテゴリへ戻る]({{ "/categories/graph.html" | relative_url }})

## 概要・前提

無向グラフの補グラフを明示的に構築せず連結成分を列挙します。ACL に補グラフ専用の探索はありません。N 頂点、隣接リスト総長 M として最悪 O(N+M) 時間、入力を除き O(N) メモリです。隣接リストは両方向に登録し、各頂点番号は [0,N)、N < INT_MAX が必要です。自己ループ・重複辺は無視されます（重複も M に数えます）。空グラフは空の成分配列です。入力を変更しません。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<vector<int>> complement_components(const vector<vector<int>>& graph)` | O(N+M) | [開く](#components) |

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

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/connected_components_of_complement_graph) の公式ケースと固定 seed を指定できる全列挙比較で検証します。
候補比較・一次資料・測定条件は [調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md) を参照してください。
