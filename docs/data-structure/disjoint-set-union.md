---
title: Disjoint Set Union
documentation_of: //blueberry/data-structure/disjoint-set-union.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

互いに素な集合を併合し、同じ集合かどうかを判定します。要素数を $N$ とすると、
union by size と経路圧縮により `merge`・`leader`・`same`・`component_size` は償却
$O(\alpha(N))$、`groups` は $O(N\alpha(N))$ です。添字は `[0,N)` です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/disjoint-set-union.hpp"
int main() {
  blueberry::DisjointSetUnion dsu(4);
  dsu.merge(0, 1);
  assert(dsu.same(0, 1));
  assert(dsu.comp_size(0) == 2);
  assert(dsu.size() == 4);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DisjointSetUnion dsu(n)` | O(N) | [開く](#construct) |
| `int dsu.leader(v)` | 償却 O(α(N)) | [開く](#leader) |
| `bool dsu.merge(u, v)` | 償却 O(α(N)) | [開く](#merge) |
| `bool dsu.same(u, v)` | 償却 O(α(N)) | [開く](#same) |
| `int dsu.component_size(v)` | 償却 O(α(N)) | [開く](#component-size) |
| `int dsu.comp_size(v)` | 償却 O(α(N)) | [開く](#comp-size) |
| `int dsu.size()` | O(1) | [開く](#size) |
| `vector<vector<int>> dsu.groups()` | O(N α(N)) | [開く](#groups) |

以下の詳細には返り値・使用例・注意点をまとめています。

<details class="api-operation" id="construct" markdown="1">
<summary><code>DisjointSetUnion dsu(n)</code> — O(N)</summary>

各要素が単独集合の状態で初期化します。

{% raw %}
```cpp
blueberry::DisjointSetUnion dsu(10);
```
{% endraw %}

注意点: `n` は0以上。要素番号は `[0,n)` です。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int dsu.leader(v)</code> — 償却 O(α(N))</summary>

要素 `v` が属する集合の代表要素を返します。経路圧縮で以後の操作が速くなります。

{% raw %}
```cpp
int representative = dsu.leader(3);
```
{% endraw %}

注意点: `v` は `[0,N)`。代表番号は集合内の任意の要素で、併合後に変わることがあります。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>bool dsu.merge(u, v)</code> — 償却 O(α(N))</summary>

`u` と `v` の集合を併合し、新たに併合できたとき `true` を返します。

{% raw %}
```cpp
bool changed = dsu.merge(0, 2);
```
{% endraw %}

注意点: 同じ集合への呼び出しは `false` で、集合状態は変わりません。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool dsu.same(u, v)</code> — 償却 O(α(N))</summary>

二つの要素が同じ集合に属するかを返します。

{% raw %}
```cpp
assert(dsu.same(0, 2));
```
{% endraw %}

注意点: 両方の要素番号が有効でなければなりません。

</details>

<details class="api-operation" id="component-size" markdown="1">
<summary><code>int dsu.component_size(v)</code> — 償却 O(α(N))</summary>

`v` を含む集合の要素数を返します。

{% raw %}
```cpp
int count = dsu.component_size(0);
```
{% endraw %}

注意点: `v` は `[0,N)`。`merge` 後は値が変わります。

</details>

<details class="api-operation" id="comp-size" markdown="1">
<summary><code>int dsu.comp_size(v)</code> — 償却 O(α(N))</summary>

`component_size` の短い別名です。新しいコードで入力を短くできます。

{% raw %}
```cpp
int count = dsu.comp_size(0);
```
{% endraw %}

注意点: 引数・返り値・計算量は `component_size` と同じです。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int dsu.size()</code> — O(1)</summary>

管理している要素数 $N$ を返します。

{% raw %}
```cpp
int n = dsu.size();
```
{% endraw %}

注意点: 連結成分数ではありません。成分数は `groups().size()` で得られます。

</details>

<details class="api-operation" id="groups" markdown="1">
<summary><code>vector&lt;vector&lt;int&gt;&gt; dsu.groups()</code> — O(N α(N))</summary>

現在の各連結成分を要素番号の配列として列挙します。

{% raw %}
```cpp
auto components = dsu.groups();
```
{% endraw %}

注意点: 返り値の成分順・各成分内の順序に依存しないでください。返り値の変更はDSUに反映されません。

</details>
