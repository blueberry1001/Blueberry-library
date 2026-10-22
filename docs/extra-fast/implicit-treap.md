---
title: Fast Implicit Treap
documentation_of: //blueberry/extra-fast/implicit-treap.hpp
---

[さらに高速なライブラリへ戻る]({{ '/extra-fast.html' | relative_url }})

## 概要

`blueberry::extra_fast::ImplicitTreap<Mod>`は、挿入・削除で長さが変わる整数列の区間和・反転・アフィン変換を扱います。既定の法は998244353で、`Mod=0`ではuint64_tのmod 2^64演算を使います。

整数indexの小さなノード、削除領域の再利用、O(N+1)の一括構築を使います。区間和とアフィン更新は境界の経路だけをたどり、全区間の`prod/apply/reverse`はO(1)です。通常操作は期待O(log(N+1))、保持メモリはO(C+1)です。汎用の演算・値型には[通常版Implicit Treap]({{ '/blueberry/data-structure/implicit-treap.hpp.html' | relative_url }})を使ってください。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`ImplicitTreap(values = {}, seed = …)`](#construct) | O(N+1) |
| [`tree.size() → int`](#size) | O(1) |
| [`tree.empty() → bool`](#empty) | O(1) |
| [`tree.insert(p, x)`](#insert) | 期待・償却O(log(N+1)) |
| [`tree.erase(p)`](#erase) | 期待O(log(N+1)) |
| [`tree.get(p) → value_type`](#get) | 期待O(log(N+1)) |
| [`tree.set(p, x)`](#set) | 期待O(log(N+1)) |
| [`tree.prod(l, r) → value_type`](#prod) | 期待O(log(N+1)) |
| [`tree.all_prod() → value_type`](#all-prod) | O(1) |
| [`tree.reverse(l, r)`](#reverse) | 期待O(log(N+1)) |
| [`tree.apply(l, r, a, b)`](#apply) | 期待O(log(N+1)) |
| [`tree.to_vector() → vector<value_type>`](#to-vector) | O(N+1) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`ImplicitTreap<Mod>::value_type`](#value-type) | O(1)（型別名） |
| [`tree.reserve(capacity)`](#reserve) | O(P+1) |
| [`tree.clear()`](#clear) | O(P+1) |
| [`ImplicitTreap(other)`](#copy-construct) | O(P+1) |
| [`tree = other`](#copy-assign) | O(P+Q+1) |
| [`ImplicitTreap(std::move(other))`](#move-construct) | O(1) |
| [`tree = std::move(other)`](#move-assign) | O(Q+1) |
| [`tree.~ImplicitTreap()`](#destroy) | O(P+1) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- テンプレート引数は`std::uint32_t Mod = 998244353`です。`Mod=0`または`1 <= Mod <= 2^31`を指定します。素数である必要はありません。
- `Mod>0`では値とアフィン変換の係数を法で正規化します。中間積を含め符号付きoverflowを使いません。`Mod=0`の加算・乗算はuint64_tで行い、wraparoundをそのままmod 2^64と解釈します。
- 奇数の法では内部をMontgomery表現にして剰余演算を減らします。偶数の法は通常の定数法による演算です。公開APIの入力・出力はどちらも通常の整数表現です。
- 添字は0-indexed、区間は半開区間`[l,r)`です。`0 <= N < INT_MAX`が必要で、追加後もこの範囲に収めてください。公開操作はこの前提を満たす入力で使います。
- Nは現在の要素数、Pは`clear`以降に確保したpoolのslot数、Cはそのcapacity、Qは代入前の代入先のslot数です。削除したslotは再利用しますが、削除だけではpoolやcapacityを縮めません。
- 保持量はO(C+1)です。通常のGCC/Clangではノードは`Mod>0`で32byte、`Mod=0`で48byteです。構築・全列取得の一時領域はO(N+1)、通常操作の再帰stackは木の高さに比例します。
- 固定seedの擬似乱数priorityを使います。期待計算量はpriorityと独立な操作列についての評価で、最悪の高さ・時間・再帰stackはO(N+1)です。敵対的入力に対する最悪時間保証はありません。
- `insert`でpoolを増設すると単発O(P+N+1)になる場合があります。`reserve`を使わない連続追加は再確保を含めた償却評価です。他の通常操作はpoolを増設しません。
- 各オブジェクトが値を所有し、コピーは独立します。公開フィールド・反復子・要素参照はなく、返り値は所有値です。入力vectorや返したvectorの寿命に依存しません。
- `get`・`prod`・`to_vector`は遅延伝搬のため非constですが、論理的な列は変えません。`size`・`empty`・`all_prod`はconstです。移動元は空になり、通常の空の木として再利用できます。
- allocationを行う構築・コピー・`reserve`・pool増設・`to_vector`では標準例外を送出する場合があります。
- 通常版と`blueberry/all.hpp`は変更せず、個別includeする実験版です。SIMDやasmによる並列化は行わず、ノード配置と不要な分割・結合・遅延伝搬の削減に絞っています。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>
#include "blueberry/extra-fast/implicit-treap.hpp"
int main() {
  using Tree = blueberry::extra_fast::ImplicitTreap<>;
  Tree tree(std::vector<std::uint32_t>{1, 2, 3});
  tree.reserve(16);
  tree.insert(1, 7);
  tree.apply(1, 3, 2, 1);
  tree.reverse(0, 4);
  assert((tree.to_vector() == std::vector<std::uint32_t>{3, 5, 15, 1}));
  assert(tree.prod(1, 3) == 20);
  tree.erase(2);
  tree.set(0, 9);
  assert(tree.size() == 3 && tree.get(0) == 9);
  assert(tree.all_prod() == 15 && tree.prod(2, 2) == 0);
  blueberry::extra_fast::ImplicitTreap<0> wrap(
      std::vector<std::uint64_t>{std::numeric_limits<std::uint64_t>::max(), 2});
  assert(wrap.all_prod() == 1);
  wrap.clear();
  assert(wrap.empty());
}
```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>explicit ImplicitTreap(const std::vector&lt;value_type&gt;&amp; values = {}, std::uint64_t seed = 0x243f6a8885a308d3ULL)</code> — O(N+1)</summary>

valuesをコピーして同じ順序の列を構築します。Cartesian treeを一括構築します。seedはpriority生成に使い、同じseedと操作列を再現できます。

{% raw %}
```cpp
using Tree = blueberry::extra_fast::ImplicitTreap<>;
Tree empty;
Tree tree(std::vector<std::uint32_t>{1, 2, 3}, 12345);
```
{% endraw %}

注意点: 空vectorも有効です。`values.size() < INT_MAX`と領域確保が必要です。`Mod>0`では各値を正規化します。

</details>

<details class="api-operation" id="value-type" markdown="1">
<summary><code>ImplicitTreap&lt;Mod&gt;::value_type</code> — O(1)（型別名）</summary>

`Mod=0`では`std::uint64_t`、それ以外では`std::uint32_t`です。値・区間和・アフィン係数に同じ型を使います。

{% raw %}
```cpp
using Value = blueberry::extra_fast::ImplicitTreap<0>::value_type;
Value x = 42;
```
{% endraw %}

注意点: 符号付き整数からの変換はC++の整数変換規則に従います。負数の剰余を意図する場合は呼出側で正規化してください。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

現在の要素数を返します。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 空なら0です。削除済みslotや予約capacityは含みません。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool tree.empty() const</code> — O(1)</summary>

要素数が0かを返します。

{% raw %}
```cpp
if (tree.empty()) tree.insert(0, 1);
```
{% endraw %}

注意点: `reserve`後や移動元の木にも使えます。

</details>

<details class="api-operation" id="reserve" markdown="1">
<summary><code>void tree.reserve(int capacity)</code> — O(P+1)</summary>

capacity個の要素を同時に保持するためのpool領域を予約します。列の長さ・値・順序は変えません。

{% raw %}
```cpp
tree.reserve(200000);
```
{% endraw %}

注意点: `0 <= capacity < INT_MAX`です。確保済みcapacity以下ならO(1)で、縮小しません。増設では既存のP個のslotを移すことがあります。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void tree.clear() noexcept</code> — O(P+1)</summary>

列を空にし、poolと予約領域を解放します。

{% raw %}
```cpp
tree.clear();
tree.insert(0, 7);
```
{% endraw %}

注意点: 空の木にも使えます。削除済みslotもまとめて解放します。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>void tree.insert(int p, value_type x)</code> — 期待・償却O(log(N+1))</summary>

位置pの直前にxを挿入します。`0 <= p <= size()`で、p=size()なら末尾追加です。

{% raw %}
```cpp
tree.insert(0, 5);
tree.insert(tree.size(), 8);
```
{% endraw %}

注意点: 追加後も`size() < INT_MAX`が必要です。削除slotを優先して再利用します。予約領域が足りる場合は再確保せず、単発の最悪時間はO(N+1)です。再確保を伴う場合の最悪時間はO(P+N+1)です。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>void tree.erase(int p)</code> — 期待O(log(N+1))</summary>

位置pの要素を1個削除します。`0 <= p < size()`です。後ろの添字は1ずつ小さくなります。

{% raw %}
```cpp
tree.erase(tree.size() - 1);
```
{% endraw %}

注意点: 空の木には使えません。削除slotは将来のinsertで再利用し、保持領域は縮小しません。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>value_type tree.get(int p)</code> — 期待O(log(N+1))</summary>

位置pの値を返します。`0 <= p < size()`です。

{% raw %}
```cpp
auto x = tree.get(0);
```
{% endraw %}

注意点: 内部参照ではなく値を返します。反転・アフィン変換を適用した後の論理位置の値です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void tree.set(int p, value_type x)</code> — 期待O(log(N+1))</summary>

位置pの値をxで置き換えます。`0 <= p < size()`です。

{% raw %}
```cpp
tree.set(0, 17);
```
{% endraw %}

注意点: 過去の遅延作用を反映した現在の位置に代入します。`Mod>0`ではxを正規化します。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>value_type tree.prod(int l, int r)</code> — 期待O(log(N+1))、全区間はO(1)</summary>

`[l,r)`の和を返します。`0 <= l <= r <= size()`です。

{% raw %}
```cpp
auto sum = tree.prod(0, tree.size());
auto zero = tree.prod(0, 0);
```
{% endraw %}

注意点: 空区間は0です。`Mod>0`なら法で正規化した和、`Mod=0`ならmod 2^64の和です。遅延伝搬を行うため非constです。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>value_type tree.all_prod() const</code> — O(1)</summary>

全要素の和を返します。

{% raw %}
```cpp
auto sum = tree.all_prod();
```
{% endraw %}

注意点: 空の木では0です。遅延作用が残っていても現在の和を返します。

</details>

<details class="api-operation" id="reverse" markdown="1">
<summary><code>void tree.reverse(int l, int r)</code> — 期待O(log(N+1))、全区間はO(1)</summary>

`[l,r)`の要素の順序を反転します。`0 <= l <= r <= size()`です。

{% raw %}
```cpp
tree.reverse(0, tree.size());
```
{% endraw %}

注意点: 空区間と長さ1の区間も有効です。各要素の値と全体和は変えません。

</details>

<details class="api-operation" id="apply" markdown="1">
<summary><code>void tree.apply(int l, int r, value_type a, value_type b)</code> — 期待O(log(N+1))、全区間はO(1)</summary>

`[l,r)`の各要素xを`a*x+b`に更新します。`0 <= l <= r <= size()`です。

{% raw %}
```cpp
tree.apply(0, tree.size(), 1, 5);
tree.apply(0, tree.size(), 0, 7);
```
{% endraw %}

注意点: 上の例は全要素への5加算、その後7代入です。作用は呼出順に合成します。空区間も有効で、係数は`Mod>0`なら正規化し、`Mod=0`なら積と和のwraparoundを使います。

</details>

<details class="api-operation" id="to-vector" markdown="1">
<summary><code>std::vector&lt;value_type&gt; tree.to_vector()</code> — O(N+1)</summary>

現在の論理的な順序で、全要素を新しいvectorにコピーして返します。

{% raw %}
```cpp
auto values = tree.to_vector();
```
{% endraw %}

注意点: 空なら空vectorです。返したvectorは独立しており、その後の木の変更の影響を受けません。追加領域O(N+1)を使います。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>ImplicitTreap(const ImplicitTreap&amp; other)</code> — O(P+1)</summary>

otherと同じ列を持つ独立した木を構築します。priorityと乱数状態もコピーします。

{% raw %}
```cpp
auto copy = tree;
copy.insert(copy.size(), 9);
```
{% endraw %}

注意点: 削除済みslotを含むpoolをコピーするため、現在のNだけで計算量は決まりません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>ImplicitTreap&amp; tree.operator=(ImplicitTreap other) noexcept</code> — コピー時O(P+Q+1)</summary>

値渡しの引数otherをコピー構築してから代入先と交換し、自身への参照を返します。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: Pはコピー元のpool slot数、Qは代入前の代入先のslot数です。自己代入も有効です。関数本体はnoexceptですが、引数のコピー構築では確保例外を送出する場合があり、その場合は代入先を変更しません。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>ImplicitTreap(ImplicitTreap&amp;&amp; other) noexcept</code> — O(1)</summary>

otherの所有領域を引き継ぎます。移動元は空になります。

{% raw %}
```cpp
auto moved = std::move(tree);
tree.insert(0, 1);
```
{% endraw %}

注意点: 例では`<utility>`のincludeが必要です。移動元も通常の空の木として操作できます。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>ImplicitTreap&amp; tree.operator=(ImplicitTreap other) noexcept</code> — 移動時O(Q+1)</summary>

値渡しの引数otherを移動構築してから代入先と交換し、自身への参照を返します。他オブジェクトからの移動では、移動元は空になります。

{% raw %}
```cpp
copy = std::move(tree);
```
{% endraw %}

注意点: Qは代入前の代入先のslot数です。`<utility>`が必要です。自己移動代入では元の状態を保ちます。

</details>

<details class="api-operation" id="destroy" markdown="1">
<summary><code>tree.~ImplicitTreap()</code> — O(P+1)</summary>

保持するpoolと予約領域を解放します。

{% raw %}
```cpp
{ blueberry::extra_fast::ImplicitTreap<> temporary; }
```
{% endraw %}

注意点: 通常はscope終了時に自動で呼ばれます。明示的な破棄後に同じオブジェクトを操作しないでください。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- 通常版の汎用Implicit Treapと、整数和・アフィン作用に絞った本実装を使い分けます。ACLには可変長列の挿入・削除・区間反転に対応する構造はありません。
- 対応問題はLibrary Checkerの[Dynamic Sequence Range Affine Range Sum](https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum)と[Range Reverse Range Sum](https://judge.yosupo.jp/problem/range_reverse_range_sum)です。
- 速度の比較測定は今回省略しています。ノード配置・一括構築・不要な処理の削減を行った実験版であり、最速や通常版に対する改善率は保証していません。
- [Fastestの調査と設計記録]({{ '/docs/development/fast-implicit-treap.html' | relative_url }})。

</details>
