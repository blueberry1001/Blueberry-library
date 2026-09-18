---
title: Static Range Distinct
documentation_of: //blueberry/data-structure/static-range-distinct.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

不変配列の半開区間に現れる異なる値の個数をオンラインで返します。同値性はTのoperator<が定めるstrict weak orderingで判断し、コピーと比較をO(1)とします。Nは配列長でINT_MAX未満。直前の同値要素の位置をWaveletMatrixに格納し、構築O(N log(N+1))、保持メモリO(N log(N+1)) bits + O(N) words。入力への参照を保持せず、空入力も許可。ACL単体にはこの用途のAPIがありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/static-range-distinct.hpp"
int main() {
  blueberry::StaticRangeDistinct<int> d(std::vector<int>{2,1,2,3});
  assert(d.size()==4 && d.count(0,4)==3);
  assert(d.count(1,3)==2 && d.count(2,2)==0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `StaticRangeDistinct<T>()` | O(1) | [開く](#op-0) |
| `StaticRangeDistinct<T>(const std::vector<T>& a)` | O(N log(N+1)) | [開く](#op-1) |
| `int d.size() const` | O(1) | [開く](#op-2) |
| `int d.count(int l, int r) const` | O(log(N+1)) | [開く](#op-3) |
| `StaticRangeDistinct(const StaticRangeDistinct& other)` | O(N log(N+1)) | [開く](#value-0) |
| `StaticRangeDistinct(StaticRangeDistinct&& other)` | O(1) | [開く](#value-1) |
| `StaticRangeDistinct& operator=(const StaticRangeDistinct& other)` | O(N log(N+1)) + O(旧状態の破棄) | [開く](#value-2) |
| `StaticRangeDistinct& operator=(StaticRangeDistinct&& other)` | O(1) + O(旧状態の破棄) | [開く](#value-3) |

<details class="api-operation" id="op-0" markdown="1">
<summary><code>StaticRangeDistinct&lt;T&gt;()</code> — O(1)</summary>

空配列を構築します。

{% raw %}
```cpp
blueberry::StaticRangeDistinct<int> d;
```
{% endraw %}

注意点: count(0,0)は0。

</details>

<details class="api-operation" id="op-1" markdown="1">
<summary><code>StaticRangeDistinct&lt;T&gt;(const std::vector&lt;T&gt;&amp; a)</code> — O(N log(N+1))</summary>

aの静的な異なる値の個数を前計算します。

{% raw %}
```cpp
blueberry::StaticRangeDistinct<int> d(std::vector<int>{1,2,1});
```
{% endraw %}

注意点: 構築後のaの更新・破棄はdに影響しません。内部配列の更新APIはありません。

</details>

<details class="api-operation" id="op-2" markdown="1">
<summary><code>int d.size() const</code> — O(1)</summary>

構築した配列長を返します。

{% raw %}
```cpp
int n=d.size();
```
{% endraw %}

注意点: 空配列は0。

</details>

<details class="api-operation" id="op-3" markdown="1">
<summary><code>int d.count(int l, int r) const</code> — O(log(N+1))</summary>

[l,r)内の異なる値の数を返します。

{% raw %}
```cpp
int n=d.count(0,d.size());
```
{% endraw %}

注意点: 0<=l<=r<=size()。空区間は0。返り値は0以上r-l以下でintに収まります。

</details>

<details class="api-operation" id="value-0" markdown="1">
<summary><code>StaticRangeDistinct(const StaticRangeDistinct&amp; other)</code> — O(N log(N+1))</summary>

otherの全状態を独立にコピーします。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
blueberry::StaticRangeDistinct<int> copy(original);
```
{% endraw %}

注意点: コピー先は独立した状態を所有し、コピー元の変更・破棄に影響されません。入力への参照は保持しません。

</details>

<details class="api-operation" id="value-1" markdown="1">
<summary><code>StaticRangeDistinct(StaticRangeDistinct&amp;&amp; other)</code> — O(1)</summary>

otherの記憶領域を移動します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
blueberry::StaticRangeDistinct<int> moved(std::move(original));
```
{% endraw %}

注意点: 移動元は破棄または再代入のみ行ってください。移動先が状態を所有します。

</details>

<details class="api-operation" id="value-2" markdown="1">
<summary><code>StaticRangeDistinct&amp; operator=(const StaticRangeDistinct&amp; other)</code> — O(N log(N+1)) + O(旧状態の破棄)</summary>

otherの全状態で置き換え、自分への参照を返します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
copy = original;
```
{% endraw %}

注意点: コピー元は不変で、代入後も両者は独立しています。代入前の配列は置き換えられ、以後の問い合わせは新しい配列を対象にします。

</details>

<details class="api-operation" id="value-3" markdown="1">
<summary><code>StaticRangeDistinct&amp; operator=(StaticRangeDistinct&amp;&amp; other)</code> — O(1) + O(旧状態の破棄)</summary>

otherの記憶領域を移して置き換え、自分への参照を返します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
copy = std::move(original);
```
{% endraw %}

注意点: 移動元は破棄または再代入のみ行ってください。移動先の以前の配列は置き換えられます。

</details>

## 出典・検証

[Library Checker](https://judge.yosupo.jp/problem/static_range_count_distinct) と全区間のset比較で検証。[suisenのoffline Fenwick設計](https://suisen-cp.github.io/cp-library-cpp/test/src/range_query/static%20range_count_distinct/static_range_count_distinct.test.cpp) と既存WaveletMatrixへの変換を比較し、即時クエリ可能な後者を採用。測定: [DS report](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-ds.md)。
