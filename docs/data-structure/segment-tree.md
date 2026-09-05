---
title: Segment Tree
documentation_of: //blueberry/data-structure/segment-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

モノイドの一点更新・区間積です。通常はACLのsegtreeを推奨します。この互換実装にはmax_right/min_leftはありません。Nは配列長、メモリ O(N)。演算と値のコピーを O(1) とします。

## 最小使用例

```cpp
#include <cassert>
#include <functional>
#include <vector>
#include "blueberry/data-structure/segment-tree.hpp"
int main() {
  blueberry::SegmentTree<long long, std::plus<long long>> seg(
      std::vector<long long>{1, 2, 3}, std::plus<long long>{}, 0LL);
  assert(seg.product(0, 3) == 6);
  seg.set(1, 10);
  assert(seg.product(1, 3) == 13);
  assert(seg.product(1, 1) == 0);
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SegmentTree<S, Op> seg(n, op, e) / seg(values, op, e)` | O(N) | [開く](#construct) |
| `void seg.set(i, x)` | O(log N) | [開く](#set) |
| `const S& seg.get(i) const` | O(1) | [開く](#get) |
| `S seg.product(l, r) const` | O(log N) | [開く](#product) |
| `const S& seg.all_product() const` | O(1) | [開く](#all-product) |
| `int seg.size() const` | O(1) | [開く](#size) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>SegmentTree&lt;S, Op&gt; seg(n, op, e) / seg(values, op, e)</code> — O(N)</summary>

長さnの単位元の列、またはvectorから構築します。

```cpp
auto op = [](long long a, long long b) { return a + b; };
blueberry::SegmentTree<long long, decltype(op)> seg(5, op, 0LL);
```

注意点: n>=0。opは結合的、eは左右の単位元が必要です。可換性は不要です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void seg.set(i, x)</code> — O(log N)</summary>

a[i]=xと代入します。

```cpp
seg.set(2, 9LL);
```

注意点: 0<=i<N。加算する場合はgetで取得してからsetします。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>const S&amp; seg.get(i) const</code> — O(1)</summary>

要素iのconst参照を返します。

```cpp
auto value = seg.get(2);
```

注意点: 0<=i<N。更新前の値を保存したい場合は参照ではなくコピーしてください。

</details>

<details class="api-operation" id="product" markdown="1">
<summary><code>S seg.product(l, r) const</code> — O(log N)</summary>

[l,r) を添字の昇順で畳み込みます。空区間は単位元です。

```cpp
auto value = seg.product(1, 3);
```

注意点: 0<=l<=r<=N。非可換演算ではopの順番に注意してください。ACLの対応名はprodです。

</details>

<details class="api-operation" id="all-product" markdown="1">
<summary><code>const S&amp; seg.all_product() const</code> — O(1)</summary>

全体の区間積をconst参照で返します。N=0なら単位元です。

```cpp
auto total = seg.all_product();
```

注意点: ACLの対応名はall_prod。更新前の結果を保存する場合はコピーしてください。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int seg.size() const</code> — O(1)</summary>

元の配列長を返します。

```cpp
int n = seg.size();
```

注意点: 内部で2冪に切り上げた長さではありません。

</details>
