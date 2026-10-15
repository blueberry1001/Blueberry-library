---
title: Sparse Table
documentation_of: //blueberry/data-structure/sparse-table.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

更新のない配列のmin/max/gcd等の区間集約を扱います。演算Opには結合則と冪等性op(x,x)==xが必要です。

Nは要素数。構築・メモリはO(N log N)、prodはO(1)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`SparseTable<T,`](#construct) | O(N log N) |
| [`table.prod`](#prod) | O(1) |
| [`table.product`](#product) | O(1) |
| [`table.size`](#size) | O(1) |

以下の操作を開くと、返り値・使用例・注意点を確認できます。断片の例は必要なヘッダと
有効な `table` がある前提です。

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 和・xor等の冪等でない演算にはACL segtree等を使ってください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <algorithm>
#include <cassert>
#include <vector>
#include "blueberry/data-structure/sparse-table.hpp"
int main() {
  auto minimum = [](int a, int b) { return std::min(a, b); };
  blueberry::SparseTable<int, decltype(minimum)> table(
      std::vector<int>{5, 2, 7, 1}, minimum);
  assert(table.prod(0, 3) == 2);
  assert(table.prod(2, 4) == 1);
  assert(table.product(0, 2) == 2);  // 旧 API も利用可能
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>SparseTable&lt;T, Op&gt; table(values, op)</code> — O(N log N)</summary>

`values` をコピーし、二冪長区間の値を前計算します。以後、元配列を書き換えても内容は変わりません。

{% raw %}
```cpp
auto minimum = [](int a, int b) { return std::min(a, b); };
blueberry::SparseTable<int, decltype(minimum)> table(
    std::vector<int>{3, 1, 4}, minimum);
```
{% endraw %}

注意点: 空配列は不可です。`Op` の結果を `T` に変換でき、結合則・冪等性が成立する必要があります。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T table.prod(l, r) const</code> — O(1)</summary>

半開区間 $[l,r)$ の演算結果を返します。`prod` が新しい短い呼び出し名です。

{% raw %}
```cpp
int minimum = table.prod(0, 2);
```
{% endraw %}

注意点: `0 <= l < r <= N` が必要で、空区間は指定できません。冪等性がない演算では重複区間の
合成が正しい結果にならないため利用しないでください。

</details>

<details class="api-operation" id="product" markdown="1">
<summary><code>T table.product(l, r) const</code> — O(1)</summary>

既存コードとの互換性のために残している別名です。実装は `prod` に委譲します。

{% raw %}
```cpp
int minimum = table.product(0, 2);
```
{% endraw %}

注意点: 新規コードでは短い `prod` を推奨します。引数・区間の制約は `prod` と同じです。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int table.size() const</code> — O(1)</summary>

構築時に渡した配列の長さ $N$ を返します。

{% raw %}
```cpp
int n = table.size();
```
{% endraw %}

注意点: 構築後に要素数は変わりません。更新が必要なら再構築または別のデータ構造を選んでください。

</details>
