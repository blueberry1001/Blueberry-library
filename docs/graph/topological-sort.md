---
title: Topological Sort
documentation_of: //blueberry/graph/topological-sort.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要

有向グラフをKahn法でトポロジカルソートし、閉路があれば`nullopt`を返します。自己ループ・平行辺・非連結入力に対応します。

Nを頂点数、Mを辺数とすると、最悪時間O(N+M)、追加メモリO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`topological_sort`](#sort) | O(N+M) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `N <= INT_MAX`、端点は`[0,N)`とします。入次数カウントはsize_tに収まる必要があります。
- ACLのSCCで代用できますが、この単一操作はより短く使えます。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/topological-sort.hpp"
int main() {
  auto order = blueberry::topological_sort({{1},{2},{}});
  assert(order && *order == std::vector<int>({0,1,2}));
  assert(!blueberry::topological_sort({{0}}));
}
```
{% endraw %}

<details class="api-operation" id="sort" markdown="1">
<summary><code>topological_sort(graph)</code> — O(N+M)</summary>

`graph[u]` はuからの行き先一覧。DAGなら全頂点が1回ずつ現れ、各辺u→vでuが先行する順列を返します。
閉路ならnullopt（部分結果は返しません）。N=0なら有効な空vectorです。辞書順最小は保証しません。

{% raw %}
```cpp
auto empty = blueberry::topological_sort({});
assert(empty && empty->empty());
```
{% endraw %}

注意点: 入力を変更・保持せず、返却値は独立して所有します。再帰はなく、vectorをキュー兼結果として使います。
計算量は最悪値（内部push_backの償却を合計）です。
</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

[DFSによる実装](https://cp-algorithms.com/graph/topological-sort.html) と
[KACTLのKahn法](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/TopoSort.h)
を調査し、入次数とvectorキューによる独自実装を採用。コードの転用はありません。
直接対応するLibrary Checker問題はなく、ランダムグラフの推移閉包による閉路判定・各辺の順序・20万頂点の道で検証します。
測定は [graph-batch-two](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/graph-batch-two.md)。

</details>
