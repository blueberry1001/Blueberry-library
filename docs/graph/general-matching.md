---
title: General Graph Matching
documentation_of: //blueberry/graph/general-matching.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

無向グラフの最大マッチングを求めます。奇数長の閉路を含むグラフにも使える、Edmondsのblossom法です。ACLには一般グラフの最大マッチングはありません。二部グラフでは、このライブラリの[HopcroftKarp](hopcroft-karp.md)を使うと計算量を抑えられます。

頂点数をN、入力辺数をMとします。自己ループは無視し、多重辺は1本にまとめます。正規化を含む最悪時間計算量はO(N³+M)、入力と返り値を除く補助メモリはO(N²)です。疎なグラフでも正規化にN×Nの作業配列を使います。Library CheckerのN≤500を主な利用規模として検証していますが、固定の頂点数上限は設けていません。

頂点番号は0からN−1です。Nは非負のint型で、N²がstd::size_tに収まり、必要なメモリを確保できることを前提とします。N=0では辺も空にし、空配列を返します。この場合の時間と補助メモリはO(1)です。入力は変更しません。乱数も再帰呼び出しも使いません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/general-matching.hpp"
int main() {
  std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}, {2, 0}, {2, 3}};
  auto mate = blueberry::general_matching(4, edges);
  int size = 0;
  for (int v = 0; v < 4; ++v) {
    assert(mate[v] >= 0 && mate[mate[v]] == v);
    size += v < mate[v];
  }
  assert(size == 2);
  assert(blueberry::general_matching(0, {}).empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> general_matching(int n, const vector<pair<int, int>>& edges)` | O(N³+M)、最悪 | [開く](#general-matching) |

<details class="api-operation" id="general-matching" markdown="1">
<summary><code>vector&lt;int&gt; general_matching(int n, const vector&lt;pair&lt;int, int&gt;&gt;&amp; edges)</code>：O(N³+M)</summary>

N=n頂点の無向グラフから、辺数が最大になるマッチングを1つ求めます。edgesの各要素(u,v)は無向辺を表し、0≤u,v&lt;Nが必要です。自己ループでも端点の範囲条件は同じです。重複や端点の順序を区別しません。

返り値mateは長さNの配列です。頂点vが未マッチならmate[v]=−1、マッチしていればmate[v]は相手の頂点番号です。マッチした頂点ではmate[mate[v]]=vとなり、(v,mate[v])は入力にある自己ループ以外の辺です。v&lt;mate[v]を満たす頂点を数えるとマッチングの辺数になります。

{% raw %}
```cpp
auto mate = blueberry::general_matching(
    3, std::vector<std::pair<int, int>>{{0, 1}, {1, 0}, {0, 0}});
assert(mate[0] == 1 && mate[1] == 0 && mate[2] == -1);
```
{% endraw %}

注意点: 解が複数ある場合の選び方は保証しません。返り値は入力への参照を持たず、入力の寿命に依存しません。範囲外の頂点や負のNは契約違反で、assertが有効なときに検出します。重み付きマッチングは扱いません。

最初にO(N²+M)で辺を正規化し、貪欲法で初期マッチングを作ります。増加路探索では奇数閉路を縮約し、親配列を使ってマッチングを反転します。1回の探索はO(N²)、探索回数は高々N回です。辺数が⌊N/2⌋に達したら探索を終了します。

</details>

## 出典・検証

アルゴリズムの出典はJack Edmondsの[Paths, Trees, and Flowers](https://doi.org/10.4153/CJM-1965-045-4)です。[Library Checkerの公式参照実装](https://github.com/yosupo06/library-checker-problems/blob/master/graph/general_matching/sol/correct.cpp)、[ei1333のGabowEdmonds](https://github.com/ei1333/library/blob/master/graph/flow/gabow-edmonds.hpp)、[KACTLのGeneralMatching](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/GeneralMatching.h)を調査しました。KACTLは確率的な行列法なので、乱数に依存しない縮約法を採用しました。公開提出のコードは転載していません。

[Library CheckerのMatching on General Graph](https://judge.yosupo.jp/problem/general_matching)に対応するverifyを用意しています。乱択テストでは、6頂点以下の全単純グラフ、16頂点以下の愚直解との比較、多重辺、自己ループ、頂点番号と辺順の置換、非連結グラフ、入れ子の縮約を検証します。seedを指定して再現できます。

同一入力・コンパイラでの候補比較と採用理由は[性能調査記録](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/general-matching.md)に記載しています。
