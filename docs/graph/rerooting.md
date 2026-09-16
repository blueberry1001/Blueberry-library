---
title: Rerooting DP
documentation_of: //blueberry/graph/rerooting.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

木の全頂点をそれぞれ根としたDPを一度に計算します。ACLにない全方位木DPの機能です。
頂点数を N とし、状態Tのコピー・代入と各callbackが O(1) なら、最悪時間・メモリは O(N) です。
木の巡回は反復処理で、20万頂点の鎖でも再帰スタックを消費しません。

頂点は `[0,N)`、`0 <= N <= INT_MAX`。入力は無向木の辺列 `vector<pair<int,int>>` で、
`N>0` なら辺数は `N-1`、連結かつ自己ループ・多重辺・閉路がないことが前提です。
`N=0` と空の辺列では空vectorを返し、callbackを呼びません。
辺IDは入力添字 `[0,N-1)`、各頂点での併合順はその頂点に接続する辺IDの昇順です。
**mergeに交換法則は不要**です。親方向の寄与も元の辺IDの位置に置くため、非可換なDPを扱えます。

状態Tはコピー構築・コピー代入可能で、default constructorは不要です。
`merge(T,T)->T` は結合則を満たし、`identity` は左右の単位元とします。
`add_vertex(T,int)->T` は子側の寄与を併合した値に頂点を加えます。
`transfer(T,int,int,int)->T` は頂点を加えた値を指定した辺で隣へ移します。
全callbackは入力を変更せず、同じ引数に同じ値を返す必要があります。呼び出し回数や評価順に依存しないでください。
逆元や除算は要求しません。値・重みのoverflow対策はcallback側で行ってください。
状態が可変長文字列などなら、実際の時間・メモリにはそのコピーとcallbackの費用を掛けて考えます。

## 最小使用例

{% raw %}
```cpp
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/rerooting.hpp"
int main() {
  std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}};
  std::vector<long long> weight{2, 3};
  // 各頂点からの最遠距離。非負辺重みなので単位元は0。
  auto farthest = blueberry::rerooting(3, edges, 0LL,
      [](long long a, long long b) { return std::max(a, b); },
      [](long long value, int) { return value; },
      [&](long long value, int id, int, int) { return value + weight[id]; });
  assert(farthest == std::vector<long long>({5, 3, 5}));
}
```
{% endraw %}

## 操作一覧

公開APIは次の関数だけです。入力・callbackへの参照を返り値に保持しません。
返り値は頂点順の独立したvectorで、呼び出し終了後も保持できます。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<T> rerooting(n, edges, identity, merge, add_vertex, transfer)` | O(N) | [開く](#rerooting) |

<details class="api-operation" id="rerooting" markdown="1">
<summary><code>vector&lt;T&gt; rerooting(int n, const vector&lt;pair&lt;int,int&gt;&gt;&amp; edges, T identity, Merge merge, AddVertex add_vertex, Transfer transfer)</code> — O(N)</summary>

`result[v]` は頂点vを根にした木全体のDP値です。各callbackの意味は次の通りです。

| callback | 意味 |
| --- | --- |
| `merge(left, right)` | 隣接部分木から来た寄与を辺ID順に併合する |
| `add_vertex(merged, v)` | 頂点v以外の寄与を併合した値にv自身を加える |
| `transfer(value, edge_id, from, to)` | fromを根とした側の状態を、辺edge_idを通してtoへ渡す |

`from` と `to` は辺の両端で、入力の `{u,v}` の向きとは無関係です。
方向別の重みや遷移にも対応できます。葉は `add_vertex(identity,v)`、
`N=1` の返り値は `{add_vertex(identity,0)}` と等価です。

次の例は各根から全頂点への距離の和です。状態は `{距離の和, 頂点数}` とし、
辺を1本通るたびに距離の和へ頂点数を加えます。

{% raw %}
```cpp
using State = std::pair<long long, long long>;
auto result = blueberry::rerooting(3, edges, State{0, 0},
    [](State a, State b) { return State{a.first + b.first, a.second + b.second}; },
    [](State value, int) { return State{value.first, value.second + 1}; },
    [](State value, int, int, int) {
      return State{value.first + value.second, value.second};
    });
assert(result[0].first == 3);  // edges = {{0,1}, {1,2}}
```
{% endraw %}

注意点: 最悪 O(N) はcallbackとTのコピーが定数時間の場合です。各頂点でprefixとsuffixを用いて
1方向を除外するため、次数が大きくても次数の二乗にはなりません。入力vectorとcallbackが参照する
重み配列は呼び出し中有効である必要があります。返り値への参照・iteratorは通常のvectorと同じ規則で無効化されます。
assertを無効化した場合も不正な木を渡してよいわけではありません。

</details>

## 出典・検証

[Nyaan Rerooting](https://nyaannyaan.github.io/library/tree/rerooting.hpp.html) と
[MtSaka ReRooting](https://mtsaka.github.io/library/graph/tree/rerooting.hpp) の
2方向の木DP・prefix/suffixによる除外計算・callback設計を比較し、標準的な全方位木DPから独立に実装しました。
コードの転用はしていません。本実装では反復巡回、辺ID順の非可換併合、明示的な頂点追加と辺遷移、
再利用する一時vectorを採用しています。callbackはテンプレート引数とし、`std::function` を使いません。

公式問題は [Tree Path Composite Sum](https://judge.yosupo.jp/problem/tree_path_composite_sum)。
検証コードはACL modintで `{sum,count}` を持ち、辺 `bx+c` を通す際に `sum=b*sum+c*count` とします。
`tests/random/rerooting.cpp` は全頂点を個別に根とする愚直解と、非可換文字列・方向依存の重みを比較します。
空入力・単独頂点・20万頂点の鎖と星も検証します。
