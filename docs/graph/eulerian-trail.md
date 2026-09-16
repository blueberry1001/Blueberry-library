---
title: Eulerian Trail
documentation_of: //blueberry/graph/eulerian-trail.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

有向・無向多重グラフで、すべての辺をちょうど一度使うオイラー路を求めます。
`EulerianTrail<true>` が有向、`EulerianTrail<false>` が無向です。Nを頂点数、Mを入力辺数とすると構築時間・メモリは O(N+M) です。
ACLにはない機能です。Hierholzer法をvectorの明示的なスタックで実行し、長い道でも再帰スタックに依存しません。

入力は `vector<pair<int,int>>` で、有向では(u,v)がu→vです。入力の0-based位置が辺IDになります。
N>=0、M<INT_MAX、0<=u,v<N を満たしてください。自己ループ・多重辺・孤立頂点を許可します。
入力を変更せず構築時に解きます。次数条件に加えて使用した辺の総数を検査するため、辺を持つ連結成分が複数なら存在しないと判定します。

start=-1は始点自動選択、それ以外は0<=start<Nで指定します。指定頂点が始点になれない場合はexists()==falseです。
N>0,M=0なら任意の頂点が有効で、自動選択では頂点0だけを返します。N=M=0は本APIでは空の有効な道と定義し、両列を空で返します（start=-1のみ）。
解の辞書順最小性・選択順は保証しません。intの次数差・辺IDとsize_tの隣接配列添字を用います。十分なメモリを確保できることが前提です。
返すconst参照はオブジェクトの破棄・代入・移動で無効になり得ます。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/eulerian-trail.hpp"
int main() {
  blueberry::EulerianTrail<true> directed(3, {{0, 1}, {1, 2}});
  assert(directed.exists());
  assert(directed.vertices().front() == 0);
  assert(directed.vertices().back() == 2);
  assert(directed.edges().size() == 2);
  blueberry::EulerianTrail<false> undirected(2, {{0, 1}, {0, 1}}, 1);
  assert(undirected.exists());
  assert(undirected.vertices().front() == 1);
  blueberry::EulerianTrail<true> impossible(2, {{0, 0}, {1, 1}});
  assert(!impossible.exists());
}
```
{% endraw %}


## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `EulerianTrail<Directed> trail(n, edges, start = -1)` | O(N+M) | [開く](#construct) |
| `bool trail.exists() const` | O(1) | [開く](#exists) |
| `const vector<int>& trail.vertices() const` | O(1) | [開く](#vertices) |
| `const vector<int>& trail.edges() const` | O(1) | [開く](#edges) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>EulerianTrail&lt;Directed&gt; trail(n, edges, start = -1)</code> — O(N+M)</summary>

boolテンプレート引数で方向を指定し、intの頂点数・辺列const参照・任意の始点から道を求めます。有向では出次数−入次数、無向では次数の偶奇を検査します。自動選択は開いた道の端点、閉じた道では辺を持つ頂点を選びます。

{% raw %}
```cpp
blueberry::EulerianTrail<true> trail(3, {{0, 1}, {1, 2}});
blueberry::EulerianTrail<false> reverse(3, {{0, 1}, {1, 2}}, 2);
```
{% endraw %}

注意点: 不正な頂点番号・負のNは前提違反です。範囲内だが道の始点になれない指定は正常な不存在結果です。入力の寿命に依存せず、自分自身への辺も消費回数は1回です。

</details>


<details class="api-operation" id="exists" markdown="1">
<summary><code>bool trail.exists() const</code> — O(1)</summary>

指定条件で全辺を一度ずつ通る道があればtrue、なければfalseです。

{% raw %}
```cpp
if (trail.exists()) { assert(trail.edges().size() == 2); }
```
{% endraw %}

注意点: 空辺列でも有効な道はあります。vertices().empty()だけで存在判定をしないでください。

</details>


<details class="api-operation" id="vertices" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; trail.vertices() const</code> — O(1)</summary>

通る順の頂点列をconst参照で返します。成功かつN>0なら長さM+1です。閉路の場合は最初と最後が同じ頂点です。

{% raw %}
```cpp
const auto& vertices = trail.vertices();
assert(vertices.front() == 0);
```
{% endraw %}

注意点: 不存在またはN=0なら空。front/back/添字アクセスの前に必要な長さを確認してください。参照の寿命は本体に従属します。

</details>


<details class="api-operation" id="edges" markdown="1">
<summary><code>const vector&lt;int&gt;&amp; trail.edges() const</code> — O(1)</summary>

通る順の入力辺IDをconst参照で返します。成功時は[0,M)の置換です。第i要素はvertices()[i]からvertices()[i+1]へ渡る辺で、有向時は入力方向に一致します。

{% raw %}
```cpp
const auto& ids = trail.edges();
assert(ids.size() == 2);
```
{% endraw %}

注意点: 不存在またはM=0なら空。多重辺でも入力位置が異なる別の辺として識別されます。入力ペア列への参照ではありません。

</details>

## 出典・検証

- Library Checkerの [有向オイラー路](https://judge.yosupo.jp/problem/eulerian_trail_directed)・[無向オイラー路](https://judge.yosupo.jp/problem/eulerian_trail_undirected) で検証します。
- [CP-Algorithms](https://lib.cp-algorithms.com/cp-algo/graph/euler.hpp.html)、Fastest調査対象 [286963](https://judge.yosupo.jp/submission/286963)・[286968](https://judge.yosupo.jp/submission/286968) の明示スタック・次数検査・辺消費数検査を分析し、標準的なHierholzer法を独自実装しました。
- 小入力の辺を使う全経路を探索して存在判定を比較し、全始点指定・自己ループ・多重辺・切り離された閉路・空入力・長いパスを検証します。公開コードのコピーは行っていません。
- 比較条件と測定結果は `benchmark/results/lc-expansion-graph/` に保存します。
