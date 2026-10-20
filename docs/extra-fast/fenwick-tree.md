---
title: Padded Fenwick Tree
documentation_of: //blueberry/extra-fast/fenwick-tree.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::FenwickTree`はuint64_tの一点加算・区間和とprefix探索を行います。
内部添字に約0.1%のpaddingを入れ、2の冪間隔のcache衝突を減らします。
構築はO(N+1)、更新・総和・探索はO(log(N+1))、メモリはO(N+1)です。
16分木のSumSegmentTreeより省メモリで、prefix探索も使えます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`FenwickTree(n = 0)`](#construct) | O(N+1) |
| [`FenwickTree(values)`](#construct-values) | O(N+1) |
| [`tree.size() → int`](#size) | O(1) |
| [`tree.add(p, x)`](#add) | O(log(N+1)) |
| [`tree.set(p, x)`](#set) | O(log(N+1)) |
| [`tree.get(p) → uint64_t`](#get) | O(log(N+1)) |
| [`tree.pref(r) → uint64_t`](#pref) | O(log(N+1)) |
| [`tree.prod(l, r) → uint64_t`](#prod) | O(log(N+1)) |
| [`tree.all_prod() → uint64_t`](#all-prod) | O(log(N+1)) |
| [`tree.lower_bound(x) → int`](#lower-bound) | O(log(N+1)) |
| [`FenwickTree(other)`](#copy-construct) | O(N+1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`tree = other`](#copy-assign) | O(N+V+1) |
| [`FenwickTree(std::move(other))`](#move-construct) | O(1) |
| [`tree = std::move(other)`](#move-assign) | O(V+1) |
| [`tree.~FenwickTree()`](#destroy) | O(N+1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 値は`std::uint64_t`で、加減算・乗算はすべてmod 2^64です。符号付きオーバーフローは使いません。
- 添字は0-indexed、区間は半開区間です。不正な添字はassertの対象で、NDEBUG時は検査しません。
- 各オブジェクトが値を所有し、コピーは独立しています。公開フィールドや要素への参照はありません。
- 移動元には破棄と再代入だけを行ってください。構築・コピーの確保失敗では標準例外を送出します。更新時の追加allocationはありません。
- Nは要素数、Vは代入前の代入先の要素数です。計算量はすべて最悪計算量です。縮小代入後の未使用capacityをC語とすると保持領域はO(N+C+1)になる場合があります。
- 通常版やACLを変更せず、個別includeする実験版です。`blueberry/all.hpp`には含まれません。
- `0 <= N < INT_MAX`で領域を確保できることが必要です。配列は`N + floor(N/1024) + 1`語です。
- prefix探索では現在の要素の合計がUINT64_MAX以下で、全prefixが単調非減少であることが必要です。
- 手書きasmやISA必須化は行いません。BITの依存したメモリアクセスをSIMD化するより、配置とO(N)構築を優先します。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/fenwick-tree.hpp"
int main() {
  blueberry::extra_fast::FenwickTree tree(std::vector<std::uint64_t>{1, 2, 3});
  assert(tree.lower_bound(3) == 2);
  tree.add(1, 4);
  tree.set(2, 5);
  assert(tree.prod(1, 3) == 11);
  assert(tree.get(1) == 6);
  assert(tree.pref(0) == 0);
  assert(tree.all_prod() == 12);
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>FenwickTree(n = 0)</code> — O(N+1)</summary>

長さnの0列を構築します。`0 <= n < INT_MAX`です。

{% raw %}
```cpp
blueberry::extra_fast::FenwickTree tree(8);
```
{% endraw %}

注意点: n=0も有効です。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>FenwickTree(values)</code> — O(N+1)</summary>

`const std::vector<std::uint64_t>& values`をコピーして構築します。

{% raw %}
```cpp
std::vector<std::uint64_t> a = {1, 2, 3};
blueberry::extra_fast::FenwickTree tree(a);
```
{% endraw %}

注意点: 空vectorも有効です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>tree.size() → int</code> — O(1)</summary>

要素数Nを返します。

{% raw %}
```cpp
assert(tree.size() == 3);
```
{% endraw %}

注意点: 空列でも長さ0を返します。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>tree.add(p, x)</code> — O(log(N+1))</summary>

`0 <= p < N`の要素にuint64_tのxを加えます。

{% raw %}
```cpp
tree.add(1, 5);
```
{% endraw %}

注意点: 空列には呼べません。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>tree.set(p, x)</code> — O(log(N+1))</summary>

`0 <= p < N`の要素をuint64_tのxに置き換えます。

{% raw %}
```cpp
tree.set(1, 10);
```
{% endraw %}

注意点: 空列には呼べません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>tree.get(p) → uint64_t</code> — O(log(N+1))</summary>

`0 <= p < N`の現在の値を返します。

{% raw %}
```cpp
auto x = tree.get(1);
```
{% endraw %}

注意点: 内部のFenwick部分木の差から復元します。

</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>tree.pref(r) → uint64_t</code> — O(log(N+1))</summary>

`0 <= r <= N`について[0,r)の総和を返します。

{% raw %}
```cpp
auto x = tree.pref(2);
```
{% endraw %}

注意点: r=0では0です。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>tree.prod(l, r) → uint64_t</code> — O(log(N+1))</summary>

`0 <= l <= r <= N`について[l,r)の総和を返します。

{% raw %}
```cpp
auto x = tree.prod(0, 2);
```
{% endraw %}

注意点: 空区間では0です。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>tree.all_prod() → uint64_t</code> — O(log(N+1))</summary>

全体の総和を返します。

{% raw %}
```cpp
auto x = tree.all_prod();
```
{% endraw %}

注意点: 空列では0です。

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>tree.lower_bound(x) → int</code> — O(log(N+1))</summary>

`pref(r) >= x`となる最小のrを[0,N]から返します。存在しなければN+1、x=0なら0です。現在の全要素が非負の通常整数として解釈でき、prefixがuint64_tをオーバーフローしないことが必要です。

{% raw %}
```cpp
// treeは {1,2,3} とする。
assert(tree.lower_bound(3) == 2);
assert(tree.lower_bound(7) == 4);
```
{% endraw %}

注意点: mod 2^64で折り返す列には使えません。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>FenwickTree(other)</code> — O(N+1)</summary>

独立したコピーを作ります。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: コピー後の更新は相手に影響しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>tree = other</code> — O(N+V+1)</summary>

独立したコピーを代入します。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入も有効です。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>FenwickTree(std::move(other))</code> — O(1)</summary>

確保領域を引き継ぎます。

{% raw %}
```cpp
auto moved = std::move(copy);
```
{% endraw %}

注意点: 移動元には破棄か再代入だけを行います。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>tree = std::move(other)</code> — O(V+1)</summary>

旧領域を解放し、確保領域を引き継ぎます。

{% raw %}
```cpp
copy = std::move(moved);
```
{% endraw %}

注意点: 型・添字・数値の条件は共通前提に従います。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~FenwickTree()</code> — O(N+1)</summary>

所有領域を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::FenwickTree temporary(4); }
```
{% endraw %}

注意点: 通常はスコープ末尾で自動的に呼び出されます。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [LC Point Add Range Sum](https://judge.yosupo.jp/problem/point_add_range_sum)と固定seedの境界・ランダム愚直比較。
- [Algorithmica: Segment Trees](https://en.algorithmica.org/hpc/data-structures/segment-trees/)のcache配置の議論を参考に、独立に実装しました。
- [上位提出調査と比較測定]({{ '/docs/development/measurements/extra-fast-more/lazy-bit/README.html' | relative_url }})。

</details>
