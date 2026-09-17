---
title: Cycle Detection
documentation_of: //blueberry/graph/cycle-detection.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

有向・無向グラフから単純閉路を1つ探し、入力の辺番号を巡回順で返します。ACLには閉路の辺列復元APIがありません。
頂点数 N、辺数 M とし、0 ≤ N ≤ INT_MAX、M ≤ INT_MAX、端点は [0,N)。自己ループ・多重辺・非連結グラフに対応します。
時間・作業メモリは最悪 O(N+M)、返却値は O(N)。再帰を使わず、深いグラフでも呼び出しスタックを消費しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/cycle-detection.hpp"
int main() {
  std::vector<std::pair<int,int>> e{{0,1},{1,2},{2,0}};
  auto cycle = blueberry::find_cycle<true>(3, e);
  assert(cycle == std::vector<int>({0,1,2}));
  assert(blueberry::find_cycle<false>(2, {{0,1}}).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> find_cycle<Directed>(int n, const vector<pair<int,int>>& edges)` | O(N+M) | [開く](#find-cycle) |

<details class="api-operation" id="find-cycle" markdown="1">
<summary><code>find_cycle&lt;Directed&gt;(n, edges)</code> — O(N+M)</summary>

`Directed=true` は有向、`false` は無向です。`edges[i]` の番号 i を閉路に沿って返します。有向の場合は各辺の終点が次の始点です。
無向の場合はどちら向きの巡回も許し、必要なら最初の辺の両端を試して頂点列を復元します。閉路なし（N=0を含む）は空配列。
無向の自己ループは長さ1、異なる平行辺は長さ2の閉路になります。返す閉路の選択は保証しません。

{% raw %}
```cpp
auto ids = blueberry::find_cycle<false>(2, {{0,1},{0,1}});
assert(ids.size() == 2);
```
{% endraw %}

注意点: 入力は変更・保持せず、返却vectorは独立に所有されます。辺番号はint、隣接位置はsize_tです。計算量は最悪値で償却クエリではありません。
</details>

## 出典・検証

[CP-AlgorithmsのDFS閉路復元](https://lib.cp-algorithms.com/cp-algo/graph/cycle.hpp.html) と
[CP-Algorithmsの再帰DFS実装](https://cp-algorithms.com/graph/finding-cycle.html)
を調査し、三色DFSと親辺番号による独自の非再帰実装を採用。コードの転用はありません。
Library Checker [Cycle Detection](https://judge.yosupo.jp/problem/cycle_detection) /
[Undirected](https://judge.yosupo.jp/problem/cycle_detection_undirected)、乱択による推移閉包・DSU比較、20万頂点の道で検証します。
候補比較と測定条件は [graph-batch-two](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/graph-batch-two.md) を参照。
