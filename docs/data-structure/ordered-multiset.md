---
title: Ordered Multiset
documentation_of: //blueberry/data-structure/ordered-multiset.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

キー順の多重集合に、重複込みの順位取得と順位区間のmonoid集約を加えます。旧 `blueberry/implicit_treap.hpp` の実際の用途の移行先です。位置で並びを管理する `ImplicitTreap`、重複を持たない `OrderedSet` と使い分けます。ACLには対応するオンライン順序付き多重集合はありません。

`OrderedMultiset<T, op, e, Compare = std::less<T>>`。Tはキー兼葉の集約値、`T op(T,T)` は結合的、`T e()` は両側単位元です。非可換でも**Compare順のinorder**に集約します。Compareはstrict weak orderingで、`!comp(a,b) && !comp(b,a)` を同値とします。同値クラス内は挿入順を維持し、erase(value)は最初の同値要素を1つ消します。演算子==は不要で、等価性はCompareだけで決まります。

T/Compareはcopy/move/assignment可能な値型を使ってください。Tのdefault constructorは不要です。比較は葉の値だけに適用し、集約結果の値をキーとして探索しません。利用者のopや比較は副作用を持たせないでください。整数和等のoverflowを避ける型・範囲は利用者の責任です。

Nを現在の要素数、Hをclear以降の最大同時要素数に対応するpool slot数とすると、探索・区間集約は期待O(log(N+1))、最悪O(N)。insert/erase/erase_kthはpool・free-listのvector増設を含め期待・償却O(log(N+1))、単一呼出しの最悪O(H+N)です。削除時にもfree-list増設で過去の最大要素数Hに比例するコピーが生じ得ます。size/empty/all_prodはO(1)。演算・比較・値コピーのコストをO(1)とみなします。メモリは削除slotとvector容量を含めO(H)、clearは容量も解放します。添字/個数はint、0<=N<=INT_MAXで、満杯時のinsertは禁止です。

pool/indexでノードを所有し、削除slotを再利用します。返り値は所有値で内部参照・iteratorは公開しません。優先度は局所の固定seed擬似乱数。敵対的入力での最悪時間保証はなく、内部再帰の深さは木の高さ（期待O(log(N+1))、最悪O(N)）です。allocation/比較/代数演算が例外を投げた後の強い例外保証は提供しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include "blueberry/data-structure/ordered-multiset.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  using Tree = blueberry::OrderedMultiset<long long, op, e>;
  Tree tree;
  tree.insert(5); tree.insert(2); tree.insert(5);
  assert(tree.count(5) == 2);
  assert(tree.rank(5) == 1);
  assert(tree.kth(2).value() == 5);
  assert(tree.prod(1, 3) == 10);
  assert(tree.erase(5));
  assert(tree.count(5) == 1);
  assert(!tree.erase(9));
  assert(!tree.kth(-1));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `OrderedMultiset(Compare compare = {}, uint64_t seed = 0x243f6a8885a308d3ULL)` | O(1) | [開く](#construct) |
| `Tree copy(const Tree& other)` | O(H) | [開く](#copy-construct) |
| `Tree& tree.operator=(const Tree& other)` | O(H + Hdst) | [開く](#copy-assign) |
| `Tree moved(Tree&& other)` | O(1) | [開く](#move-construct) |
| `Tree& tree.operator=(Tree&& other)` | O(Hdst) | [開く](#move-assign) |
| `tree.~Tree()` | O(H) | [開く](#destruct) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `bool tree.empty() const` | O(1) | [開く](#empty) |
| `void tree.clear()` | O(H) | [開く](#clear) |
| `void tree.insert(const T& value)` | 期待・償却 O(log(N+1)) | [開く](#insert) |
| `bool tree.erase(const T& value)` | 期待・償却 O(log(N+1)) | [開く](#erase) |
| `void tree.erase_kth(int k)` | 期待・償却 O(log(N+1)) | [開く](#erase-kth) |
| `bool tree.contains(const T& value) const` | 期待 O(log(N+1)) | [開く](#contains) |
| `int tree.count(const T& value) const` | 期待 O(log(N+1)) | [開く](#count) |
| `int tree.rank(const T& value) const` | 期待 O(log(N+1)) | [開く](#rank) |
| `optional<T> tree.kth(int k) const` | 期待 O(log(N+1)) | [開く](#kth) |
| `optional<T> tree.lower_bound(const T& value) const` | 期待 O(log(N+1)) | [開く](#lower-bound) |
| `optional<T> tree.upper_bound(const T& value) const` | 期待 O(log(N+1)) | [開く](#upper-bound) |
| `optional<T> tree.floor(const T& value) const` | 期待 O(log(N+1)) | [開く](#floor) |
| `T tree.prod(int l, int r) const` | 期待 O(log(N+1)) | [開く](#prod) |
| `T tree.all_prod() const` | O(1) | [開く](#all-prod) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>OrderedMultiset(Compare compare = {}, uint64_t seed = 0x243f6a8885a308d3ULL)</code> — O(1)</summary>

空集合を作り、比較器を値として所有します。以下のTreeは利用者が定義したOrderedMultisetの型aliasです。

{% raw %}
```cpp
using Tree = blueberry::OrderedMultiset<long long, op, e>;
Tree tree;
```
{% endraw %}

注意点: Compareがstateを持つ場合も、その比較順をコピー・移動後に維持します。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>Tree copy(const Tree&amp; other)</code> — O(H)</summary>

ノードpool・free list・内部状態を独立してコピーします。Compareとpriority生成状態もコピーします。

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

重複を含む要素数を返します。

{% raw %}
```cpp
int n = tree.size();
```
{% endraw %}

注意点: 異なるキーの数ではありません。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool tree.empty() const</code> — O(1)</summary>

要素が0個ならtrueです。

{% raw %}
```cpp
bool vacant = tree.empty();
```
{% endraw %}

注意点: 空集合のall_prodはe()です。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void tree.clear()</code> — O(H)</summary>

全slotとfree listの容量を解放して空にします。

{% raw %}
```cpp
tree.clear();
assert(tree.empty());
```
{% endraw %}

注意点: 比較器と乱数stateは保持します。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>void tree.insert(const T&amp; value)</code> — 期待・償却 O(log(N+1))</summary>

valueを1個追加します。同値要素があっても追加し、同値クラスの末尾に入れます。

{% raw %}
```cpp
tree.insert(5); tree.insert(5);
```
{% endraw %}

注意点: N<INT_MAX。入力の寿命に依存せず値をコピーします。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool tree.erase(const T&amp; value)</code> — 期待・償却 O(log(N+1))</summary>

最初のCompare同値要素を1つ削除してtrue。存在しなければ何もせずfalseです。

{% raw %}
```cpp
bool removed = tree.erase(5);
```
{% endraw %}

注意点: 同値要素を全部消す操作ではありません。空集合でもfalseを返します。

</details>

<details class="api-operation" id="erase-kth" markdown="1">
<summary><code>void tree.erase_kth(int k)</code> — 期待・償却 O(log(N+1))</summary>

重複込み0-indexed順位kの要素を1つ削除します。

{% raw %}
```cpp
tree.erase_kth(0);
```
{% endraw %}

注意点: 0<=k<N。範囲外はassert対象です。

</details>

<details class="api-operation" id="contains" markdown="1">
<summary><code>bool tree.contains(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

Compare同値の要素が1個以上あればtrueです。

{% raw %}
```cpp
bool found = tree.contains(5);
```
{% endraw %}

注意点: Tのoperator==は使用しません。

</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>int tree.count(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

Compare同値要素の個数を返します。

{% raw %}
```cpp
int duplicates = tree.count(5);
```
{% endraw %}

注意点: 存在しない値・空集合では0です。

</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int tree.rank(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

Compareでvalueより前にある要素数を返します。lower_boundの挿入順位です。

{% raw %}
```cpp
int position = tree.rank(5);
```
{% endraw %}

注意点: 存在しない値でも0..Nの順位を返します。旧indexの「不存在なら-1」とは異なります。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>optional&lt;T&gt; tree.kth(int k) const</code> — 期待 O(log(N+1))</summary>

重複込み順位kの値を返します。

{% raw %}
```cpp
auto value = tree.kth(0);
```
{% endraw %}

注意点: k<0またはk>=Nならnullopt。返り値の変更は集合へ反映されません。

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>optional&lt;T&gt; tree.lower_bound(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

valueより前でない最初の要素を返します。

{% raw %}
```cpp
auto value = tree.lower_bound(5);
```
{% endraw %}

注意点: 存在しなければnullopt。同値クラス内では最初に挿入された生存要素です。

</details>

<details class="api-operation" id="upper-bound" markdown="1">
<summary><code>optional&lt;T&gt; tree.upper_bound(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

valueより厳密に後にある最初の要素を返します。

{% raw %}
```cpp
auto value = tree.upper_bound(5);
```
{% endraw %}

注意点: 存在しなければnullopt。Compare順を使うので、greater<T>では数値の向きが逆です。

</details>

<details class="api-operation" id="floor" markdown="1">
<summary><code>optional&lt;T&gt; tree.floor(const T&amp; value) const</code> — 期待 O(log(N+1))</summary>

valueより後でない最後の要素を返します。

{% raw %}
```cpp
auto value = tree.floor(5);
```
{% endraw %}

注意点: 存在しなければnullopt。同値クラス内では最後に挿入された生存要素です。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T tree.prod(int l, int r) const</code> — 期待 O(log(N+1))</summary>

順位区間[l,r)をCompare順・同値クラス挿入順で集約します。

{% raw %}
```cpp
auto subtotal = tree.prod(0, tree.size());
```
{% endraw %}

注意点: 0<=l<=r<=N。空区間はe()で、opの可換性は不要です。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>T tree.all_prod() const</code> — O(1)</summary>

集合全体の順序付き集約を返します。

{% raw %}
```cpp
auto total = tree.all_prod();
```
{% endraw %}

注意点: 空集合はe()。値の返却であり内部参照ではありません。

</details>

## 出典・検証

[Point Set Range Composite (Large Array)](https://judge.yosupo.jp/problem/point_set_range_composite_large_array) の専用verifyでは、64bit座標をキーとして更新済みの関数だけ保存し、`prod(rank(l),rank(r))` で合成します。未更新位置は恒等関数なので格納不要です。

公式問題: [Double Ended Priority Queue](https://judge.yosupo.jp/problem/double_ended_priority_queue)で重複の挿入・端点削除、[Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite)で非可換な順位区間集約を検証します。固定seedのstable sorted-vector oracleで全bound/count/rank、同値でも異なる値の順序、copy/move、削除slot再利用を補います。

標準的なtreapの回転・優先度mergeを独立実装しています。旧実装にあった未存在erase/空root参照や、op/eを無視した加算、ノードの解放漏れを引き継ぎません。[CP-Algorithms](https://github.com/cp-algorithms/cp-algorithms/blob/main/src/data_structures/treap.md)と公開LC提出のAPI/データ配置を調査しました。公開ソースは転記していません。公開上位の専用min-max queue、逆写像を使う多分木、固定長segment treeは提供機能が異なるので、順位集約付き多重集合との同一機能の速度差とは扱いません。
