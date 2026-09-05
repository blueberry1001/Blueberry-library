---
title: Fenwick Tree
documentation_of: //blueberry/data-structure/fenwick-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

一点加算・区間和を扱います。標準操作はACLを優先してください。この実装には累積和のlower_boundがあります。Nは配列長、メモリ O(N)。Tの演算を O(1) とした計算量です。T{}が零元となり加算・減算が必要です。

## 最小使用例

```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/fenwick-tree.hpp"
int main() {
  blueberry::FenwickTree<long long> bit(std::vector<long long>{2, 0, 3});
  assert(bit.sum(0, 3) == 5);
  bit.add(1, 4);
  assert(bit.get(1) == 4);
  assert(bit.prefix_sum(2) == 6);
  assert(bit.lower_bound(3) == 2);
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `FenwickTree<T> bit(n) / bit(values)` | O(N) | [開く](#construct) |
| `void bit.add(i, delta)` | O(log N) | [開く](#add) |
| `T bit.prefix_sum(r) const` | O(log N) | [開く](#prefix-sum) |
| `T bit.sum(l, r) const` | O(log N) | [開く](#sum) |
| `T bit.get(i) const` | O(log N) | [開く](#get) |
| `int bit.lower_bound(target) const` | O(log N) | [開く](#lower-bound) |
| `int bit.size() const` | O(1) | [開く](#size) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="construct" markdown="1">
<summary><code>FenwickTree&lt;T&gt; bit(n) / bit(values)</code> — O(N)</summary>

零列またはvectorのコピーから構築します。

```cpp
blueberry::FenwickTree<long long> bit(5);
```

注意点: n>=0。累積和がオーバーフローしない型を選んでください。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void bit.add(i, delta)</code> — O(log N)</summary>

a[i] += deltaを行います。

```cpp
bit.add(2, 5LL);
```

注意点: 代入ではありません。0<=i<N。負の更新は可能ですがlower_boundの前提に注意してください。

</details>

<details class="api-operation" id="prefix-sum" markdown="1">
<summary><code>T bit.prefix_sum(r) const</code> — O(log N)</summary>

半開区間 [0,r) の和を返します。

```cpp
auto sum = bit.prefix_sum(3);
```

注意点: 0<=r<=N。r=0なら零元です。

</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T bit.sum(l, r) const</code> — O(log N)</summary>

半開区間 [l,r) の和を返します。

```cpp
auto sum = bit.sum(1, 3);
```

注意点: 0<=l<=r<=N。空区間は零元です。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T bit.get(i) const</code> — O(log N)</summary>

a[i]を値として返します。

```cpp
auto value = bit.get(2);
```

注意点: 0<=i<N。単なる配列アクセスのように O(1) ではありません。

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>int bit.lower_bound(target) const</code> — O(log N)</summary>

prefix_sum(r)>=targetを満たす最小のrを返します。target<=0は0、未到達はNです。

```cpp
int r = bit.lower_bound(3LL);
bool found = bit.prefix_sum(bit.size()) >= 3LL;
```

注意点: 各要素は非負でなければなりません。返り値は要素番号ではなく区間右端です。Nで到達した場合と未到達の場合を区別するには全体和も確認します。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int bit.size() const</code> — O(1)</summary>

配列長を返します。

```cpp
int n = bit.size();
```

注意点: 構築後に長さは変更できません。

</details>
