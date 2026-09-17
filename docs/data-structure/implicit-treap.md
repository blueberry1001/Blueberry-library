---
title: Implicit Treap
documentation_of: //blueberry/data-structure/implicit-treap.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

挿入・削除で長さが変わる列を、位置をキーとするtreapで管理します。ACLの固定長segtree/lazy_segtreeと異なり、位置への挿入・削除と区間反転を扱います。旧 `blueberry/implicit_treap.hpp` のキー順集合とは別のデータ構造です。

`ImplicitTreap<S, op, e>` は通常のmonoid、`ImplicitTreap<S, op, e, F, mapping, composition, id>` は遅延作用付きです。`S op(S,S)` は結合的、`S e()` は両側単位元。非可換でも利用できます。各ノードに正順・逆順の両集約を保持し、reverseでは両者を入れ替えます。反転は**葉の並び順**だけを変え、葉のS内部を書き換えません。

作用はACLと同じ `S mapping(F,S)`、`F composition(F newer,F older)`、`F id()` です。合成は newer(older(x)) の順。mappingは単位元を保ち、opに分配し、全要素へ同じように作用する必要があります。位置依存の等差数列加算等は対象外です。区間和へのaffine作用では、Sに `{sum,length}` を含め、葉のlengthを1、eのlengthを0にします。Fの等値比較、S/Fのdefault constructorは不要。copy/move/assignment可能な値型を使ってください。

Nを現在の要素数、Hをclear以降の最大同時要素数に対応するpool slot数とします。木の操作は期待 O(log(N+1))、最悪 O(N)。insert/eraseはpool・free-listのvector増設を含め期待・償却 O(log(N+1))、単一呼出しの最悪 O(H+N)です。set/prod/reverse/applyも作業スタックの増設を含め期待・償却 O(log(N+1))（最悪 O(N)）です。構築と全列取得はO(N)、all_prod/size/emptyはO(1)。メモリは削除slotの再利用とvector容量を含めO(H)。clearは容量も解放します。op/mapping/copy等をO(1)とみなした計算量で、string等はその操作コストが掛かります。数値型のop/mapping/compositionにおける中間値を含むoverflowを避ける責任は呼出側にあります。

添字はint、0<=N<=INT_MAX、区間は `[l,r)`。満杯時の追加は禁止です。priorityは局所の擬似乱数で生成します。固定seedなので入力と操作列を再現可能ですが、敵対的なpriority依存入力に対する最悪時間保証はありません。内部split/merge/set/traversalは明示スタックを用い、木の高さに比例するC++再帰を行いません。

ノードはvectorと整数indexで所有し、eraseでslotを再利用します。get/prod/to_vectorはlazy伝搬のためnonconstですが論理的な値を変更しません。返り値は所有値で、内部参照・ノードhandleは公開しません。入力vectorの寿命に依存しません。利用者の代数演算やallocationが例外を投げた後の強い例外保証は提供しません。操作に使う演算は副作用を持たせないでください。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/data-structure/implicit-treap.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  using Tree = blueberry::ImplicitTreap<long long, op, e>;
  Tree tree(std::vector<long long>{1, 2, 3});
  tree.insert(1, 7);       // 1,7,2,3
  tree.reverse(0, 3);      // 2,7,1,3
  tree.set(2, 5);          // 2,7,5,3
  tree.erase(0);           // 7,5,3
  assert(tree.get(1) == 5);
  assert(tree.prod(0, 2) == 12);
  assert(tree.all_prod() == 15);
  assert((tree.to_vector() == std::vector<long long>{7, 5, 3}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `ImplicitTreap(const vector<S>& values = {}, uint64_t seed = 0x243f6a8885a308d3ULL)` | O(N) | [開く](#construct) |
| `Tree copy(const Tree& other)` | O(H) | [開く](#copy-construct) |
| `Tree& tree.operator=(const Tree& other)` | O(H + Hdst) | [開く](#copy-assign) |
| `Tree moved(Tree&& other)` | O(1) | [開く](#move-construct) |
| `Tree& tree.operator=(Tree&& other)` | O(Hdst) | [開く](#move-assign) |
| `tree.~Tree()` | O(H) | [開く](#destruct) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `bool tree.empty() const` | O(1) | [開く](#empty) |
| `void tree.clear()` | O(H) | [開く](#clear) |
| `void tree.insert(int p, const S& value)` | 期待・償却 O(log(N+1)) | [開く](#insert) |
| `void tree.erase(int p)` | 期待・償却 O(log(N+1)) | [開く](#erase) |
| `S tree.get(int p)` | 期待 O(log(N+1)) | [開く](#get) |
| `void tree.set(int p, const S& value)` | 期待・償却 O(log(N+1)) | [開く](#set) |
| `S tree.prod(int l, int r)` | 期待・償却 O(log(N+1)) | [開く](#prod) |
| `S tree.all_prod() const` | O(1) | [開く](#all-prod) |
| `void tree.reverse(int l, int r)` | 期待・償却 O(log(N+1)) | [開く](#reverse) |
| `void tree.apply(int l, int r, const F& action)` | 期待・償却 O(log(N+1)) | [開く](#apply) |
| `vector<S> tree.to_vector()` | O(N) | [開く](#to-vector) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>ImplicitTreap(const vector&lt;S&gt;&amp; values = {}, uint64_t seed = 0x243f6a8885a308d3ULL)</code> — O(N)</summary>

valuesの順序を保ち、Cartesian treeを線形時間で構築します。引数なしなら空列です。以下のTreeは利用者が定義したImplicitTreapの型aliasです。

{% raw %}
```cpp
using Tree = blueberry::ImplicitTreap<long long, op, e>;
Tree tree(std::vector<long long>{1, 2, 3}, 12345);
```
{% endraw %}

注意点: values.size()<=INT_MAX。指定seedは期待計算量の改善保証を与えません。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>Tree copy(const Tree&amp; other)</code> — O(H)</summary>

ノードpool・free list・内部状態を独立してコピーします。lazy未伝搬の状態もそのまま複製します。

{% raw %}
```cpp
Tree copy(tree);
```
{% endraw %}

注意点: 元とコピーの更新は互いに影響しません。現在のNだけでなく、確保済みslot数Hに比例します。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>Tree&amp; tree.operator=(const Tree&amp; other)</code> — O(H + Hdst)</summary>

既存の内容を破棄して独立したコピーを代入し、*thisを返します。

{% raw %}
```cpp
copy = tree;
```
{% endraw %}

注意点: 自己代入は有効です。Hdstは代入前の自身のpoolサイズです。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>Tree moved(Tree&amp;&amp; other)</code> — O(1)</summary>

poolの所有権を引き継ぎます。移動元を空の再利用可能な状態にします。

{% raw %}
```cpp
Tree moved(std::move(tree));
assert(tree.empty());
```
{% endraw %}

注意点: 移動後のtreeにもinsert等を呼べます。内部priority生成状態は引き継ぎます。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>Tree&amp; tree.operator=(Tree&amp;&amp; other)</code> — O(Hdst)</summary>

既存poolを解放して所有権を引き継ぎ、*thisを返します。

{% raw %}
```cpp
copy = std::move(moved);
assert(moved.empty());
```
{% endraw %}

注意点: 自己move代入は何もしません。移動元は空で再利用可能です。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>tree.~Tree()</code> — O(H)</summary>

すべてのpool slotと格納値・作用を破棄します。個別のnew/deleteや利用者による解放は不要です。

{% raw %}
```cpp
{ Tree temporary; } // スコープ終了時に全ノードを解放
```
{% endraw %}

注意点: destructorを通常コードから直接呼ぶ必要はありません。T/S/Fは通常の値型として破棄可能であることが前提です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

現在の要素数を返します。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 0以上INT_MAX以下です。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool tree.empty() const</code> — O(1)</summary>

要素が0個ならtrueです。

{% raw %}
```cpp
bool vacant = tree.empty();
```
{% endraw %}

注意点: prodの単位元と要素の有無は別なので、値から空判定しないでください。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void tree.clear()</code> — O(H)</summary>

全要素を破棄し、pool/free list/作業領域のvector容量も解放します。

{% raw %}
```cpp
tree.clear();
assert(tree.empty());
```
{% endraw %}

注意点: 乱数stateは初期seedへ戻しません。次のinsertで新しいpoolを作ります。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>void tree.insert(int p, const S&amp; value)</code> — 期待・償却 O(log(N+1))</summary>

位置pの直前に1要素を追加します。p=Nなら末尾です。

{% raw %}
```cpp
tree.insert(tree.size(), 9);
```
{% endraw %}

注意点: 0<=p<=N、N<INT_MAX。valueは1個の葉として格納します。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>void tree.erase(int p)</code> — 期待・償却 O(log(N+1))</summary>

位置pの1要素を削除し、後続の添字を1つ詰めます。

{% raw %}
```cpp
tree.erase(0);
```
{% endraw %}

注意点: 0<=p<N。空列へのeraseや範囲外はassert対象です。slotは次のinsertで再利用します。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>S tree.get(int p)</code> — 期待 O(log(N+1))</summary>

位置pの値を所有値として返します。

{% raw %}
```cpp
auto value = tree.get(0);
```
{% endraw %}

注意点: 0<=p<N。返り値の変更は列へ反映されません。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void tree.set(int p, const S&amp; value)</code> — 期待・償却 O(log(N+1))</summary>

位置pの葉をvalueで置き換えます。長さNは変わりません。

{% raw %}
```cpp
tree.set(0, 20);
```
{% endraw %}

注意点: 0<=p<N。過去のlazy作用を伝搬してから置き換えます。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S tree.prod(int l, int r)</code> — 期待・償却 O(log(N+1))</summary>

現在の列の順にopを畳み込んだ区間集約を返します。空区間はe()です。

{% raw %}
```cpp
auto value = tree.prod(0, tree.size());
```
{% endraw %}

注意点: 0<=l<=r<=N。内部split/mergeを行うためnonconstです。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>S tree.all_prod() const</code> — O(1)</summary>

全列の正順集約を返します。空列はe()です。

{% raw %}
```cpp
auto total = tree.all_prod();
```
{% endraw %}

注意点: 未伝搬のlazy/reverseがあっても有効な集約値です。

</details>

<details class="api-operation" id="reverse" markdown="1">
<summary><code>void tree.reverse(int l, int r)</code> — 期待・償却 O(log(N+1))</summary>

[l,r)の葉の並びを逆順にします。

{% raw %}
```cpp
tree.reverse(0, tree.size());
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間は何もしません。S内部の文字列等は反転しません。

</details>

<details class="api-operation" id="apply" markdown="1">
<summary><code>void tree.apply(int l, int r, const F&amp; action)</code> — 期待・償却 O(log(N+1))</summary>

[l,r)へpointwiseな作用を適用します。

{% raw %}
```cpp
struct Sum { long long sum; int length; };
struct Affine { long long a, b; };
constexpr auto combine = [](Sum x, Sum y) -> Sum { return {x.sum + y.sum, x.length + y.length}; };
constexpr auto unit = []() -> Sum { return {0, 0}; };
constexpr auto map = [](Affine f, Sum x) -> Sum { return {f.a * x.sum + f.b * x.length, x.length}; };
constexpr auto compose = [](Affine f, Affine g) -> Affine { return {f.a * g.a, f.a * g.b + f.b}; };
constexpr auto action_id = []() -> Affine { return {1, 0}; };
using LazyTree = blueberry::ImplicitTreap<Sum, combine, unit, Affine, map, compose, action_id>;
LazyTree lazy(std::vector<Sum>{{1, 1}, {2, 1}});
lazy.apply(0, 2, Affine{3, 4}); // 7,10
assert(lazy.all_prod().sum == 17);
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間は何もしません。compositionの引数は新作用→旧作用の順です。NoAction既定ではこの操作は恒等作用のみです。

</details>

<details class="api-operation" id="to-vector" markdown="1">
<summary><code>vector&lt;S&gt; tree.to_vector()</code> — O(N)</summary>

現在の列の値を先頭から順にコピーして返します。

{% raw %}
```cpp
auto values = tree.to_vector();
```
{% endraw %}

注意点: 追加メモリO(N)。lazy伝搬のためnonconstですが、列を削除しません。

</details>

## 出典・検証

公式問題: [Dynamic Sequence Range Affine Range Sum](https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum)、[Range Reverse Range Sum](https://judge.yosupo.jp/problem/range_reverse_range_sum)。固定seedのvector oracle、非可換連結、非可換作用合成、コピー・移動・slot再利用もテストします。

[Nyaanの反転可能RBST](https://nyaannyaan.github.io/library/rbst/lazy-reversible-rbst.hpp.html)、[CP-Algorithmsのtreap](https://github.com/cp-algorithms/cp-algorithms/blob/main/src/data_structures/treap.md)、公開提出[402977](https://judge.yosupo.jp/submission/402977)・[278236](https://judge.yosupo.jp/submission/278236)の設計を比較しました。Fastestはブロック単位の平衡木、次点は固定配列splayであり、同じtreapの定数倍差ではありません。公開ソースを転記せず、値所有のindex poolと標準的なtreap操作を独立実装しています。
