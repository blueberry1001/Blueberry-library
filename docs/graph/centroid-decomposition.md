---
title: Centroid Decomposition
documentation_of: //blueberry/graph/centroid-decomposition.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要

`CentroidDecomposition`は重心分解と、各頂点から重心祖先への距離・枝IDを保持します。`centroid_decomposition_thirds`は枝を概ね1/3〜2/3に二分する分解を行います。入力は連結・閉路なしの無重み無向木です。

Nを頂点数とすると、通常版は構築・メモリO(N log(N+1))、各祖先列の長さO(log(N+1))です。1/3版はcallbackを除く構築O(N log(N+1))、一時メモリO(1+N)。callback全体の配列長合計もO(N log(N+1))です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`CentroidDecomposition`](#constructor) | O(N log(N+1)) |
| [`size`](#size) | O(1) |
| [`parent`](#parent) | O(1) |
| [`ancestors`](#ancestors) | O(1) |
| [`Entry{int centroid, int distance, int branch}`](#entry) | O(1) |
| [`int Entry::centroid`](#centroid-field) | O(1) |
| [`int Entry::distance`](#distance-field) | O(1) |
| [`int Entry::branch`](#branch-field) | O(1) |
| [`centroid_decomposition_thirds`](#thirds) | O(N log(N+1) + C) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`CentroidDecomposition`](#copy-constructor) | O(N log(N+1)) |
| [`operator=`](#copy-assignment) | O(N log(N+1)) + D |
| [`CentroidDecomposition`](#move-constructor) | O(1) |
| [`operator=`](#move-assignment) | O(1 + D) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `0 <= N <= INT_MAX`とし、各無向辺を隣接リストの両方向に1回含めます。空木は通常版でsize=0、1/3版ではcallbackを呼びません。入力は書き換えません。
- 1/3版は枝を辺数で二分するもので、「全成分の頂点数がN/3以下になる重心」ではありません。分離点は仮想頂点として子へ複製します。
- 実頂点の根に対しては先に根と他の実頂点の組を処理して仮想化し、星の中心の実頂点がO(N)回現れる問題を避けます。各実頂点の登場はO(log(N+1))回ですが、仮想頂点には個別のO(log N)保証はありません。
- 無向の異なる頂点対を1回ずつ扱うための分解です。パス途中の頂点属性の動的変更には直接対応しません。
- 通常版の内部DFSは反復処理で、分解再帰の深さはO(log(N+1))です。
- ACLに対応機能はありません。返す整数はintの範囲で、Dは代入先の旧保持要素数です。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/centroid-decomposition.hpp"
int main(){
  std::vector<std::vector<int>> g{{1,2},{0},{0}};
  blueberry::CentroidDecomposition d(g);
  assert(d.size()==3 && d.parent(0)==-1);
  assert(d.ancestors(1).front().centroid==0);
  int pairs=0;
  blueberry::centroid_decomposition_thirds(g,[&](const auto& ids,const auto& parent,const auto& color){
    assert(ids.size()==parent.size() && ids.size()==color.size());
    for(int x:color) if(x==0) for(int y:color) if(y==1) ++pairs;
  });
  assert(pairs==3);
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>CentroidDecomposition(const std::vector&lt;std::vector&lt;int&gt;&gt;&amp; tree)</code> — O(N log(N+1))</summary>

通常の分解と頂点ごとの祖先・距離一覧を作る。

{% raw %}
```cpp
blueberry::CentroidDecomposition d(g);
```
{% endraw %}

注意点: 入力は構築後に破棄可能。木の形は固定。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>CentroidDecomposition copy(const CentroidDecomposition&amp; other)</code> — O(N log(N+1))</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
CentroidDecomposition copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>CentroidDecomposition&amp; operator=(const CentroidDecomposition&amp; other)</code> — O(N log(N+1)) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>CentroidDecomposition moved(CentroidDecomposition&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
CentroidDecomposition moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>CentroidDecomposition&amp; operator=(CentroidDecomposition&amp;&amp; other)</code> — O(1 + D)</summary>

旧データを破棄し所有権を移動、自身の参照を返す。

{% raw %}
```cpp
object = std::move(other);
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己ムーブ後も再代入または破棄のみとする。内部参照は無効化される。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

元の頂点数を返す。

{% raw %}
```cpp
assert(d.size()==3);
```
{% endraw %}

注意点: 空入力では0。

</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int parent(int v) const</code> — O(1)</summary>

重心分解木における親の元頂点番号を返す。

{% raw %}
```cpp
int p = d.parent(1);
```
{% endraw %}

注意点: 0 <= v < N。分解の根だけ -1。複数の重心がある場合の選び方は保証しない。

</details>

<details class="api-operation" id="ancestors" markdown="1">
<summary><code>const std::vector&lt;Entry&gt;&amp; ancestors(int v) const</code> — O(1)</summary>

分解の根から v 自身までの Entry 一覧への参照を返す。列挙は O(log(N+1))。

{% raw %}
```cpp
for(auto p:d.ancestors(1)) assert(p.distance>=0);
```
{% endraw %}

注意点: 0 <= v < N。参照はオブジェクトの代入・ムーブ・破棄まで有効。祖先の距離は元の木での距離。

</details>

<details class="api-operation" id="entry" markdown="1">
<summary><code>Entry{int centroid, int distance, int branch}</code> — O(1)</summary>

3整数の集約型。コピーした Entry は独立した値。

{% raw %}
```cpp
blueberry::CentroidDecomposition::Entry p{0,1,0};
```
{% endraw %}

注意点: 手動生成した Entry は分解を変更しない。

</details>

<details class="api-operation" id="centroid-field" markdown="1">
<summary><code>int Entry::centroid</code> — O(1)</summary>

重心祖先の元頂点 ID。

{% raw %}
```cpp
int c = d.ancestors(1).front().centroid;
```
{% endraw %}

注意点: 0 <= centroid < N。

</details>

<details class="api-operation" id="distance-field" markdown="1">
<summary><code>int Entry::distance</code> — O(1)</summary>

元の木で v から重心までの辺数。

{% raw %}
```cpp
int distance = d.ancestors(1).front().distance;
```
{% endraw %}

注意点: 自身のエントリだけ0。

</details>

<details class="api-operation" id="branch-field" markdown="1">
<summary><code>int Entry::branch</code> — O(1)</summary>

重心を削除した時に v が属する成分の、重心ごとに独立した ID。

{% raw %}
```cpp
int branch = d.ancestors(1).front().branch;
```
{% endraw %}

注意点: 自身の重心では -1。他の枝は0以上の連続整数。異なる重心間の ID を比較しない。

</details>

<details class="api-operation" id="thirds" markdown="1">
<summary><code>void centroid_decomposition_thirds(const std::vector&lt;std::vector&lt;int&gt;&gt;&amp; tree, F callback)</code> — O(N log(N+1) + C)</summary>

callback(vertices,parent,color) を呼ぶ。3配列は同じ長さ。vertices は元頂点 ID、parent[0]=-1、i>0 では0 <= parent[i] < i。color は0/1が実頂点、-1が仮想頂点。異なる色の実頂点対は分離点 vertices[0] を通り、全 callback を通して各異なる頂点対がちょうど1回現れる。C は利用者の callback の合計時間。

{% raw %}
```cpp
blueberry::centroid_decomposition_thirds(g, [](const auto& vertices, const auto& parent, const auto& color) {
  assert(vertices.size()==parent.size() && vertices.size()==color.size());
});
```
{% endraw %}

注意点: 配列への参照は callback 中だけ有効。入力の元頂点 ID のコピーを保存すること。距離は parent を辿る辺数で求める。自己対は通知しない。実頂点が一方の色にいない場合は通知しない。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

[maspypy の重心分解・1/3版解説](https://maspypy.com/重心分解・1-3重心分解のお絵描き) と [実装](https://maspypy.github.io/library/graph/centroid_decomposition.hpp) を比較し独立実装した。実/仮想頂点の区別が重要。公式 `frequency_table_of_tree_distance` を1/3版+ACL convolution_llで検証し、通常版は2方向の contour verify で検証。ランダムテストで全頂点対が1回、経路が分離点を通ること、星の実頂点登場回数を検証する。

</details>
