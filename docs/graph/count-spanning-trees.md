---
title: Counting Spanning Trees
documentation_of: //blueberry/graph/count-spanning-trees.hpp
---

[カテゴリへ戻る]({{ "/categories/graph.html" | relative_url }})

## 概要・前提

Matrix-Tree 定理と既存の Matrix<T>::det() を組み合わせ、無向全域木または根から外向きに到達する有向全域木を数えます。ACL の modint と併用します。T は厳密な体（例: 素数法の modint）で、整数除算や誤差を含む実数消去には対応しません。N 頂点 M 辺で最悪 O(N³+M) 時間、O(N²) 補助メモリ。辺は (u,v)、番号は [0,N)、N は int に収まること。多重辺は区別して数え、自己ループは無視します。返り値の算術は T に従い、modint なら法で割った余りです。入力は変更しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <atcoder/modint>
#include "blueberry/graph/count-spanning-trees.hpp"
int main() {
  using mint = atcoder::modint998244353;
  std::vector<std::pair<int,int>> edges{{0,1},{1,2},{0,2}};
  assert(blueberry::count_spanning_trees<mint>(3, edges).val() == 3);
  assert(blueberry::count_directed_spanning_trees<mint>(3, edges, 0).val() == 2);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `T count_spanning_trees<T>(int n, const vector<pair<int,int>>& edges)` | O(N³+M) | [開く](#undirected) |
| `T count_directed_spanning_trees<T>(int n, const vector<pair<int,int>>& edges, int root)` | O(N³+M) | [開く](#directed) |

<details class="api-operation" id="undirected" markdown="1">
<summary><code>T count_spanning_trees&lt;T&gt;(int n, const vector&lt;pair&lt;int,int&gt;&gt;& edges)</code> — O(N³+M)</summary>

無向全域木の個数を返します。各無向辺を一回だけ登録します。非連結なら 0、N=1 なら 1。N=0 かつ辺なしの値は空の積として 1 と定義します。

{% raw %}
```cpp
auto count = blueberry::count_spanning_trees<atcoder::modint998244353>(2, {{0,1}});
```
{% endraw %}

注意点: N≥0 が必要です。両方向を登録すると別の多重辺として数えてしまいます。返り値は所有する値です。

</details>

<details class="api-operation" id="directed" markdown="1">
<summary><code>T count_directed_spanning_trees&lt;T&gt;(int n, const vector&lt;pair&lt;int,int&gt;&gt;& edges, int root)</code> — O(N³+M)</summary>

根 root から全頂点に到達できる外向き全域木を数えます。N≥1、root∈[0,N) が必要です。根以外の各頂点の入次数が 1 になる木です。N=1 なら 1、根から到達できない頂点があれば 0。

{% raw %}
```cpp
auto count = blueberry::count_directed_spanning_trees<atcoder::modint998244353>(2, {{0,1}}, 0);
```
{% endraw %}

注意点: 根へ向かう内向き木が必要なら入力辺を反転してください。根へ入る辺と自己ループは寄与しません。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/counting_spanning_tree_directed) の公式ケースと固定 seed を指定できる全列挙比較で検証します。
候補比較・一次資料・測定条件は [調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-graph-research.md) を参照してください。
[無向版の公式問題](https://judge.yosupo.jp/problem/counting_spanning_tree_undirected) も検証します。
