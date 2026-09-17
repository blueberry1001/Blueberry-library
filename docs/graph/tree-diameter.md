---
title: Tree Diameter
documentation_of: //blueberry/graph/tree-diameter.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

非負重みの木の直径長と頂点列を二回の非再帰走査で求めます。ACLには木の直径APIがありません。
N頂点（N ≤ INT_MAX）、端点 [0,N)、各無向辺を同じ重みで両方向に1回ずつ登録した連結木が必要です。空入力も許可します。
自己ループ、多重辺、負重み、非連結入力は禁止。時間・追加メモリは最悪 O(N)。
Tはゼロ初期化・加算・比較・コピーが可能で、加算と順序が非負距離として整合する型です。全パス長がTに収まることが必要です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/tree-diameter.hpp"
int main() {
  std::vector<std::vector<std::pair<int,long long>>> g(3);
  g[0] = {{1,2}}; g[1] = {{0,2},{2,3}}; g[2] = {{1,3}};
  auto [length, path] = blueberry::tree_diameter(g);
  assert(length == 5 && path.size() == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `pair<T, vector<int>> tree_diameter(const vector<vector<pair<int,T>>>& graph)` | O(N) | [開く](#diameter) |

<details class="api-operation" id="diameter" markdown="1">
<summary><code>tree_diameter(graph)</code> — O(N)</summary>

最大重み和と、そのパス上の頂点番号を端点から順に返します。同値の最適パスの選択は保証しません。
空グラフは `{T{}, {}}`、1頂点は `{T{}, {0}}`、全辺が0の木では1頂点だけを返すことがあります。

{% raw %}
```cpp
std::vector<std::vector<std::pair<int,int>>> singleton(1);
auto result = blueberry::tree_diameter(singleton);
assert(result.first == 0 && result.second == std::vector<int>{0});
```
{% endraw %}

注意点: 入力を変更・保持せず、返却値は独立して所有します。NaNを含む浮動小数点は不可で、丸め誤差の影響は利用者が管理します。
整数の加算オーバーフローを検出しません。Tの各演算をO(1)として最悪O(N)、償却ではありません。
</details>

## 出典・検証

[CP-Algorithmsの木直径](https://lib.cp-algorithms.com/cp-algo/tree/diameter.hpp.html) と
[Nachiaの木直径](https://nachiavivias.github.io/cp-library/cpp/tree/tree-diameter.html)
の木DP・パス復元APIを調査し、非負重みに対象を限定した二回走査の独自実装。コードの転用はありません。
[Library Checker](https://judge.yosupo.jp/problem/tree_diameter) と全点対距離による乱択比較で検証します。
測定は [graph-batch-two](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/graph-batch-two.md)。
