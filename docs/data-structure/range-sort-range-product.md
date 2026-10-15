---
title: Range Sort Range Product
documentation_of: //blueberry/data-structure/range-sort-range-product.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

キーによる区間の昇順・降順ソート、一点置換、任意区間のモノイド積を扱います。`RangeSortRangeProduct<S, op, e, Key>` のopは結合的、e()は左右単位元で、非可換演算も使えます。Keyは符号なし整数で、**現在の全キーは互いに異なる**必要があります。

Nは固定要素数、WはKeyのビット数、Qは操作数。構築はO(N log(N+1))、get/set/prodはO(log(N+1)+W)、全操作合計はO((N+Q)(log(N+1)+W))です。sortは初期構築分を含む償却評価で、メモリはO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`RangeSortRangeProduct<S,op,e,Key>`](#default) | O(1) |
| [`RangeSortRangeProduct<S,op,e,Key>`](#vector) | O(N log(N+1)) |
| [`object.size`](#size) | O(1) |
| [`object.get`](#get) | O(log(N+1)+W) |
| [`object.set`](#set) | O(log(N+1)+W) |
| [`object.prod`](#prod) | O(log(N+1)+W) |
| [`object.all_prod`](#all-prod) | O(1) |
| [`object.sort`](#sort) | 償却 O(log(N+1)+W) |
| [`Type`](#copy-constructor) | O(M) |
| [`Type`](#move-constructor) | O(1) |
| [`operator=`](#copy-assignment) | O(M + D) |
| [`operator=`](#move-assignment) | O(1 + D) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 既定のKeyはunsignedで、boolは使えません。setで他要素のキーを使うことは禁止です。`0 <= N <= INT_MAX/2` で、空列も使えます。
- ACL segtreeで各整列ブロックの積を管理し、内部はbranching bitだけを持つPatricia trieです。昇順・降順の両方の積を保持し、ノードを再利用します。
- Sはコピー・代入可能で、計算量は演算・コピーをO(1)とみなします。文字列等では保持量と演算コストを加えてください。Sの演算のoverflowは呼出側が管理します。
- sortの単一呼出しは最大O(N(log(N+1)+W))です。
- prodも内部ブロックを分割するため非constですが、論理的な列は不変です。返り値はコピーで内部参照を公開せず、入力や返り値の寿命に依存しません。

## 操作の補足 {#操作の補足}

- 以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/data-structure/range-sort-range-product.hpp"
std::string op(std::string a,std::string b){return a+b;}
std::string e(){return {};}
int main(){
  blueberry::RangeSortRangeProduct<std::string,op,e> ds({{3,"c"},{1,"a"},{2,"b"}});
  ds.sort(0,3);
  assert(ds.prod(0,3)=="abc");
  ds.sort(0,3,true);
  assert(ds.all_prod()=="cba");
  ds.set(1,4,"d");
  assert(ds.get(1).first==4 && ds.prod(1,2)=="d");
}
```
{% endraw %}

<details class="api-operation" id="default" markdown="1">
<summary><code>RangeSortRangeProduct&lt;S,op,e,Key&gt;()</code> — O(1)</summary>

空列を構築します。

{% raw %}
```cpp
Type object;
```
{% endraw %}

注意点: prod(0,0) と all_prod() は e()。

</details>

<details class="api-operation" id="vector" markdown="1">
<summary><code>RangeSortRangeProduct&lt;S,op,e,Key&gt;(const vector&lt;pair&lt;Key,S&gt;&gt;&amp; values)</code> — O(N log(N+1))</summary>

キー・値の列をコピーします。

{% raw %}
```cpp
Type object(std::vector<std::pair<unsigned,S>>{});
```
{% endraw %}

注意点: 初期キーは全て異なること。列順は入力順です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

列長を返します。

{% raw %}
```cpp
int n=object.size();
```
{% endraw %}

注意点: 操作で列長は変わりません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>pair&lt;Key,S&gt; object.get(int p) const</code> — O(log(N+1)+W)</summary>

現在位置 p のキーと値を返します。

{% raw %}
```cpp
auto [key,value]=object.get(0);
```
{% endraw %}

注意点: 0<=p<N。ソート後の位置です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void object.set(int p, Key key, const S&amp; value)</code> — O(log(N+1)+W)</summary>

現在位置 p のキー・値を置換します。

{% raw %}
```cpp
object.set(0,7,value);
```
{% endraw %}

注意点: 0<=p<N。他の現存キーと重複不可。同じ位置の旧キーを再使用することは可能。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S object.prod(int l, int r)</code> — O(log(N+1)+W)</summary>

現在順序で [l,r) の積を返します。

{% raw %}
```cpp
auto value=object.prod(0,object.size());
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間は e()。内部分割のみ変化し、要素順は不変。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>S object.all_prod() const</code> — O(1)</summary>

列全体の積を返します。

{% raw %}
```cpp
auto value=object.all_prod();
```
{% endraw %}

注意点: 空列は e()。

</details>

<details class="api-operation" id="sort" markdown="1">
<summary><code>void object.sort(int l, int r, bool descending=false)</code> — 償却 O(log(N+1)+W)</summary>

区間をキー昇順、true なら降順にします。

{% raw %}
```cpp
object.sort(0,object.size(),true);
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間は何もしません。償却は初期 O(N(log(N+1)+W)) の費用を含む列全体の評価。単発は線形個のブロックを結合し得ます。

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

- [nachia の解説](https://www.mathenachia.blog/range-sort-range-product/) と [公式問題議論](https://github.com/yosupo06/library-checker-problems/issues/818) を比較し、圧縮 trie と ACL segtree の構成で独立実装。
- `verify/data-structure/range-sort-range-product.test.cpp`: [Point Set Range Sort Range Composite](https://judge.yosupo.jp/problem/point_set_range_sort_range_composite)。
- `tests/random/range-sort-range-product.cpp`: string 連結による非可換積を通常の配列ソート・一点置換と比較。

</details>
