---
title: Dynamic Sqrt Tree
documentation_of: //blueberry/data-structure/dynamic-sqrt-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::DynamicSqrtTree<S, op, e>` は配列の点代入と区間モノイド積を扱う。
`prod` は最悪 O(1)、`set` は最悪 O(√N)。構築・メモリは O(N L(N))、
L(N) = 1 + log log(N + 2)、N は要素数で `0 <= N <= INT_MAX`。
空配列でも定数の管理領域を持つため、メモリ・構築の厳密な表記は O(1 + N L(N))。
以下の表ではこの定数項を省略する。

ACL segtree の更新・クエリ O(log N) と異なり、**更新が少なく区間積が非常に多い**場合の
選択肢である。更新がない場合は [静的 Sqrt Tree]({{ '/blueberry/data-structure/sqrt-tree.hpp.html' | relative_url }})
のほうがクエリの演算回数は少ない。通常の点更新用途は ACL segtree を第一候補とし、
[同一条件の比較]({{ '/docs/development/dynamic-sqrt-tree-comparison.html' | relative_url }})で用途を判断する。
固定長配列であり、要素の挿入・削除、範囲更新、`max_right` / `min_left` は提供しない。

`S op(S, S)` は結合的な二項演算、`S e()` は左右単位元を返す関数。
可換性・冪等性・逆元は不要で、左から右への演算順序を保つ。
`S` はコピー構築・コピー代入できる型で、既定コンストラクタは不要。
計算量は `op` / `e` と要素のコピー・破棄が O(1) の場合。
和・積などのオーバーフロー、演算の閉性、演算中に例外を投げないことは呼び出し側の責任。

入力はコピーして保持するので構築後に変更・破棄してよい。外部参照や公開フィールドはない。
返り値もコピーなので、更新・コピー・ムーブによる返り値の参照無効化はない。
コピー先は独立した木。ムーブ元は空になり、`size()` / `prod(0, 0)` / `all_prod()` や
再代入が可能。自己コピー代入・自己ムーブ代入は内容を保つ。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/data-structure/dynamic-sqrt-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  using Tree = blueberry::DynamicSqrtTree<long long, op, e>;
  Tree tree(std::vector<long long>{3, 1, 4, 1, 5});
  assert(tree.size() == 5);
  tree.set(2, 9);
  assert(tree.get(2) == 9);
  assert(tree.prod(1, 4) == 11);
  assert(tree.prod(5, 5) == 0);
  assert(tree.all_prod() == 19);
  Tree copy = tree;
  copy.set(0, 0);
  assert(tree.get(0) == 3);
  Tree moved = std::move(copy);
  assert(copy.size() == 0 && copy.all_prod() == 0);
  assert(moved.all_prod() == 16);
}
```
{% endraw %}

## 操作一覧

以下 `Tree = blueberry::DynamicSqrtTree<S, op, e>`。
代入では N は代入元、M は代入前の代入先の要素数。すべて最悪計算量で、償却表記ではない。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Tree tree` | O(1) | [開く](#default-constructor) |
| `Tree tree(const std::vector<S>& values)` | O(N L(N)) | [開く](#constructor) |
| `Tree copy(const Tree& other)` | O(N L(N)) | [開く](#copy-constructor) |
| `Tree& tree.operator=(const Tree& other)` | O(N L(N) + M L(M)) | [開く](#copy-assignment) |
| `Tree moved(Tree&& other)` | O(1) | [開く](#move-constructor) |
| `Tree& tree.operator=(Tree&& other)` | O(M L(M)) | [開く](#move-assignment) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `S tree.get(int p) const` | O(1) | [開く](#get) |
| `void tree.set(int p, const S& value)` | O(√N) | [開く](#set) |
| `S tree.prod(int l, int r) const` | O(1) | [開く](#prod) |
| `S tree.all_prod() const` | O(1) | [開く](#all-prod) |

<details class="api-operation" id="default-constructor" markdown="1">
<summary><code>Tree tree</code> — O(1)</summary>

空の木を作る。`size() == 0`、空区間積と全体積は `e()`。

{% raw %}
```cpp
Tree empty;
assert(empty.size() == 0);
```
{% endraw %}

注意点: `get` / `set` に有効な添字はない。要素を持たせるには別の木を代入する。
</details>

<details class="api-operation" id="constructor" markdown="1">
<summary><code>Tree tree(const std::vector&lt;S&gt;&amp; values)</code> — O(N L(N))</summary>

入力配列をコピーして構築する。空配列にも対応する。

{% raw %}
```cpp
Tree tree(std::vector<S>{e(), e(), e()});
assert(tree.size() == 3);
```
{% endraw %}

注意点: `values.size() <= INT_MAX`。内部では長さを2冪に切り上げて単位元で埋めるが、
`size()` や有効な添字には含めない。前計算中の演算にもオーバーフローがないこと。
</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>Tree copy(const Tree&amp; other)</code> — O(N L(N))</summary>

全データを複製する。コピー先への更新は元の木に影響しない。

{% raw %}
```cpp
Tree copy(tree);
assert(copy.size() == tree.size());
```
{% endraw %}

注意点: 空の木のコピーも有効。補助 index も独立して保持するのでコピー時間・メモリは
単なる元配列の O(N) より大きい。
</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>Tree&amp; tree.operator=(const Tree&amp; other)</code> — O(N L(N) + M L(M))</summary>

代入元の内容で置換し、代入先自身への参照を返す。元のデータの破棄も含む上界。

{% raw %}
```cpp
Tree copy;
copy = tree;
assert(copy.size() == tree.size());
```
{% endraw %}

注意点: 空の木も代入できる。自己代入は内容を保つ。返された参照の寿命は代入先の木の寿命と同じ。
</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>Tree moved(Tree&amp;&amp; other)</code> — O(1)</summary>

内部配列の所有権を引き継ぎ、元の木を空にする。`noexcept`。

{% raw %}
```cpp
Tree copy = tree;
Tree moved(std::move(copy));
assert(copy.size() == 0);
```
{% endraw %}

注意点: `<utility>` が必要。空になった元の木への `set` / `get` はできないが、
空区間積・全体積・再代入は有効。
</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>Tree&amp; tree.operator=(Tree&amp;&amp; other)</code> — O(M L(M))</summary>

代入先の古いデータを破棄して所有権を引き継ぎ、自身への参照を返す。`noexcept`。

{% raw %}
```cpp
Tree copy = tree;
Tree moved;
moved = std::move(copy);
assert(copy.size() == 0);
```
{% endraw %}

注意点: 所有権の移動自体は O(1) だが、古い要素の破棄を含めて表記している。
自己ムーブ代入は何もせず元の内容を保ち、O(1)。通常のムーブ代入では元の木は空になる。
返された参照は代入先が生存する間有効。
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

元配列の長さ N を返す。点代入では変化しない。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 空の木・ムーブ元では 0。内部のパディングは含まない。
</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S tree.get(int p) const</code> — O(1)</summary>

添字 p の現在の値をコピーして返す。

{% raw %}
```cpp
// tree.size() > 0
S first = tree.get(0);
```
{% endraw %}

注意点: `0 <= p < N`。返り値の変更は木に影響しない。
</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void tree.set(int p, const S&amp; value)</code> — O(√N)</summary>

添字 p の値を `value` で置き換える。加算ではなく代入。

{% raw %}
```cpp
// tree.size() > 0
tree.set(0, e());
```
{% endraw %}

注意点: `0 <= p < N`。更新後の積にすぐ反映される。更新自体では配列を再確保しない。
数値の一点加算は `tree.set(p, tree.get(p) + delta)` と書ける。この場合は値型の `+` が必要で、加算のオーバーフローにも注意する。
単なる根のブロック間積の全再計算は O(N) になるため、この実装では補助 index を用いる。
</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S tree.prod(int l, int r) const</code> — O(1)</summary>

半開区間 `[l, r)` の積を元の配列順に返す。`op` の呼び出しは最大4回。

{% raw %}
```cpp
S result = tree.prod(0, tree.size());
S identity = tree.prod(tree.size(), tree.size());
```
{% endraw %}

注意点: `0 <= l <= r <= N`。空区間は `e()`。非可換演算の順序を保つ。
空区間に単位元を定義できない演算はこの API の対象外。
</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>S tree.all_prod() const</code> — O(1)</summary>

配列全体の積を返す。`prod(0, size())` と等しい。

{% raw %}
```cpp
S total = tree.all_prod();
```
{% endraw %}

注意点: 空の木では `e()`。呼び出しで状態を変更しない。
</details>

## 実装と計算量

長さを P（N 以上で最小の2冪）に切り上げ、根の子ブロック長を
B = `2^ceil(log2(P)/2)`、子の個数を C = P / B とする。B と C はともに O(√N)。
根には各ブロックの prefix/suffix を置き、ブロック総積 C 個を別の **unindexed Sqrt Tree** に格納する。
この補助 index は自身の根にも通常の between 表を持ち、さらに index を作ることはない。

点更新は根の1ブロックの prefix/suffix を O(B) で修復し、根より下の affected parent の
between 表だけを修復する。親の長さの和は O(B)。補助 index では1点更新に O(C) を使ってよい。
補助 index 全体を O(C log log C) で再構築しないこと、主木の根の between を O(N) で
再構築しないことが O(√N) の要点である。
クエリは XOR の最上位ビットから層を直接選び、必要な場合だけ補助 index に1回問い合わせる。

## 出典・検証

- [CP-Algorithms: Sqrt Tree](https://cp-algorithms.com/data_structures/sqrt-tree.html)
  の indexed/unindexed 構造と更新計算量を参照し、静的版と分離して独立実装した。
- [maspypy の Sqrt Tree](https://maspypy.github.io/library/ds/sqrt_tree.hpp)
  も構造・APIの比較対象にした。同実装は静的で固定した層構成を用いる。本実装は
  入力サイズから層を構成して点更新を追加する。公開コードはコピーしていない。
- Library Checker: [Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite)、
  [Point Add Range Sum](https://judge.yosupo.jp/problem/point_add_range_sum)。
- `tests/random/dynamic-sqrt-tree.cpp`: 固定 seed の非可換 affine 合成、空配列、1/2 要素、
  各2冪の前後、全区間・空区間・小配列の全範囲、点更新、コピー・ムーブと独立性を確認する。
  クエリと更新の `op` 回数も検査し、根の線形再計算への退行を検出する。
