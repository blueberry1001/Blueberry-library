---
title: Sparse Table
documentation_of: //blueberry/data-structure/sparse-table.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

更新のない配列のmin/max/gcdなどに使用します。ACLにはありません。N>=1は要素数。演算とコピーを O(1) としたとき前計算・メモリ O(N log N)。結合則と冪等性 op(x,x)==x が必要です。

## 最小使用例

```cpp
#include <algorithm>
#include <cassert>
#include <vector>
#include "blueberry/data-structure/sparse-table.hpp"
int main() {
  auto minimum = [](int a, int b) { return std::min(a, b); };
  blueberry::SparseTable<int, decltype(minimum)> table(
      std::vector<int>{5, 2, 7, 1}, minimum);
  assert(table.product(0, 3) == 2);
  assert(table.product(2, 4) == 1);
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SparseTable<T, Op> table(values, op)` | O(N log N) | [開く](#construct) |
| `T table.product(l, r) const` | O(1) | [開く](#product) |
| `int table.size() const` | O(1) | [開く](#size) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>SparseTable&lt;T, Op&gt; table(values, op)</code> — O(N log N)</summary>

valuesのコピーから2冪長の区間積を前計算します。

```cpp
auto minimum = [](int a, int b) { return std::min(a, b); };
blueberry::SparseTable<int, decltype(minimum)> table(std::vector<int>{3, 1, 4}, minimum);
```

注意点: 空配列は不可です。和やxorは冪等でないため使用できません。

</details>

<details class="api-operation" id="product" markdown="1">
<summary><code>T table.product(l, r) const</code> — O(1)</summary>

非空の半開区間 [l,r) の区間積を返します。

```cpp
int minimum = table.product(0, 2);
```

注意点: 0<=l<r<=N。空区間は不可。元配列の変更は反映されません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int table.size() const</code> — O(1)</summary>

元の配列長を返します。

```cpp
int n = table.size();
```

注意点: 更新が必要な場合は再構築または別のデータ構造を選んでください。

</details>
