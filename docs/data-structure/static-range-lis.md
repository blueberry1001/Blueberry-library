---
title: Static Range LIS
documentation_of: //blueberry/data-structure/static-range-lis.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`StaticRangeLIS<T>` は静的な列の任意の半開区間の**狭義**最長増加部分列の長さを返します。
ACL には区間 LIS はありません。N は入力長。構築 O(N log²(N+1))、問い合わせ O(log(N+1))、メモリ O(N log(N+1))。
semi-local LIS の subunit Monge 積で点集合を構築し、既存 WaveletMatrix で数えます。
`T` の `<` は strict weak ordering。比較を O(1) と数えます。同値要素は添字の逆順で順位化し、重複を増加として数えません。
入力はコピーによる索引化で、元配列の寿命に依存しません。更新はできません。空列・空区間の答えは 0。
長さと答えは int、N<=INT_MAX。値の加算はしません。

Monge積の再帰は、構築ごとに一度確保する O(N) の作業領域を使い回します。
一つの長さnのframeは n+5⌈n/2⌉ 個のintを使い、同時に処理しない兄弟は共有します。
同時に必要な再帰パスの容量を先に計算し、再帰中の確保と部分列vectorの再確保を省きます。
作業領域はconstructorローカルで、WaveletMatrix構築前に解放します。
インスタンス間の共有状態・入力への参照は追加しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/static-range-lis.hpp"
int main() {
  blueberry::StaticRangeLIS<int> ds(std::vector<int>{3,1,2,2,4});
  assert(ds.lis(0,5)==3);
  assert(ds.lis(2,4)==1);
  assert(ds.lis(3,3)==0);
}
```
{% endraw %}

## 操作一覧

以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `StaticRangeLIS<T>()` | O(1) | [開く](#default) |
| `StaticRangeLIS<T>(const vector<T>& values)` | O(N log²(N+1)) | [開く](#vector) |
| `int object.size() const` | O(1) | [開く](#size) |
| `int object.lis(int l, int r) const` | O(log(N+1)) | [開く](#lis) |
| `Type(const Type& other)` | O(M) | [開く](#copy-constructor) |
| `Type(Type&& other)` | O(1) | [開く](#move-constructor) |
| `Type& object.operator=(const Type& other)` | O(M + D) | [開く](#copy-assignment) |
| `Type& object.operator=(Type&& other)` | O(1 + D) | [開く](#move-assignment) |

<details class="api-operation" id="default" markdown="1">
<summary><code>StaticRangeLIS&lt;T&gt;()</code> — O(1)</summary>

空列の索引を作ります。

{% raw %}
```cpp
blueberry::StaticRangeLIS<int> ds;
```
{% endraw %}

注意点: size()==0、lis(0,0)==0。

</details>

<details class="api-operation" id="vector" markdown="1">
<summary><code>StaticRangeLIS&lt;T&gt;(const vector&lt;T&gt;&amp; values)</code> — O(N log²(N+1))</summary>

入力列を前処理します。

{% raw %}
```cpp
blueberry::StaticRangeLIS<int> ds(std::vector<int>{2,1,2});
```
{% endraw %}

注意点: 同値要素を複数使わない狭義 LIS。入力は後から変更・破棄可能です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

入力列の長さです。

{% raw %}
```cpp
int n=object.size();
```
{% endraw %}

注意点: 空列は0。

</details>

<details class="api-operation" id="lis" markdown="1">
<summary><code>int object.lis(int l, int r) const</code> — O(log(N+1))</summary>

[l,r) の狭義 LIS 長を返します。

{% raw %}
```cpp
int answer=object.lis(0,object.size());
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間0、要素列の復元は行いません。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>Type(const Type&amp; other)</code> — O(M)</summary>

全履歴・補助索引を独立にコピーします。

{% raw %}
```cpp
Type copied(object);
```
{% endraw %}

注意点: 以後の更新は互いに影響しません。M は保持する全要素・ノード・版数です。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>Type(Type&amp;&amp; other)</code> — O(1)</summary>

内部領域の所有権を移します。

{% raw %}
```cpp
Type moved(std::move(object));
```
{% endraw %}

注意点: <utility> が必要です。ムーブ元は破棄または再代入してから使用してください。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(const Type&amp; other)</code> — O(M + D)</summary>

コピーで置き換え、*this を返します。

{% raw %}
```cpp
Type copied; copied = object;
```
{% endraw %}

注意点: D は代入前の宛先保持量。旧領域の破棄も含みます。自己代入は有効です。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(Type&amp;&amp; other)</code> — O(1 + D)</summary>

旧領域を破棄し所有権を移し、*this を返します。

{% raw %}
```cpp
Type moved; moved = std::move(object);
```
{% endraw %}

注意点: D は旧宛先保持量。<utility> が必要です。自己ムーブを含めムーブ元は破棄または再代入してください。

</details>

## 出典・検証

- [Library Checker 仕様・議論](https://github.com/yosupo06/library-checker-problems/issues/737)、[m1une の semi-local LIS 実装](https://m1une.github.io/m1une_library/ds/range_query/range_lis_query.hpp) と公式解法を調査し、再帰的な置換図の積を独立実装。
- `verify/data-structure/static-range-lis.test.cpp`: [Static Range LIS Query](https://judge.yosupo.jp/problem/static_range_lis_query)。
- `tests/random/static-range-lis.cpp`: 全区間を patience sorting と比較。空列、同値、昇順、降順、重複乱数。
