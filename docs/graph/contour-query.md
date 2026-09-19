---
title: Tree Contour Query
documentation_of: //blueberry/graph/contour-query.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

`blueberry::ContourQuery<T>` は頂点加算・距離帯の和、`blueberry::ContourAdd<T>` は距離帯加算・頂点取得を提供する。両者は別の状態であり、一方の更新は他方には反映されない。

N 頂点の固定・無重み・無向木。距離帯は辺数による `[l,r)`、`0 <= l <= r <= INT_MAX`。距離0を含めれば自身も対象。r が木の最大距離より大きくてもよい。空の距離帯は和0・作用なし。全頂点の初期値は0。非空の木は連結・閉路なしで、各辺を隣接リストの両方向に1回ずつ含む。空木も構築可能だが頂点操作は禁止。

T は ACL fenwick_tree が対応する整数または static_modint。加算・減算・ゼロ構築が必要で、整数演算の結果と更新値の負数が T の範囲に収まること。構築 O(N log(N+1))、メモリ O(1+N log(N+1))、各更新・問い合わせ O(log²(N+1))。内部の平坦化された配列長も INT_MAX 以下。通常の重心分解で祖先全体から同じ枝を差し引く。Fenwick 部分は ACL を利用する。

元の木・値への外部参照は保持せず、入力は構築後に破棄可能。返り値は値なので内部参照の無効化はない。D は代入先の旧保持要素数。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/contour-query.hpp"
int main(){
  std::vector<std::vector<int>> g{{1,2},{0},{0}};
  blueberry::ContourQuery<long long> q(g);
  q.add(1,5);q.add(2,7);
  assert(q.sum(0,1,2)==12 && q.sum(1,0,1)==5);
  blueberry::ContourAdd<long long> a(g);
  a.add(1,0,2,3);
  assert(a.get(0)==3 && a.get(1)==3 && a.get(2)==0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `ContourQuery<T>(const std::vector<std::vector<int>>& tree)` | O(N log(N+1)) | [開く](#query-constructor) |
| `ContourQuery<T> copy(const ContourQuery<T>& other)` | O(N log(N+1)) | [開く](#query-copy-constructor) |
| `ContourQuery<T>& operator=(const ContourQuery<T>& other)` | O(N log(N+1)) + D | [開く](#query-copy-assignment) |
| `ContourQuery<T> moved(ContourQuery<T>&& other)` | O(1) | [開く](#query-move-constructor) |
| `ContourQuery<T>& operator=(ContourQuery<T>&& other)` | O(1 + D) | [開く](#query-move-assignment) |
| `int ContourQuery<T>::size() const` | O(1) | [開く](#query-size) |
| `void ContourQuery<T>::add(int v, T value)` | O(log²(N+1)) | [開く](#query-add) |
| `T ContourQuery<T>::sum(int v, int l, int r)` | O(log²(N+1)) | [開く](#query-sum) |
| `ContourAdd<T>(const std::vector<std::vector<int>>& tree)` | O(N log(N+1)) | [開く](#add-constructor) |
| `ContourAdd<T> copy(const ContourAdd<T>& other)` | O(N log(N+1)) | [開く](#add-copy-constructor) |
| `ContourAdd<T>& operator=(const ContourAdd<T>& other)` | O(N log(N+1)) + D | [開く](#add-copy-assignment) |
| `ContourAdd<T> moved(ContourAdd<T>&& other)` | O(1) | [開く](#add-move-constructor) |
| `ContourAdd<T>& operator=(ContourAdd<T>&& other)` | O(1 + D) | [開く](#add-move-assignment) |
| `int ContourAdd<T>::size() const` | O(1) | [開く](#add-size) |
| `void ContourAdd<T>::add(int v, int l, int r, T value)` | O(log²(N+1)) | [開く](#add-add) |
| `T ContourAdd<T>::get(int v)` | O(log²(N+1)) | [開く](#add-get) |

<details class="api-operation" id="query-constructor" markdown="1">
<summary><code>ContourQuery&lt;T&gt;(const std::vector&lt;std::vector&lt;int&gt;&gt;&amp; tree)</code> — O(N log(N+1))</summary>

頂点加算・距離帯和用のゼロ状態を構築する。

{% raw %}
```cpp
blueberry::ContourQuery<long long> q(g);
```
{% endraw %}

注意点: 初期値がある場合は各頂点に add する（全体 O(N log²(N+1))）。

</details>

<details class="api-operation" id="query-copy-constructor" markdown="1">
<summary><code>ContourQuery&lt;T&gt; copy(const ContourQuery&lt;T&gt;&amp; other)</code> — O(N log(N+1))</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
ContourQuery<T> copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="query-copy-assignment" markdown="1">
<summary><code>ContourQuery&lt;T&gt;&amp; operator=(const ContourQuery&lt;T&gt;&amp; other)</code> — O(N log(N+1)) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="query-move-constructor" markdown="1">
<summary><code>ContourQuery&lt;T&gt; moved(ContourQuery&lt;T&gt;&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
ContourQuery<T> moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="query-move-assignment" markdown="1">
<summary><code>ContourQuery&lt;T&gt;&amp; operator=(ContourQuery&lt;T&gt;&amp;&amp; other)</code> — O(1 + D)</summary>

旧データを破棄し所有権を移動、自身の参照を返す。

{% raw %}
```cpp
object = std::move(other);
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己ムーブ後も再代入または破棄のみとする。内部参照は無効化される。

</details>

<details class="api-operation" id="query-size" markdown="1">
<summary><code>int ContourQuery&lt;T&gt;::size() const</code> — O(1)</summary>

頂点数を返す。

{% raw %}
```cpp
assert(q.size()==3);
```
{% endraw %}

注意点: 空木では0。

</details>

<details class="api-operation" id="query-add" markdown="1">
<summary><code>void ContourQuery&lt;T&gt;::add(int v, T value)</code> — O(log²(N+1))</summary>

頂点 v の値に value を加える。

{% raw %}
```cpp
q.add(1,5);
```
{% endraw %}

注意点: 0 <= v < N。負の更新値も表現可能なら使える。

</details>

<details class="api-operation" id="query-sum" markdown="1">
<summary><code>T ContourQuery&lt;T&gt;::sum(int v, int l, int r)</code> — O(log²(N+1))</summary>

v からの距離が [l,r) の頂点値の総和を返す。

{% raw %}
```cpp
assert(q.sum(0,1,2)==5);
```
{% endraw %}

注意点: 0 <= v < N、0 <= l <= r。空帯は0。ACL APIのため非constだが論理的な値は変わらない。

</details>

<details class="api-operation" id="add-constructor" markdown="1">
<summary><code>ContourAdd&lt;T&gt;(const std::vector&lt;std::vector&lt;int&gt;&gt;&amp; tree)</code> — O(N log(N+1))</summary>

距離帯加算・頂点取得用のゼロ状態を構築する。

{% raw %}
```cpp
blueberry::ContourAdd<long long> a(g);
```
{% endraw %}

注意点: 初期値は利用側で別配列を保持し get に足してもよい。

</details>

<details class="api-operation" id="add-copy-constructor" markdown="1">
<summary><code>ContourAdd&lt;T&gt; copy(const ContourAdd&lt;T&gt;&amp; other)</code> — O(N log(N+1))</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
ContourAdd<T> copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="add-copy-assignment" markdown="1">
<summary><code>ContourAdd&lt;T&gt;&amp; operator=(const ContourAdd&lt;T&gt;&amp; other)</code> — O(N log(N+1)) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="add-move-constructor" markdown="1">
<summary><code>ContourAdd&lt;T&gt; moved(ContourAdd&lt;T&gt;&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
ContourAdd<T> moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="add-move-assignment" markdown="1">
<summary><code>ContourAdd&lt;T&gt;&amp; operator=(ContourAdd&lt;T&gt;&amp;&amp; other)</code> — O(1 + D)</summary>

旧データを破棄し所有権を移動、自身の参照を返す。

{% raw %}
```cpp
object = std::move(other);
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己ムーブ後も再代入または破棄のみとする。内部参照は無効化される。

</details>

<details class="api-operation" id="add-size" markdown="1">
<summary><code>int ContourAdd&lt;T&gt;::size() const</code> — O(1)</summary>

頂点数を返す。

{% raw %}
```cpp
assert(a.size()==3);
```
{% endraw %}

注意点: 空木では0。

</details>

<details class="api-operation" id="add-add" markdown="1">
<summary><code>void ContourAdd&lt;T&gt;::add(int v, int l, int r, T value)</code> — O(log²(N+1))</summary>

v から距離帯 [l,r) の全頂点に value を加える。

{% raw %}
```cpp
a.add(1,0,2,3);
```
{% endraw %}

注意点: 0 <= v < N、0 <= l <= r。空帯では状態変化なし。

</details>

<details class="api-operation" id="add-get" markdown="1">
<summary><code>T ContourAdd&lt;T&gt;::get(int v)</code> — O(log²(N+1))</summary>

頂点 v に届いた加算の合計を返す。

{% raw %}
```cpp
assert(a.get(1)==3);
```
{% endraw %}

注意点: 0 <= v < N。更新前は0。

</details>

## 出典・検証

[maspypy の contour 範囲分解](https://maspypy.github.io/library/graph/ds/contour_query_range.hpp) の1/3版と、通常重心分解の全体−枝方式を比較。本 API は加法群を対象にし、簡潔な通常版+ACL Fenwickを採用した。公式 `vertex_add_range_contour_sum_on_tree` / `vertex_get_range_contour_add_on_tree` に対応する verify を追加。星・パス・ランダム木で BFS 距離を使う愚直更新・集約と比較する。
