---
title: Segment Tree
documentation_of: //blueberry/data-structure/segment-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

モノイドの一点更新・区間積を扱います。要素数を $N$ とし、演算 `op` は結合則を満たし、
`identity` は単位元である必要があります。構築は $O(N)$、`set`・`product`・`prod` は
$O(\log N)$、`get`・`all_product`・`all_prod`・`size` は $O(1)$ です。区間は `[l,r)` です。

## 最小使用例

{% raw %}
```cpp
#include <algorithm>
#include <cassert>
#include <vector>
#include "blueberry/data-structure/segment-tree.hpp"
int main() {
  auto add = [](int a, int b) { return a + b; };
  blueberry::SegmentTree<int, decltype(add)> seg(std::vector<int>{1, 2, 3}, add, 0);
  assert(seg.prod(0, 2) == 3);
  seg.set(1, 5);
  assert(seg.all_prod() == 9);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SegmentTree<S, Op> seg(n, op, identity)` | O(N) | [開く](#construct-size) |
| `SegmentTree<S, Op> seg(values, op, identity)` | O(N) | [開く](#construct-values) |
| `void seg.set(i, value)` | O(log N) | [開く](#set) |
| `const S& seg.get(i) const` | O(1) | [開く](#get) |
| `S seg.product(l, r) const` | O(log N) | [開く](#product) |
| `S seg.prod(l, r) const` | O(log N) | [開く](#prod) |
| `const S& seg.all_product() const` | O(1) | [開く](#all-product) |
| `const S& seg.all_prod() const` | O(1) | [開く](#all-prod) |
| `int seg.size() const` | O(1) | [開く](#size) |

<details class="api-operation" id="construct-size" markdown="1">
<summary><code>SegmentTree&lt;S, Op&gt; seg(n, op, identity)</code> — O(N)</summary>

単位元だけを入れた長さ $N$ の木を作ります。

{% raw %}
```cpp
auto op = [](int a, int b) { return a + b; };
blueberry::SegmentTree<int, decltype(op)> seg(20, op, 0);
```
{% endraw %}

注意点: `n` は0以上。`op` は結合的で、`identity` は単位元でなければなりません。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>SegmentTree&lt;S, Op&gt; seg(values, op, identity)</code> — O(N)</summary>

配列を葉に置き、内部ノードをボトムアップに構築します。

{% raw %}
```cpp
auto op = [](int a, int b) { return a + b; };
blueberry::SegmentTree<int, decltype(op)> seg(std::vector<int>{1, 2, 3}, op, 0);
```
{% endraw %}

注意点: vectorの後の変更は反映されません。`S` のコピーと `op` の返り値が必要です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void seg.set(i, value)</code> — O(log N)</summary>

位置 `i` の値を置き換え、祖先ノードを更新します。

{% raw %}
```cpp
seg.set(1, 10);
```
{% endraw %}

注意点: `i` は `[0,N)`。加算ではなく代入です。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>const S& seg.get(i) const</code> — O(1)</summary>

位置 `i` の葉へのconst参照を返します。

{% raw %}
```cpp
int value = seg.get(1);
```
{% endraw %}

注意点: `i` は `[0,N)`。木を変更する操作後は以前の参照を保持しないでください。

</details>

<details class="api-operation" id="product" markdown="1">
<summary><code>S seg.product(l, r) const</code> — O(log N)</summary>

半開区間 `[l,r)` を添字順に畳み込みます。可換でない演算にも対応します。

{% raw %}
```cpp
int value = seg.product(0, 2);
```
{% endraw %}

注意点: `0<=l<=r<=N`。空区間は単位元を返します。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S seg.prod(l, r) const</code> — O(log N)</summary>

`product` の短い別名です。ACL-styleなコードから呼び出しやすくしています。

{% raw %}
```cpp
int value = seg.prod(0, 2);
```
{% endraw %}

注意点: 引数・順序・空区間の扱いは `product` と同じです。

</details>

<details class="api-operation" id="all-product" markdown="1">
<summary><code>const S& seg.all_product() const</code> — O(1)</summary>

全要素 `[0,N)` の積へのconst参照を返します。

{% raw %}
```cpp
int total = seg.all_product();
```
{% endraw %}

注意点: 返り値の参照は木の変更で無効になることがあります。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>const S& seg.all_prod() const</code> — O(1)</summary>

`all_product` の短い別名です。

{% raw %}
```cpp
int total = seg.all_prod();
```
{% endraw %}

注意点: `all_product` と同じくconst参照を返します。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int seg.size() const</code> — O(1)</summary>

葉として管理する要素数 $N$ を返します。

{% raw %}
```cpp
int n = seg.size();
```
{% endraw %}

注意点: 内部の2冪サイズではなく、指定した要素数です。

</details>
