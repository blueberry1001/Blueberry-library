---
title: Disjoint Sparse Table
documentation_of: //blueberry/data-structure/disjoint-sparse-table.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

結合的演算の静的区間集約。ACL にない O(1) クエリを提供する。
N は要素数、0 ≤ N ≤ INT_MAX。T はコピー可能、Op は const 呼び出し可能で結合則を満たすこと。
可換性・冪等性・単位元は不要。演算・コピー O(1) を仮定して構築・メモリ O(N log(N+1))。
演算内のオーバーフローは利用者が防ぐ。入力を所有コピーし元配列の変更・破棄の影響を受けない。更新は不可。

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

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `DisjointSparseTable<T, Op>(const std::vector<T>& values, Op op)` | O(N log(N+1)) | [開く](#constructor) |
| `T table.prod(int left, int right) const` | O(1) | [開く](#prod) |
| `int table.size() const` | O(1) | [開く](#size) |

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

## 出典・検証

[Luzhiled](https://ei1333.github.io/library/structure/others/disjoint-sparse-table.hpp.html) を調査し独立実装。
公式 Library Checker `static_range_sum` と固定 seed の非可換文字列連結の愚直比較。候補比較は `docs/development/sequence-batch-two.md`。
