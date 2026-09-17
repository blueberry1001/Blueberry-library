---
layout: page
title: 旧ライブラリからの移行
---

# 旧ライブラリからの移行

2026-09-17 の整備で、未検証のルート直下スニペットと旧 FPS 入口を削除しました。
新しいコードでは次のヘッダを include し、`blueberry` 名前空間を使ってください。
旧パスを使う提出ファイルは include と API の変更が必要です。過去の版は Git の履歴に残っています。

## include の対応表

| 削除した旧ファイル | 移行先 | 主な違い |
| --- | --- | --- |
| `blueberry/ConvexHulltrick.hpp` | [Dynamic Li Chao Tree]({{ '/blueberry/data-structure/dynamic-li-chao-tree.hpp.html' | relative_url }}) | 領域はコンストラクタの半開区間。未定義の最小値は optional |
| `blueberry/DynamicFenwickTree2D.hpp` | [Dynamic Fenwick Tree 2D]({{ '/blueberry/data-structure/dynamic-fenwick-tree-2d.hpp.html' | relative_url }}) / [1D]({{ '/blueberry/data-structure/dynamic-fenwick-tree.hpp.html' | relative_url }}) | 未知の座標をオンラインで追加。巨大な x 領域を密な配列で確保しない |
| `blueberry/implicit_treap.hpp` | [Ordered Multiset]({{ '/blueberry/data-structure/ordered-multiset.hpp.html' | relative_url }}) | 旧版はキー順多重集合。順位区間の集約は `prod` |
| `blueberry/fraction.hpp` | [Fraction]({{ '/blueberry/math/fraction.hpp.html' | relative_url }}) | `blueberry::Fraction`、`num()/den()`、常に既約 |
| `blueberry/RollbackUnionFind.hpp` | [Rollback Union Find]({{ '/blueberry/data-structure/rollback-union-find.hpp.html' | relative_url }}) | `merge/leader/comp_size/state/rollback` |
| `blueberry/Graph.hpp` | [Dijkstra]({{ '/blueberry/graph/dijkstra.hpp.html' | relative_url }}) など用途別の Graph カテゴリ | 全機能を1クラスに集めた互換 API は提供しない |
| `blueberry/fps.hpp` | [Formal Power Series]({{ '/blueberry/math/formal-power-series.hpp.html' | relative_url }}) | グローバルな `mint/FPS/sfps` は必要な場所で定義 |

標準機能の検証済み Blueberry 版（DSU / Fenwick Tree / Segment Tree / Z Algorithm）と、
その既存メソッド別名は残しています。通常は ACL を第一候補にしてください。

## オンライン構造の選び方

- 問い合わせ x が先に分かるなら [Li Chao Tree]({{ '/blueberry/data-structure/li-chao-tree.hpp.html' | relative_url }})、
  分からないなら Dynamic Li Chao Tree。旧テンプレートの **閉区間 `[low, high]`** は、
  新コンストラクタの **半開区間 `[low, high)`** と異なります。
  同じ領域にするには上端に 1 を加えますが、`INT64_MAX` そのものを含む領域は表現できません。
- 更新点が先に分かるなら [Offline Fenwick Tree 2D]({{ '/blueberry/data-structure/offline-fenwick-tree-2d.hpp.html' | relative_url }}) を優先。
  分からない場合に Dynamic Fenwick Tree 2D を使います。
  `sum(left, down, right, up)` は `[left,right) × [down,up)`。prefix は `pref(x,y)` です。
  旧1Dの `DynamicFenwickTree<座標型,値型>` は、新1Dでは **`DynamicFenwickTree<値型,座標型>`** です。
- 動的な**列**を扱う新しい [Implicit Treap]({{ '/blueberry/data-structure/implicit-treap.hpp.html' | relative_url }}) は、
  `insert(p,x)/erase(p)/reverse(l,r)/apply(l,r,f)` が使えます。
  キーの順序で管理したい旧 `Treap` の移行先は Ordered Multiset です。

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"
#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"
#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"
#include "blueberry/data-structure/implicit-treap.hpp"
#include "blueberry/data-structure/ordered-multiset.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  blueberry::DynamicLiChaoTree<long long> cht(-10, 11); // -10..10
  assert(!cht.query(0));
  cht.add_line(2, 3);
  cht.add_segment(0, 2, -1, 0); // x=0,1 のみ
  assert(cht.query(1).value() == -1);
  blueberry::DynamicFenwickTree2D<long long> bit(1000000000, 1000000000);
  bit.add(123, 456, 7);
  assert(bit.sum(123, 456, 124, 457) == 7);
  blueberry::DynamicFenwickTree<long long> sparse(1000000000);
  sparse.add(123, 4);
  assert(sparse.lower_bound(4) == 123);
  blueberry::OrderedMultiset<long long, op, e> sorted;
  sorted.insert(3); sorted.insert(1); sorted.insert(3);
  assert(sorted.count(3) == 2 && sorted.prod(1, 3) == 6);
  assert(!sorted.erase(9)); // 存在しないキーも安全
  blueberry::ImplicitTreap<long long, op, e> sequence({1, 2, 3});
  sequence.reverse(0, 3);
  sequence.insert(1, 9);
  assert(sequence.get(0) == 3 && sequence.prod(0, 2) == 12);
}
```
{% endraw %}

旧 Treap の `query(l,r)` は `prod(l,r)`、`operator[](k)` は `kth(k)` です。
`kth` は範囲外なら空の optional を返します。`erase(x)` は同値キーを1個だけ削除し、削除できたかを返します。
`index(x)` の代わりに `rank(x)` を使いますが、`rank` は不在でも挿入位置を返します。
旧版の不在時 `-1` が必要なら `contains(x) ? rank(x) : -1`、最後の順位なら
`contains(x) ? rank(x) + count(x) - 1 : -1` とします。`count` の不在時は正しく 0 です。

## FPS・Fraction・Rollback Union Find

FPS の係数型を自分で選び、必要な別名だけ定義します。疎多項式は `FPS::Sparse` です。
Fraction の `raw_assign` はなく、構築・代入で常に正規化します。約分後の分子・正の分母が
int64 に収まること、分母や除数がゼロでないことが必要です。
Rollback Union Find の `size()` は頂点数、成分サイズは `comp_size(v)` です。
旧 `unite/find/get_state` は `merge/leader/state` へ、`rollback(-1)` は引数なしの `rollback()` へ変更します。

{% raw %}
```cpp
#include <cassert>
#include <atcoder/modint>
#include "blueberry/math/formal-power-series.hpp"
#include "blueberry/math/fraction.hpp"
#include "blueberry/data-structure/rollback-union-find.hpp"
using mint = atcoder::modint998244353;
using FPS = blueberry::FPS<mint>;
using sfps = FPS::Sparse;
int main() {
  FPS f{1, 1};
  auto inv = f.inv(3);
  assert(inv[0] == 1 && inv[1] == mint(-1) && inv[2] == 1);
  blueberry::Fraction q(6, -8);
  assert(q.num() == -3 && q.den() == 4);
  q = blueberry::Fraction(2, 3);
  assert(q + 1 == blueberry::Fraction(5, 3));
  blueberry::RollbackUnionFind uf(3);
  int saved = uf.state();
  uf.merge(0, 1);
  assert(uf.comp_size(0) == 2);
  uf.rollback(saved);
  assert(!uf.same(0, 1));
}
```
{% endraw %}

## Graph の置き換え

旧 `init()` の入力処理と 1-index → 0-index 変換は、解答側で明示します。
隣接リストへ辺を追加し、非負重みの最短路には `dijkstra(graph, source)` を使います。
無向辺は両方向に追加します。到達不能は `result.infinity`、経路は `result.path_to(v)` です。
全辺の重みが1なら、単純な BFS は引き続き O(V+E) で有用です。

旧 `isbipartite_graph()` のように二部グラフか判定したい場合は、
各辺に `色[u] XOR 色[v] = 1` を課す Potential Union Find が使えます。
Hopcroft–Karp は左右の集合が与えられた後の最大マッチング用で、二部性の判定ではありません。

{% raw %}
```cpp
#include <cassert>
#include <functional>
#include "blueberry/graph/dijkstra.hpp"
#include "blueberry/data-structure/potential-union-find.hpp"
struct XorInverse { int operator()(int x) const { return x; } };
int main() {
  blueberry::WeightedGraph<long long> graph(3);
  graph[0].push_back({1, 4}); graph[1].push_back({0, 4});
  graph[1].push_back({2, 2}); graph[2].push_back({1, 2});
  auto result = blueberry::dijkstra(graph, 0);
  assert(result.distance[2] == 6 && result.path_to(2).size() == 3);
  blueberry::PotentialUnionFind<int, std::bit_xor<int>, XorInverse> colors(3);
  assert(colors.merge(0, 1, 1));
  assert(colors.merge(1, 2, 1));
  assert(!colors.merge(2, 0, 1)); // 三角形は二部グラフでない
}
```
{% endraw %}

各新ライブラリのページで型の条件、計算量、空入力・境界の扱い、対応する公式 verify を確認してください。
