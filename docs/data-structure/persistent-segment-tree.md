---
title: Persistent Segment Tree
documentation_of: //blueberry/data-structure/persistent-segment-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

過去の配列を保ち、任意の版から一点更新で分岐できるSegment Treeです。`PersistentSegmentTree<T, Op>` のopは結合的、identityは両側単位元で、非可換演算も使えます。区間は0始まりの `[l,r)`。applyは一点にop(old,value)を適用します。

Nは配列長、Uは更新回数、V=U+1は版数。全点単位元の長さ構築はO(1)、配列構築はO(N)。更新は償却O(log(N+1))、区間積はO(log(N+1))、メモリはO(N+U log(N+1)+V)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`PersistentSegmentTree`](#construct-size) | O(1) |
| [`PersistentSegmentTree`](#construct-array) | O(N) |
| [`size`](#size) | O(1) |
| [`versions`](#versions) | O(1) |
| [`set`](#set) | 償却 O(log(N+1)) |
| [`apply`](#apply) | 償却 O(log(N+1)) |
| [`T`](#get) | O(log(N+1)) |
| [`T`](#prod) | O(log(N+1)) |
| [`T`](#all-prod) | O(1) |
| [`PersistentSegmentTree`](#copy-move) | コピー O(N+U log(N+1)+V)、ムーブ O(1) ※代入先解放を除く |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLの通常segtreeにない履歴を提供します。遅延区間更新・版間の区間コピーは提供しません。
- N=0でも版0が存在し、空区間積は単位元です。一点更新の深さはO(log(N+1))。計算量はTのコピーとopをO(1)とし、文字列等には実際の操作コストを加えてください。数値オーバーフローは利用側で防いでください。
- ノードを整数indexで参照するappend-onlyの連続領域に保持し、ノード数・版数はINT_MAX以下が必要です。vector再確保時の単発更新は格納ノード数に比例し得ます。
- 版0は初期版で、更新は末尾に新しい版番号を返します。番号はこのオブジェクトに属し、他オブジェクトから流用できません。過去版は破棄されず、内部再確保でも番号は変わりません。
- 返り値はコピーで、参照・公開フィールドはありません。コピーは全ノード・版を複製した独立した所有者となり、同じ番号が同じ履歴を指します。ムーブ元には破棄・再代入のみ行ってください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <functional>
#include <vector>
#include "blueberry/data-structure/persistent-segment-tree.hpp"
int main() {
  blueberry::PersistentSegmentTree tree(std::vector<long long>{2,3,5},
                                       std::plus<long long>{}, 0LL);
  int a = tree.set(0, 1, 10);
  int b = tree.apply(0, 2, 4);
  assert(tree.prod(0, 0, 3) == 10);
  assert(tree.all_prod(a) == 17 && tree.all_prod(b) == 14);
  assert(tree.get(a, 1) == 10 && tree.versions() == 3);
}
```
{% endraw %}

<details class="api-operation" id="construct-size" markdown="1">
<summary><code>PersistentSegmentTree(int n, Op op, T identity)</code> — O(1)</summary>

N 点すべて単位元の版0を作る。

{% raw %}
```cpp
blueberry::PersistentSegmentTree empty(0, std::plus<int>{}, 0);
assert(empty.prod(0, 0, 0) == 0);
```
{% endraw %}

注意点: N>=0。未確保部分木は単位元を表す。

</details>

<details class="api-operation" id="construct-array" markdown="1">
<summary><code>PersistentSegmentTree(const std::vector&lt;T&gt;&amp; a, Op op, T identity)</code> — O(N)</summary>

a をコピーして版0を構築する。

{% raw %}
```cpp
blueberry::PersistentSegmentTree t(std::vector<int>{1,2}, std::plus<int>{}, 0);
```
{% endraw %}

注意点: 入力 vector は後で変更・破棄してよい。空配列も有効。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

すべての版に共通する配列長。

{% raw %}
```cpp
assert(tree.size() == 3);
```
{% endraw %}

注意点: 更新では長さは変化しない。

</details>

<details class="api-operation" id="versions" markdown="1">
<summary><code>int versions() const</code> — O(1)</summary>

現在存在する版の数を返す。

{% raw %}
```cpp
assert(tree.versions() == 3);
```
{% endraw %}

注意点: 初期状態は空配列でも 1。版番号は [0,versions())。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>int set(int version, int p, const T&amp; value)</code> — 償却 O(log(N+1))</summary>

指定版の p を value に置換した新しい版番号を返す。

{% raw %}
```cpp
int c = tree.set(0, 0, 20);
assert(tree.get(0, 0) == 2 && tree.get(c, 0) == 20);
```
{% endraw %}

注意点: 有効な版と 0<=p<N が必要。古い版は変化せず、値が同じでも新しい版を作る。

</details>

<details class="api-operation" id="apply" markdown="1">
<summary><code>int apply(int version, int p, const T&amp; value)</code> — 償却 O(log(N+1))</summary>

指定版の p を op(old,value) にした新しい版番号を返す。

{% raw %}
```cpp
int c = tree.apply(0, 0, 7);
assert(tree.get(c, 0) == 9);
```
{% endraw %}

注意点: 一点のモノイド演算であり、区間作用ではない。引数順序を逆にしない。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T get(int version, int p) const</code> — O(log(N+1))</summary>

指定版の一点をコピーして返す。

{% raw %}
```cpp
assert(tree.get(0, 2) == 5);
```
{% endraw %}

注意点: 有効な版と 0<=p<N が必要。空配列で呼べない。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T prod(int version, int l, int r) const</code> — O(log(N+1))</summary>

指定版の [l,r) を左から右へ op で畳み込む。

{% raw %}
```cpp
assert(tree.prod(0, 1, 3) == 8);
assert(tree.prod(0, 2, 2) == 0);
```
{% endraw %}

注意点: 有効な版と 0<=l<=r<=N。空範囲は identity。可換性は不要。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>T all_prod(int version) const</code> — O(1)</summary>

指定版の全要素の積を返す。

{% raw %}
```cpp
assert(tree.all_prod(0) == 10);
```
{% endraw %}

注意点: 空配列なら identity。有効な版番号が必要。

</details>

<details class="api-operation" id="copy-move" markdown="1">
<summary><code>PersistentSegmentTree(other); operator=(other)</code> — コピー O(N+U log(N+1)+V)、ムーブ O(1) ※代入先解放を除く</summary>

標準のコピーとムーブを持つ。コピー先の更新は元を変えない。

{% raw %}
```cpp
auto copy = tree;
int v = copy.set(0, 0, 99);
assert(copy.get(v, 0) == 99 && tree.get(0, 0) == 2);
```
{% endraw %}

注意点: Op/T の特殊メンバ操作が可能であること。ムーブ後は破棄または再代入する。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [公式 Rectangle Sum](https://judge.yosupo.jp/problem/rectangle_sum) は x 接頭辞ごとの版を作り、過去の二つの版の差から矩形和を求める。
- [Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite) で非可換の積順序も検証する。
- 固定 seed の配列コピーを正解とする任意の過去版からの分岐テストを含む。
- 比較資料: [ei1333 のポインタ版](https://ei1333.github.io/library/structure/segment-tree/persistent-segment-tree.hpp.html)、[Nyaan の固定プール版](https://nyaannyaan.github.io/library/segment-tree/persistent-segment-tree.hpp.html)。
- 本実装は可変の index pool を使い、固定巨大領域を要求しない。
- Rectangle Sum Fastest [393934](https://judge.yosupo.jp/submission/393934) の main はオフライン sweep + Fenwick を使う。
- 静的矩形和だけなら効率的だが、任意の版から分岐する API とは用途が違う。
- 永続 range affine の Fastest は範囲コピーと遅延作用を含むため対象外。
- 測定条件と独立したポインタ候補は `benchmark/data-structure-expansion.cpp` と `benchmark/results/lc-expansion-ds/` を参照。

</details>
