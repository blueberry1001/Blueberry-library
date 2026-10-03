---
title: Dominator Tree
documentation_of: //blueberry/graph/dominator-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

有向グラフの始点から各頂点へ進むとき、必ず通る頂点を支配木で表します。
頂点vへのすべての経路が頂点uを通るとき、uはvを支配します。
v自身を除く支配頂点のうち、支配木でvに最も近い頂点が直接支配頂点です。
`dominator_tree`は各頂点の直接支配頂点を返します。ACLに同等の機能はありません。

Nは頂点数、Mは有向辺数です。頂点番号は`[0,N)`で、`N <= INT_MAX`を前提とします。
辺数と隣接リストの位置は`size_t`に収まる必要があります。自己ループ、平行辺、非連結グラフに対応します。
辺の重みは扱いません。Nが正なら`0 <= root < N`が必要です。
空グラフには`root = -1`を指定し、空の配列を受け取ります。

単純版Lengauer–Tarjan法を使います。Nが正のとき、最悪時間はO((N+M) log(N+1))、追加メモリはO(N+M)です。
返却配列にO(N)を使い、作業用の逆向き辺には到達可能な頂点から出る辺だけを格納します。
空グラフの時間・追加メモリはO(1)です。DFSと経路圧縮はいずれも反復処理で、再帰の深さに依存しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/dominator-tree.hpp"
int main() {
  // 0から3へは1・2のどちらからも進める。4へ進むには3が必要。
  std::vector<std::vector<int>> graph{{1, 2}, {3}, {3}, {4}, {}, {3}};
  auto idom = blueberry::dominator_tree(graph, 0);
  assert(idom == std::vector<int>({0, 0, 0, 0, 3, -1}));
  assert(blueberry::dominator_tree({}, -1).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> dominator_tree(const vector<vector<int>>& graph, int root)` | 最悪O((N+M) log(N+1))、N=0ならO(1) | [開く](#dominator-tree) |

<details class="api-operation" id="dominator-tree" markdown="1">
<summary><code>vector&lt;int&gt; dominator_tree(graph, root)</code> — O((N+M) log(N+1))</summary>

`graph[u]`にuから出る有向辺の行き先を並べます。
長さNの配列`idom`を返し、`idom[root] = root`とします。
rootから到達できる頂点vについて、vがrootと異なれば`idom[v]`は直接支配頂点です。
到達できない頂点は`-1`です。到達できない頂点から到達できる頂点への辺は、支配関係に影響しません。

各到達可能頂点vから`idom[v]`をたどるとrootへ至ります。
この列に現れる頂点はすべてvを支配します。支配木を隣接リストに直すには、root以外の到達可能頂点vを`idom[v]`の子として追加します。

{% raw %}
```cpp
std::vector<std::vector<int>> graph{{1}, {}, {0, 1}, {2}};
auto idom = blueberry::dominator_tree(graph, 2);
assert(idom == std::vector<int>({2, 2, 2, -1}));
std::vector<std::vector<int>> tree(graph.size());
for (int v = 0; v < static_cast<int>(graph.size()); ++v)
  if (idom[v] != -1 && idom[v] != v) tree[idom[v]].push_back(v);
assert(tree[2] == std::vector<int>({0, 1}));
```
{% endraw %}

注意点: 入力を変更せず、参照も保持しません。返却配列は独立して所有されます。
入力を変更した後の結果が必要なら、もう一度関数を呼び出します。
不正な頂点番号やrootは前提違反です。assertを無効にしたビルドでも前提を満たす必要があります。
頂点番号と辺の位置以外の数値計算はありません。

DFS順に番号を付け、半支配頂点を逆順に計算します。
逆向き辺を連続した配列へ格納し、処理待ちの頂点は配列による連結リストで管理します。
最後にDFS順で直接支配頂点を確定します。経路圧縮を使う単純版の全体計算量を上に示しており、逆Ackermann関数による計算量は保証しません。
</details>

## 出典・検証

[Lengauer–Tarjanの原論文](https://doi.org/10.1145/357062.357071)に基づく独自実装です。
[ei1333の実装](https://ei1333.github.io/library/graph/others/dominator-tree.hpp.html)と
[Library Checker公式解](https://github.com/yosupo06/library-checker-problems/blob/master/graph/dominatortree/sol/correct.cpp)を調査しました。
前者はUnlicense、後者はApache-2.0で公開されています。実装コードの転用はありません。
再帰の除去と連続配列による格納を採用し、比較条件と測定結果を
[性能レポート](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/dominator-tree.md)に記録しています。

[Library Checker: Dominator Tree](https://judge.yosupo.jp/problem/dominatortree)で検証します。
ランダムテストでは頂点を1つずつ除いて到達性を調べる愚直解と比較します。
4頂点以下の自己ループを除く全有向グラフについて、すべての始点を検証します。
自己ループ・平行辺・到達不能頂点を含むランダムグラフ、隣接順の変更、20万頂点の道と戻り辺も確認します。
