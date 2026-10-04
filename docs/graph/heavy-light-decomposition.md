---
title: Heavy Light Decomposition
documentation_of: //blueberry/graph/heavy-light-decomposition.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

木のパス上の値をまとめて調べたり更新したりするときに使います。
HLDは木の頂点を配列の位置に対応させ、パスをO(log N)個の半開区間 `[l,r)` に分けます。
各区間の値はACLの `fenwick_tree`、`segtree`、`lazy_segtree` などで管理します。
`HeavyLightDecomposition` の短縮名が `HLD` です。ACLにHLD自体の実装はありません。

Nは頂点数です。HLD単体の構築時間・メモリはO(N)です。
木の形と根は構築後に変えません。頂点や辺の値は、組み合わせたデータ構造で更新します。
すべての区間は左端を含み右端を含まない `[l,r)` です。`r` を1減らしてACLへ渡す必要はありません。

入力は `1 <= N <= INT_MAX` の連結な木で、頂点番号は `[0,N)`、根も `[0,N)` です。
無向隣接リスト、または指定した根から全頂点へ辿れる親→子リストを渡します。
空グラフ、閉路、自己ループ、同じ辺の重複は前提外です。
無向辺を両方向の隣接リストに1回ずつ入れる通常の表現は使えます。
入力への参照は保持しません。各頂点の位置は構築時に決まり、値を更新しても変わりません。

| やりたいこと | 値を置く位置 | パスの指定 | 使用例 |
| --- | --- | --- | --- |
| 頂点への加算・パスの和 | `hld.in(v)` | `vertex_query(u,v,f)` | [頂点の和](#vertex-sum) |
| 辺の重みの変更・パスの和 | `hld.edge_index(u,v)` | `edge_query(u,v,f)` | [辺の和](#edge-sum) |
| 根から頂点までの辺への加算・和 | 根以外の `hld.in(v)` | `edge_query(hld.root(),v,f)` | [根からの辺パス](#root-edge-sum) |
| u→vの順で関数を合成 | `hld.in(v)` | 3引数の `vertex_query` | [順序が必要な演算](#path-composite) |

頂点パスには両端とLCAが含まれます。辺パスでは、各辺を深い側の頂点の位置で表し、LCAの位置を除きます。
根には親から入る辺がないため、辺用配列の `hld.in(hld.root())` は使わない位置です。
和なら0など、演算の単位元を置いておきます。

<a id="vertex-sum"></a>

## 最小使用例

頂点の値への加算と、パス上の頂点の和を扱います。
更新する位置は元の頂点番号ではなく `hld.in(v)` です。

{% raw %}
```cpp
#include <atcoder/fenwicktree>
#include <cassert>
#include <vector>
#include "blueberry/graph/heavy-light-decomposition.hpp"
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::HLD hld(tree);
  std::vector<long long> value{10, 20, 30, 40};
  atcoder::fenwick_tree<long long> bit(hld.size());
  for (int v = 0; v < hld.size(); ++v) bit.add(hld.in(v), value[v]);

  auto path_sum = [&](int u, int v) {
    long long answer = 0;
    hld.vertex_query(u, v, [&](int l, int r) { answer += bit.sum(l, r); });
    return answer;
  };
  assert(path_sum(2, 3) == 100);  // 頂点2→0→1→3
  bit.add(hld.in(1), 5);
  assert(path_sum(2, 3) == 105);
  assert(path_sum(2, 2) == 30);   // 同じ頂点なら、その頂点の値
  assert(hld.lca(2, 3) == 0);
  assert(hld.jump(2, 3, 2) == 1);
}
```
{% endraw %}

この例の初期化はO(N log N)、1頂点への加算はO(log N)、パスの和はO(log² N)です。
和を保持する `long long` に中間結果まで収まることを前提とします。
練習問題は [Library Checker: Vertex Add Path Sum](https://judge.yosupo.jp/problem/vertex_add_path_sum) です。

<h3 id="edge-sum">辺の重みを変更し、パス上の和を求める</h3>

辺 `(u,v)` の位置は `edge_index(u,v)` で取得します。
辺番号で更新する問題では、入力時の両端を保存しておくとそのまま使えます。
`edge_query` はLCAの位置を自動で除くため、呼び出し側で引く必要はありません。

{% raw %}
```cpp
#include <atcoder/segtree>
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/heavy-light-decomposition.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::HLD hld(tree);
  std::vector<std::pair<int, int>> edges{{0, 1}, {0, 2}, {1, 3}};
  std::vector<long long> weight{3, 5, 7};
  std::vector<long long> base(hld.size(), 0);  // 根の位置は0のまま
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    auto [u, v] = edges[i];
    base[hld.edge_index(u, v)] = weight[i];
  }
  atcoder::segtree<long long, op, e> seg(base);
  auto set_edge = [&](int id, long long value) {
    auto [u, v] = edges[id];
    seg.set(hld.edge_index(u, v), value);
  };
  auto path_sum = [&](int u, int v) {
    long long answer = 0;
    hld.edge_query(u, v, [&](int l, int r) { answer += seg.prod(l, r); });
    return answer;
  };
  assert(path_sum(2, 3) == 15);  // 辺2-0、0-1、1-3
  set_edge(2, 10);              // edges[2]、つまり辺1-3を変更
  assert(path_sum(2, 3) == 18);
  assert(path_sum(3, 3) == 0);  // 辺がないのでcallbackも呼ばれない
}
```
{% endraw %}

初期化はO(N)、1辺の変更はO(log N)、パスの和はO(log² N)です。
`hld.distance(u,v)` は辺の本数を返すため、重みの和には上の `path_sum` を使います。
[AtCoder ABC294 G: Distance Queries on a Tree](https://atcoder.jp/contests/abc294/tasks/abc294_g) が対応します。
この問題の頂点番号・辺番号は1始まりなので、入力時に1を引きます。重みの和には `long long` が必要です。

<h3 id="root-edge-sum">根から頂点までの辺に加算し、和を求める</h3>

根からvへの辺パスは `edge_query(hld.root(),v,f)` で指定します。
`AffineSum<long long>` とACLの `lazy_segtree` を使うと、区間への加算を `{1,delta}` で表せます。
各辺は `leaf(初期値)`、根の使わない位置は単位元 `e()` で初期化します。

{% raw %}
```cpp
#include <atcoder/lazysegtree>
#include <cassert>
#include <vector>
#include "blueberry/algebra/monoids.hpp"
#include "blueberry/graph/heavy-light-decomposition.hpp"
using M = blueberry::monoid::AffineSum<long long>;
using Seg = atcoder::lazy_segtree<M::S, M::op, M::e, M::F,
                                M::mapping, M::composition, M::id>;
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::HLD hld(tree);
  std::vector<M::S> base(hld.size(), M::leaf(0));
  base[hld.in(hld.root())] = M::e();  // 根に対応する辺はない
  Seg seg(base);
  auto add_from_root = [&](int v, long long delta) {
    hld.edge_query(hld.root(), v, [&](int l, int r) {
      seg.apply(l, r, M::F{1, delta});
    });
  };
  auto sum_from_root = [&](int v) {
    long long answer = 0;
    hld.edge_query(hld.root(), v, [&](int l, int r) {
      answer += seg.prod(l, r).sum;
    });
    return answer;
  };
  add_from_root(3, 4);  // 辺0-1、1-3に4を加算
  add_from_root(1, 2);  // 辺0-1に2を加算
  assert(sum_from_root(3) == 10);
  assert(sum_from_root(1) == 6);
  assert(sum_from_root(2) == 0);
  add_from_root(hld.root(), 100);  // 空パスなので更新なし
  assert(sum_from_root(hld.root()) == 0);
  assert(seg.get(hld.in(hld.root())).len == 0);
}
```
{% endraw %}

初期化はO(N)、パスへの加算・和はそれぞれO(log² N)です。
`len` はその区間が表す辺の本数です。すべてを `e()` で初期化すると、加算しても値が増えません。
加算量と本数の積、累積和が `long long` に収まるようにします。
[AOJ GRL_5_E: Range Query on a Tree II](https://onlinejudge.u-aizu.ac.jp/problems/GRL_5_E) の操作に対応します。

<h3 id="path-composite">u→vの順に関数を合成する</h3>

関数合成では順序を交換できないため、`(l,r,reversed)` を受け取る3引数callbackを使います。
callbackはu→vのパス順に呼ばれます。`reversed` は、その1区間を右から左へ読むかどうかです。
パス全体の向きや、辺の有向・無向を表す値ではありません。関数を逆関数に変える操作でもありません。

次の例は各頂点の一次関数 `a*x+b` を合成します。
区間の正順・逆順の結果を1つのACL `segtree` にまとめ、頂点の更新も1回で済ませます。
`AffineComposition::op(first,second)` はfirstを適用してからsecondを適用する順序です。

{% raw %}
```cpp
#include <atcoder/modint>
#include <atcoder/segtree>
#include <cassert>
#include <vector>
#include "blueberry/algebra/monoids.hpp"
#include "blueberry/graph/heavy-light-decomposition.hpp"
using Mint = atcoder::modint998244353;
using M = blueberry::monoid::AffineComposition<Mint>;
struct S { M::S forward, backward; };
S op(S left, S right) {
  return {M::op(left.forward, right.forward), M::op(right.backward, left.backward)};
}
S e() { return {M::e(), M::e()}; }
S leaf(M::S f) { return {f, f}; }
int main() {
  std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
  blueberry::HLD hld(tree);
  std::vector<M::S> functions{{2, 1}, {3, 4}, {5, 6}, {7, 8}};
  std::vector<S> base(hld.size(), e());
  for (int v = 0; v < hld.size(); ++v) base[hld.in(v)] = leaf(functions[v]);
  atcoder::segtree<S, op, e> seg(base);
  auto apply_path = [&](int u, int v, Mint x) {
    M::S combined = M::e();
    hld.vertex_query(u, v, [&](int l, int r, bool reversed) {
      S part = seg.prod(l, r);
      combined = M::op(combined, reversed ? part.backward : part.forward);
    });
    return combined.a * x + combined.b;
  };
  assert(apply_path(2, 3, 1) == Mint(519));  // f2→f0→f1→f3
  assert(apply_path(3, 2, 1) == Mint(501));  // 逆向きでは結果が異なる
  assert(apply_path(2, 2, 1) == Mint(11));
  seg.set(hld.in(0), leaf(M::leaf(1, 0)));  // f0を恒等関数に変更
  assert(apply_path(2, 3, 1) == Mint(267));
}
```
{% endraw %}

初期化はO(N)、1頂点の関数変更はO(log N)、パスの合成はO(log² N)です。
この例は998244353を法として計算します。
辺に関数を置く場合も、値を `edge_index` に置いて `edge_query` を呼ぶ形にできます。
ただし、上りと下りで別の関数を使う有向の操作は、この例の正順・逆順だけでは表せません。
練習問題は [Library Checker: Vertex Set Path Composite](https://judge.yosupo.jp/problem/vertex_set_path_composite) です。

## 操作一覧

ここでNは頂点数です。区間callback自身の処理時間は表の計算量に含めません。
区間を1回処理する時間がO(log N)なら、パス全体はO(log² N)です。
API節の短いコード例では、最小使用例と同じ4頂点の木で構築した `hld` を使います。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `HLD hld(tree, root = 0)` | O(N) | [開く](#construct) |
| `int hld.size() const` | O(1) | [開く](#size) |
| `int hld.root() const` | O(1) | [開く](#root) |
| `int hld.parent(v) const` | O(1) | [開く](#parent) |
| `int hld.depth(v) const` | O(1) | [開く](#depth) |
| `int hld.subtree_size(v) const` | O(1) | [開く](#subtree-size) |
| `int hld.head(v) const` | O(1) | [開く](#head) |
| `int hld.in(v) const` | O(1) | [開く](#in) |
| `int hld.out(v) const` | O(1) | [開く](#out) |
| `int hld.vertex_at(position) const` | O(1) | [開く](#vertex-at) |
| `int hld.edge_index(u, v) const` | O(1) | [開く](#edge-index) |
| `pair<int,int> hld.subtree_range(v, edge = false) const` | O(1) | [開く](#subtree-range) |
| `bool hld.is_ancestor(a, v) const` | O(1) | [開く](#is-ancestor) |
| `int hld.lca(u, v) const` | O(log N) | [開く](#lca) |
| `int hld.kth_ancestor(v, k) const` | O(log N) | [開く](#kth-ancestor) |
| `int hld.kth(v, k) const` | O(log N) | [開く](#kth) |
| `int hld.distance(u, v) const` | O(log N) | [開く](#distance) |
| `int hld.jump(u, v, k) const` | O(log N) | [開く](#jump) |
| `hld.path_query(u, v, f, edge = false) const` | O(log N) callbacks | [開く](#path-query) |
| `hld.vertex_query(u, v, f) const` | O(log N) callbacks | [開く](#vertex-path-query) |
| `hld.edge_query(u, v, f) const` | O(log N) callbacks | [開く](#edge-query) |
| `hld.vertex_query(v, f) const` | O(1) callback | [開く](#single-vertex-query) |
| `hld.subtree_query(v, f, edge = false) const` | O(1) callback | [開く](#subtree-query) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>HLD hld(tree, root = 0)</code> — O(N)</summary>

指定した木をrootを根として構築します。rootを省略すると0です。
構築時間・メモリはO(N)で、元の隣接リストを変更しません。

{% raw %}
```cpp
std::vector<std::vector<int>> tree{{1, 2}, {0, 3}, {0}, {1}};
blueberry::HLD hld(tree, 0);
```
{% endraw %}

注意点: `tree` はN>=1の連結な木、`root` は `[0,N)` です。空の木は構築できません。
木や根を変えるときは再構築が必要です。入力への参照は保持しないため、構築後は入力を破棄できます。
`HLD` は `HeavyLightDecomposition` の型別名です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int hld.size() const</code> — O(1)</summary>

頂点数 N を返します。

{% raw %}
```cpp
int n = hld.size();
```
{% endraw %}

注意点: 構築後に値は変化しません。

</details>

<details class="api-operation" id="root" markdown="1">
<summary><code>int hld.root() const</code> — O(1)</summary>

構築時に指定した根を返します。

{% raw %}
```cpp
int root = hld.root();
```
{% endraw %}

注意点: 根の変更には再構築が必要です。

</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int hld.parent(v) const</code> — O(1)</summary>

`v` の親を返します。

{% raw %}
```cpp
int p = hld.parent(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根については根自身を返します。

</details>

<details class="api-operation" id="depth" markdown="1">
<summary><code>int hld.depth(v) const</code> — O(1)</summary>

根から `v` までの辺数を返します。

{% raw %}
```cpp
int d = hld.depth(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。根の深さは0です。

</details>

<details class="api-operation" id="subtree-size" markdown="1">
<summary><code>int hld.subtree_size(v) const</code> — O(1)</summary>

`v` 自身を含む部分木の頂点数を返します。

{% raw %}
```cpp
int count = hld.subtree_size(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。部分木は構築時の根に依存し、返り値は `[1,N]` です。

</details>

<details class="api-operation" id="head" markdown="1">
<summary><code>int hld.head(v) const</code> — O(1)</summary>

`v` が属するheavy pathの、根に最も近い頂点を返します。

{% raw %}
```cpp
int chain_head = hld.head(3);
```
{% endraw %}

注意点: `v` は `[0,N)` です。分解の結果は隣接リスト順に依存し、特定のheadを選ぶ保証はありません。

</details>

<details class="api-operation" id="in" markdown="1">
<summary><code>int hld.in(v) const</code> — O(1)</summary>

`v` のHLD配列上の位置を返します。

{% raw %}
```cpp
int position = hld.in(3);
```
{% endraw %}

注意点: `v` と返り値は `[0,N)` です。同じ位置を親からvへの辺にも使いますが、根には親辺がありません。
位置は構築したHLDの中でのみ有効です。別の根や隣接順で作り直した後に使い回さないでください。

</details>

<details class="api-operation" id="out" markdown="1">
<summary><code>int hld.out(v) const</code> — O(1)</summary>

`v` の部分木区間 `[in(v),out(v))` の右端を返します。

{% raw %}
```cpp
int right = hld.out(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。`out(v)` 自体は区間に含まれず、値域は `[1,N]` です。

</details>

<details class="api-operation" id="vertex-at" markdown="1">
<summary><code>int hld.vertex_at(position) const</code> — O(1)</summary>

HLD配列の `position` に置かれた頂点を返します。`in` の逆写像です。

{% raw %}
```cpp
int v = hld.vertex_at(hld.in(3));
```
{% endraw %}

注意点: `position` は `[0,N)` です。

</details>

<details class="api-operation" id="edge-index" markdown="1">
<summary><code>int hld.edge_index(u, v) const</code> — O(1)</summary>

隣接する `u,v` を結ぶ辺を、深い側の頂点の `in` に写します。

{% raw %}
```cpp
int position = hld.edge_index(1, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)` の異なる頂点で、実際の辺の両端でなければなりません。
`edge_index(u,v)` と `edge_index(v,u)` は同じ値を返します。元の辺番号を返す操作ではありません。

</details>

<details class="api-operation" id="subtree-range" markdown="1">
<summary><code>pair&lt;int,int&gt; hld.subtree_range(v, edge = false) const</code> — O(1)</summary>

頂点部分木なら `[in(v),out(v))`、`edge=true` なら `[in(v)+1,out(v))` を返します。
辺の場合はvの部分木内で両端が完結する辺が対象です。vとその親を結ぶ辺は含みません。

{% raw %}
```cpp
auto [left, right] = hld.subtree_range(1);
```
{% endraw %}

注意点: `v` は `[0,N)`。`edge=true` の葉では空区間になります。
根を指定した辺区間には木の全辺が入り、根の使わない位置は除かれます。

</details>

<details class="api-operation" id="is-ancestor" markdown="1">
<summary><code>bool hld.is_ancestor(a, v) const</code> — O(1)</summary>

`a` が `v` の祖先（自身を含む）なら `true` を返します。

{% raw %}
```cpp
bool contains = hld.is_ancestor(1, 3);
```
{% endraw %}

注意点: `a,v` は `[0,N)`。関係は構築時の根に依存します。

</details>

<details class="api-operation" id="lca" markdown="1">
<summary><code>int hld.lca(u, v) const</code> — O(log N)</summary>

`u,v` の最も深い共通祖先（LCA）を返します。

{% raw %}
```cpp
int ancestor = hld.lca(2, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)`。結果は構築時の根に依存します。

</details>

<details class="api-operation" id="kth-ancestor" markdown="1">
<summary><code>int hld.kth_ancestor(v, k) const</code> — O(log N)</summary>

`v` から親方向へ `k` 辺進んだ祖先を返します。

{% raw %}
```cpp
int parent = hld.kth_ancestor(3, 1);
```
{% endraw %}

注意点: `v` は `[0,N)`、`0<=k<=depth(v)` です。`k==0` なら `v`、`k==depth(v)` なら根を返します。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>int hld.kth(v, k) const</code> — O(log N)</summary>

`kth_ancestor(v, k)` の競技中向け短縮名です。

{% raw %}
```cpp
int parent = hld.kth(3, 1);
```
{% endraw %}

注意点: 引数の範囲と返り値は `kth_ancestor` と同じです。

</details>

<details class="api-operation" id="distance" markdown="1">
<summary><code>int hld.distance(u, v) const</code> — O(log N)</summary>

`u-v` 間の単純パスの辺数を返します。

{% raw %}
```cpp
int edges = hld.distance(2, 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)` です。辺の重みは参照しません。`u==v` なら0、最大値はN-1です。

</details>

<details class="api-operation" id="jump" markdown="1">
<summary><code>int hld.jump(u, v, k) const</code> — O(log N)</summary>

`u` から `v` への単純パス上で、`u` を0番目とする `k` 番目の頂点を返します。

{% raw %}
```cpp
int middle = hld.jump(2, 3, 2);
```
{% endraw %}

注意点: `u,v` は `[0,N)`、`0<=k<=distance(u,v)` です。`k==0` ならu、`k==distance(u,v)` ならvを返します。
範囲外のkを許す問題では、呼び出し前に範囲を判定してください。

</details>

<details class="api-operation" id="path-query" markdown="1">
<summary><code>hld.path_query(u, v, f, edge = false) const</code> — O(log N) callbacks</summary>

u→vのパスを半開区間に分け、パス順にcallbackを呼びます。
callbackは `f(l,r)` または `f(l,r,reversed)` の形で受け取ります。
両方の形を受け取れるcallbackでは3引数版が使われます。

各区間は `0 <= l < r <= N` です。`reversed=false` は `l,l+1,...,r-1`、
`reversed=true` は `r-1,r-2,...,l` の順でその区間を辿ることを表します。
区間を受け取る順番と、区間内を読む向きは別に扱います。
`edge=false` は頂点パスで、両端とLCAを含みます。
`edge=true` はLCAの位置だけを除く辺パスです。

{% raw %}
```cpp
std::vector<int> path;
hld.path_query(2, 3, [&](int l, int r, bool reversed) {
  if (reversed) {
    for (int i = r; i > l; --i) path.push_back(hld.vertex_at(i - 1));
  } else {
    for (int i = l; i < r; ++i) path.push_back(hld.vertex_at(i));
  }
});
assert(path == std::vector<int>({2, 0, 1, 3}));
```
{% endraw %}

注意点: `u,v` は `[0,N)` です。2引数callbackでも区間を渡す順番はパス順ですが、区間内の向きは渡りません。
和・最小値などの可換な集約や、各区間へ同じ操作をする範囲更新に使います。
非可換な演算では3引数版を使い、区間内の向きも反映してください。
callbackは呼び出し中だけ使用され、保持されません。
このコード例は順番を確認するために頂点を列挙しており、処理時間はパス上の頂点数に比例します。
集約には冒頭のACLを使う例を参照してください。

</details>

<details class="api-operation" id="vertex-path-query" markdown="1">
<summary><code>hld.vertex_query(u, v, f) const</code> — O(log N) callbacks</summary>

`path_query(u,v,f,false)` の明示的な頂点パス版で、両端とLCAを含みます。

{% raw %}
```cpp
int count = 0;
hld.vertex_query(2, 3, [&](int l, int r) { count += r - l; });
assert(count == 4);
```
{% endraw %}

注意点: `u,v` は `[0,N)` です。`u==v` なら長さ1の区間に対して1回呼び、`reversed` はfalseです。
非可換な積では3引数callbackを使い、`reversed` に応じた区間積を用意してください。

</details>

<details class="api-operation" id="edge-query" markdown="1">
<summary><code>hld.edge_query(u, v, f) const</code> — O(log N) callbacks</summary>

`path_query(u,v,f,true)` の辺パス版です。各辺は深い側の頂点の `in` で表します。

{% raw %}
```cpp
int count = 0;
hld.edge_query(2, 3, [&](int l, int r) { count += r - l; });
assert(count == 3);
```
{% endraw %}

注意点: `u,v` は `[0,N)` です。`u==v` では空パスとなりcallbackを呼びません。
LCAの位置と根の使わない位置は自動で除かれるため、呼び出し側で区間を短くしないでください。
非可換なら3引数callbackを使います。callbackの順序・向き・寿命は `path_query` と同じです。

</details>

<details class="api-operation" id="single-vertex-query" markdown="1">
<summary><code>hld.vertex_query(v, f) const</code> — O(1) callback</summary>

1頂点を表す `[in(v),in(v)+1)` についてcallbackを1回呼びます。

{% raw %}
```cpp
int position = -1;
hld.vertex_query(3, [&](int l, int r) {
  position = l;
  assert(r == l + 1);
});
assert(position == hld.in(3));
```
{% endraw %}

注意点: `v` は `[0,N)` です。2頂点を指定するパス版とは異なるオーバーロードです。
3引数callbackを渡した場合の `reversed` はfalseです。callbackは保持されません。

</details>

<details class="api-operation" id="subtree-query" markdown="1">
<summary><code>hld.subtree_query(v, f, edge = false) const</code> — O(1) callback</summary>

`subtree_range(v,edge)` が空でなければ、その区間についてcallbackを1回呼びます。

{% raw %}
```cpp
int count = 0;
hld.subtree_query(1, [&](int l, int r) { count = r - l; });
assert(count == 2);  // 頂点1と3
```
{% endraw %}

注意点: `v` は `[0,N)`。`edge=true` の葉ではcallbackを呼びません。
3引数callbackの `reversed` はfalseです。非可換な集約の順序はHLDの配列順で、任意の木の走査順を指定するものではありません。
callbackは保持されません。

</details>

## 出典・検証

API設計では [NyaanNyaan/library](https://github.com/NyaanNyaan/library) の頂点/辺query分離、
[ei1333/library](https://github.com/ei1333/library/blob/master/graph/tree/heavy-light-decomposition.hpp) の短い `la`・順序付き集約、
[sotanishy/cp-library-cpp](https://github.com/sotanishy/cp-library-cpp/blob/main/tree/hld.hpp) の非可換パス積を比較しました。
実装は入力グラフを書き換えず、深い木でも再帰DFSでスタックオーバーフローしない反復構築として独自に作成しています。

Library Checkerの `Vertex Add Path Sum`、`Vertex Add Subtree Sum`、`Vertex Set Path Composite`、
`Jump on Tree` で検証し、ランダム木では全操作を愚直解と比較しています。

## 旧版からの移行

先行PR #5では `path_query` / `subtree_query` の末尾boolは `vertex=true` でした。
現行版はTree PR #9に統一し、`edge=false`（頂点）、`edge=true`（辺）です。
引数省略時の動作は同じですが、#5で末尾boolを明示したコードは値を反転してください。
パスは `vertex_query(u,v,f)` / `edge_query(u,v,f)` を使うと対象を名前で指定できます。
既存の2引数callbackも、区間を受け取る順序はu→vのパス順です。
