---
title: Static Range LIS
documentation_of: //blueberry/data-structure/static-range-lis.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

静的な列の半開区間に対する**狭義**最長増加部分列の長さを返します。Tの `<` はstrict weak orderingが必要で、重複を増加として数えません。更新はできず、空列・空区間の答えは0です。

Nは入力長。構築はO(N log²(N+1))、問合せはO(log(N+1))、メモリはO(N log(N+1))です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`StaticRangeLIS<T>`](#default) | O(1) |
| [`StaticRangeLIS<T>`](#vector) | O(N log²(N+1)) |
| [`object.size`](#size) | O(1) |
| [`object.lis`](#lis) | O(log(N+1)) |
| [`Type`](#copy-constructor) | O(M) |
| [`Type`](#move-constructor) | O(1) |
| [`operator=`](#copy-assignment) | O(M + D) |
| [`operator=`](#move-assignment) | O(1 + D) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `StaticRangeLIS<T>` を使います。ACLに区間LISはありません。`N <= INT_MAX`、長さと答えはintです。比較をO(1)とし、値の加算はしません。
- semi-local LISのsubunit Monge積で点集合を構築し、既存WaveletMatrixで数えます。同値要素を添字の逆順で順位化します。
- 入力をコピーによって索引化し、元配列の寿命に依存しません。
- Monge積の再帰は構築ごとに一度確保したO(N)の作業領域を使い回します。長さnのframeはn+5⌈n/2⌉個のintを使い、同時に処理しない兄弟は共有します。
- 再帰パスに同時に必要な容量を先に求め、再帰中の確保と部分列vectorの再確保を省きます。作業領域はconstructorローカルで、WaveletMatrix構築前に解放します。
- インスタンス間の共有状態・入力への参照は追加しません。

## 操作の補足 {#操作の補足}

- 以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

</details>

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

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Library Checker 仕様・議論](https://github.com/yosupo06/library-checker-problems/issues/737)、[m1une の semi-local LIS 実装](https://m1une.github.io/m1une_library/ds/range_query/range_lis_query.hpp) と公式解法を調査し、再帰的な置換図の積を独立実装。
- `verify/data-structure/static-range-lis.test.cpp`: [Static Range LIS Query](https://judge.yosupo.jp/problem/static_range_lis_query)。
- `tests/random/static-range-lis.cpp`: 全区間を patience sorting と比較。空列、同値、昇順、降順、重複乱数。

</details>
