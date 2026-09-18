---
title: Minimum Spanning Forest
documentation_of: //blueberry/graph/minimum-spanning-forest.hpp
---

[カテゴリへ戻る]({{ "/categories/graph.html" | relative_url }})

## 概要・前提

Kruskal 法と ACL dsu で無向重み付きグラフの最小全域森を求めます。dsu は自作せず ACL を使う薄い組合せです。N 頂点 M 辺、時間 O(N+M log(M+1)) 最悪（DSU 部分は償却）、補助メモリ O(N+M)。辺は tuple<int,int,T> の (u,v,weight)、u,v∈[0,N)、N,M は int に収まること。T は厳密な大小比較と加算ができ、選ばれた重みの途中の和が収まる型を使います。負辺、多重辺、自己ループ、非連結グラフを許します。入力は変更しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/minimum-spanning-forest.hpp"
int main() {
  std::vector<std::tuple<int,int,long long>> edges{{0,1,4},{1,2,1},{0,2,2}};
  auto [cost, ids] = blueberry::minimum_spanning_forest(4, edges);
  assert(cost == 3 && ids.size() == 2);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `pair<T, vector<int>> minimum_spanning_forest(int n, const vector<tuple<int,int,T>>& edges)` | O(N+M log(M+1)) | [開く](#forest) |

<details class="api-operation" id="forest" markdown="1">
<summary><code>pair&lt;T, vector&lt;int&gt;&gt; minimum_spanning_forest(int n, const vector&lt;tuple&lt;int,int,T&gt;&gt;& edges)</code> — O(N+M log(M+1))</summary>

最小総費用と採用辺の入力中の添字を返します。入力の各連結成分を木にします。元の成分数 C に対し辺数は N−C。重み順に選択し、同じ重みの順序は保証しません。

{% raw %}
```cpp
auto [cost, ids] = blueberry::minimum_spanning_forest<long long>(2, {{0,1,-3}});
```
{% endraw %}

注意点: N=0 では辺なしに限り {0,{}}。孤立点は費用 0、自己ループは採用しません。返り値は入力を参照しない所有する値です。重みの総和のオーバーフローは検出しません。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/minimum_spanning_tree) の公式ケースと固定 seed を指定できる全列挙比較で検証します。
候補比較・一次資料・測定条件は [調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md) を参照してください。
