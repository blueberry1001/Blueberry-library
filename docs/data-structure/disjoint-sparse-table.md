---
title: Disjoint Sparse Table
documentation_of: //blueberry/data-structure/disjoint-sparse-table.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

更新のない配列の区間集約を最悪O(1)で返します。コンストラクタに渡す `op` は結合的で、可換性・冪等性・単位元は不要です。区間は `[l,r)` で、空区間は扱いません。

Nは要素数。構築・メモリはO(N log(N+1))です。ACLにない静的O(1)クエリを提供します。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`DisjointSparseTable<T,`](#constructor) | O(N log(N+1)) |
| [`table.prod`](#prod) | O(1) |
| [`table.size`](#size) | O(1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `0 <= N <= INT_MAX`。Tはコピー可能、Opはconst呼び出し可能であることが必要です。計算量は演算・コピーをO(1)とみなします。
- **集約演算はコンストラクタに渡したopで決まります。** 区間和は `x + y`、区間最小値は `std::min(x, y)` を指定します。Library Checkerには問題に合うverifyコードを使ってください。区間和版をStatic RMQに提出すると、合計を出力するためWAになります。

| 問題 | 演算 | 提出用コード |
| --- | --- | --- |
| Static Range Sum（区間和） | `[](long long x, long long y) { return x + y; }` | [区間和版]({{ '/verify/data-structure/disjoint-sparse-table.test.cpp.html' | relative_url }}) |
| Static RMQ（区間最小値） | `[](int x, int y) { return std::min(x, y); }` | [区間最小値版]({{ '/verify/data-structure/disjoint-sparse-table-rmq.test.cpp.html' | relative_url }}) |

- 演算内のオーバーフローは利用者が防いでください。
- 入力を所有コピーし、元配列の変更・破棄の影響を受けません。所有コピーを第0層として共用し、同一内容を別に保持するN要素分の領域を省いています。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
int main() {
  blueberry::DisjointSparseTable table(std::vector<int>{2, 3, 4}, [](int x, int y) { return x + y; });
  assert(table.size() == 3);
  assert(table.prod(0, 3) == 9);
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>DisjointSparseTable&lt;T, Op&gt;(const std::vector&lt;T&gt;&amp; values, Op op)</code> — O(N log(N+1))</summary>

values と op を保持する。空配列も構築可能。T のデフォルト構築は不要。

{% raw %}
```cpp
blueberry::DisjointSparseTable table(std::vector<int>{2, 3}, [](int x, int y) { return x + y; });
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T table.prod(int left, int right) const</code> — O(1)</summary>

0 ≤ left < right ≤ N。半開区間を左から右へ集約した独立した値を返す。空区間は禁止。計算量は最悪であり償却ではない。

{% raw %}
```cpp
assert(table.prod(0, 2) == 5);
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int table.size() const</code> — O(1)</summary>

要素数を返す。空構築では0。状態を変更しない。

{% raw %}
```cpp
assert(table.size() == 2);
```
{% endraw %}

注意点: 上記の入力範囲・空入力の規約を守ること。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Luzhiled](https://ei1333.github.io/library/structure/others/disjoint-sparse-table.hpp.html) を調査し独立実装。
- 公式 Library Checker `static_range_sum` と固定 seed の非可換文字列連結の愚直比較。
- 候補比較は `docs/development/sequence-batch-two.md`。

</details>
