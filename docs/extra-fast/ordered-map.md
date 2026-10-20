---
title: Ordered Map / Set (SIMD)
documentation_of: //blueberry/extra-fast/ordered-map.hpp
---

[さらに高速なライブラリへ戻る]({{ "/extra-fast.html" | relative_url }})

## 概要

整数キーの動的16分B+木です。32/64bitの符号付き・符号なしキーに対し、挿入・削除・前後検索・順位・k番目を扱います。未知のキーをオンラインで追加できます。

OrderedMapと、同じ構造を使うOrderedSetを提供します。標準コンテナとの互換ラッパーではありません。

現在の要素数をN、過去に確保したノード量をMとすると、探索・削除は最悪O(log(N+1))、挿入は償却O(log(N+1))、メモリはO(M+1)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`OrderedMap(use_simd) / OrderedSet(use_simd)`](#construct) | O(1) |
| [`m.insert(key, value) / s.insert(key)`](#insert) | 償却 O(log(N+1))、再確保時 O(M+log(N+1)) |
| [`m.insert_or_assign(key, value)`](#insert-or-assign) | 償却 O(log(N+1))、再確保時 O(M+log(N+1)) |
| [`m[key]`](#subscript) | 償却 O(log(N+1))、再確保時 O(M+log(N+1)) |
| [`m.get(key)`](#get) | O(log(N+1)) |
| [`contains(key)`](#contains) | O(log(N+1)) |
| [`erase(key)`](#erase) | O(log(N+1)) |
| [`rank(key)`](#rank) | O(log(N+1)) |
| [`kth(k)`](#kth) | O(log(N+1)) |
| [`lower_bound(key)`](#lower-bound) | O(log(N+1)) |
| [`upper_bound(key)`](#upper-bound) | O(log(N+1)) |
| [`floor(key)`](#floor) | O(log(N+1)) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`for_each(f)`](#for-each) | O(N+1) |
| [`size() / empty()`](#size) | O(1) |
| [`clear()`](#clear) | O(M) |
| [`value_type / iterator / const_iteratorなど`](#iterator-types) | O(1) |
| [`begin() / cbegin()`](#begin) | O(1) |
| [`end() / cend()`](#end) | O(1) |
| [`rbegin() / crbegin()`](#rbegin) | O(1) |
| [`rend() / crend()`](#rend) | O(1) |
| [`find(key)`](#find) | O(log(N+1)) |
| [`erase(pos)`](#erase-iterator) | O(log(N+1)) |
| [`iteratorの型別名`](#iterator-traits) | O(1) |
| [`iteratorの構築・代入・const変換`](#iterator-construct) | O(1) |
| [`*it / it->`](#iterator-dereference) | O(1) |
| [`++it / it++`](#iterator-increment) | O(1) |
| [`--it / it--`](#iterator-decrement) | O(1) |
| [`it == other / it != other`](#iterator-equality) | O(1) |
| [`uses_simd() / simd_available()`](#simd) | O(1) |
| [`OrderedMap(other) / OrderedSet(other) / = other`](#copy) | O(Msrc+Mdst) |
| [`OrderedMap(std::move(other)) / OrderedSet(std::move(other)) / = std::move(other)`](#move) | 構築 O(1)、代入 O(Mdst+1) |
| [`m.swap(other)`](#swap) | O(1) |
| [`~OrderedMap() / ~OrderedSet()`](#destruct) | O(M+1) |
</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- Keyは32/64bit整数型。自然順序のみで、予約キーはありません。N<=INT_MAXが必要です。Nは現在の要素数、Mは過去に確保したノード量（現在のNより大きい場合があります）。メモリはO(M+1)。B=16を定数とし探索・削除は最悪O(log(N+1))、挿入は同じ償却計算量でvector再確保時にO(M)が加わります。

- Valueはtrivialかつtrivialなコピー構築・コピー代入が可能な型です。文字列などリソースを所有する非trivial型は対象外。Value{}を初期値に使います。キー・値は所有コピーで、返却optionalもコピーです。公開フィールドはありません。双方向反復子からMapの実pairとSetのconstキーを参照できます。MapはSIMD用キー列に加えてpair内にもキーを保持するため、反復子追加前の候補より保持量が増えます。Setではキーを重複保存しません。

- 挿入・削除・clear・代入・swapで既存の全反復子・参照が失効します。erase(pos)が返した反復子は有効です。ノード内の要素も移動するためstd::mapの参照安定性は提供しません。既存値だけをoperator[]の参照やiterator->second経由で変更する場合は、要素配置を変更しません。lower_bound/upper_bound/floor/kthはoptionalの所有コピーを返し、stdの反復子返却APIとは異なります。

- キーの最小値・最大値を含む全範囲を扱い、キー加算は行いません。整数型の変換や利用側の値更新で起きるoverflowは呼出側の責任です。

- x86のGCC/Clangでは対応命令を利用し、それ以外はスカラーにフォールバックします。`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`で強制スカラー、`BLUEBERRY_EXTRA_FAST_USE_ASM`で比較命令をGNUインラインasmに切り替えます。既定はintrinsicsで、asmを指定しても速度向上は保証しません。マクロ設定は全翻訳単位で一致させてください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <utility>
#include "blueberry/extra-fast/ordered-map.hpp"
int main() {
  blueberry::extra_fast::OrderedMap<> m;
  blueberry::extra_fast::OrderedSet<> s;
  assert(m.insert(4, 9));
  assert(!m.insert(4, 2));
  assert(m.get(4) == 9);
  m[4] = 12;
  assert(m.get(4) == 12);
  assert(s.insert(4));
  assert(s.contains(4));
  s.insert(2);
  assert(s.rank(4) == 1);
  assert(s.kth(0) == 2);
  assert(s.floor(3) == 2);
  for (auto& [key, value] : m) { assert(key == 4); value += 1; }
  assert(m.find(4)->second == 13);
  m.find(4)->second = 12;
  assert(*s.rbegin() == 4);
  unsigned visits = 0;
  m.for_each([&](auto key, auto value) { assert(key == 4 && value == 12); ++visits; });
  assert(visits == 1);
  assert(s.erase(4));
  assert(!s.contains(4));
  auto moved = std::move(m);
  assert(m.empty());
  assert(moved.get(4) == 12);
}

```
{% endraw %}

<details class="api-operation" id="construct" markdown="1">
<summary><code>OrderedMap&lt;Key, Value&gt;(bool use_simd = true); OrderedSet&lt;Key&gt;(bool use_simd = true)</code> — O(1)</summary>

注意点: 空で構築します。falseなら常にスカラー実装。trueでも対応環境でのみSIMDを使用します。Mapは `Value`、Setはキーのみを保持します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
blueberry::extra_fast::OrderedMap<std::uint32_t, unsigned> m(false);
blueberry::extra_fast::OrderedSet<std::uint32_t> s;
```
{% endraw %}

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>bool m.insert(Key key, Value value); bool s.insert(Key key)</code> — 償却 O(log(N+1))、再確保時 O(M+log(N+1))</summary>

注意点: 未登録なら挿入してtrue。既存キーには変更せずfalseを返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.insert(4, 9); s.insert(4);
```
{% endraw %}

</details>

<details class="api-operation" id="insert-or-assign" markdown="1">
<summary><code>bool m.insert_or_assign(Key key, Value value)</code> — 償却 O(log(N+1))、再確保時 O(M+log(N+1))</summary>

注意点: 挿入または値を上書きします。新規キーのときだけtrue。Setにはありません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.insert_or_assign(4, 12);
```
{% endraw %}

</details>

<details class="api-operation" id="subscript" markdown="1">
<summary><code>Value&amp; m.operator[](Key key)</code> — 償却 O(log(N+1))、再確保時 O(M+log(N+1))</summary>

注意点: 既存の値への参照を返します。なければValue{}で挿入します。Setにはありません。参照の失効条件は共通制約を参照してください。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m[4] += 1;
```
{% endraw %}

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>std::optional&lt;Value&gt; m.get(Key key) const</code> — O(log(N+1))</summary>

注意点: 値のコピーを返します。未登録ならnullopt。検索で挿入されません。Setにはありません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto value = m.get(4);
```
{% endraw %}

</details>

<details class="api-operation" id="contains" markdown="1">
<summary><code>bool m.contains(Key key) const; bool s.contains(Key key) const</code> — O(log(N+1))</summary>

注意点: キーがあればtrue。空のコンテナではfalse。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool found = s.contains(4);
```
{% endraw %}

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool m.erase(Key key); bool s.erase(Key key)</code> — O(log(N+1))</summary>

注意点: キーを削除し、存在していたときだけtrueを返します。空や未登録キーは変更しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool erased = m.erase(4);
```
{% endraw %}

</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int m.rank(Key key) const; int s.rank(Key key) const</code> — O(log(N+1))</summary>

注意点: key未満のキーの個数を返します。空なら0。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
int count = s.rank(4);
```
{% endraw %}

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;Key, Value&gt;&gt; m.kth(int k) const; std::optional&lt;Key&gt; s.kth(int k) const</code> — O(log(N+1))</summary>

注意点: 昇順0-indexedのk番目の要素をコピーして返します。k<0またはk>=size()ならnullopt。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto first = s.kth(0);
```
{% endraw %}

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;Key, Value&gt;&gt; m.lower_bound(Key key) const; std::optional&lt;Key&gt; s.lower_bound(Key key) const</code> — O(log(N+1))</summary>

注意点: key以上の最小キーの要素をコピーして返します。該当要素がなければnullopt。型の最小値・最大値もそのまま渡せます。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto entry = s.lower_bound(4);
```
{% endraw %}

</details>

<details class="api-operation" id="upper-bound" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;Key, Value&gt;&gt; m.upper_bound(Key key) const; std::optional&lt;Key&gt; s.upper_bound(Key key) const</code> — O(log(N+1))</summary>

注意点: keyより大きい最小キーの要素をコピーして返します。該当要素がなければnullopt。型の最小値・最大値もそのまま渡せます。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto entry = s.upper_bound(4);
```
{% endraw %}

</details>

<details class="api-operation" id="floor" markdown="1">
<summary><code>std::optional&lt;std::pair&lt;Key, Value&gt;&gt; m.floor(Key key) const; std::optional&lt;Key&gt; s.floor(Key key) const</code> — O(log(N+1))</summary>

注意点: key以下の最大キーの要素をコピーして返します。該当要素がなければnullopt。型の最小値・最大値もそのまま渡せます。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto entry = s.floor(4);
```
{% endraw %}

</details>

<details class="api-operation" id="for-each" markdown="1">
<summary><code>template&lt;class F&gt; void m.for_each(F&amp;&amp; f) const; template&lt;class F&gt; void s.for_each(F&amp;&amp; f) const</code> — O(N+1)</summary>

注意点: キーの昇順に全要素を訪問します。Mapはf(const Key&, const Value&)、Setはf(const Key&)を呼びます。コールバックの時間を除きます。空なら呼び出しません。呼出中の同じコンテナの変更は禁止。引数参照を呼出後に保存しないでください。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.for_each([](const auto& key, const auto& value) { (void)key; (void)value; });
s.for_each([](const auto& key) { (void)key; });
```
{% endraw %}

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int m.size() const; bool m.empty() const; s.size(); s.empty()</code> — O(1)</summary>

注意点: 重複しないキーの個数と、空かどうかを返します。Setも同じです。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto n = m.size(); bool is_empty = s.empty();
```
{% endraw %}

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void m.clear(); void s.clear()</code> — O(M)</summary>

注意点: 全要素を消去してsizeを0にします。確保した容量は保持します。既存の参照はすべて失効します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.clear(); s.clear();
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-types" markdown="1">
<summary><code>key_type; mapped_type; value_type; iterator; const_iterator; reverse_iterator; const_reverse_iterator</code> — O(1)</summary>

注意点: Mapのkey_type=Key、mapped_type=Value、value_type=std::pair<const Key,Value>です。Setにはkey_type=Key、value_type=Keyがありmapped_typeはありません。Mapのiteratorはvalue_type&、const_iteratorはconst value_type&を返します。Setのiterator=const_iteratorでconst Key&を返します。reverse_iteratorとconst_reverse_iteratorは対応するstd::reverse_iteratorです。Map要素のfirstはconstキー、secondは値で、非const iteratorからsecondを更新できます。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
using M = decltype(m);
using I = M::iterator;
using CI = M::const_iterator;
static_assert(std::bidirectional_iterator<I>);
static_assert(std::bidirectional_iterator<decltype(s)::iterator>);
```
{% endraw %}

</details>

<details class="api-operation" id="begin" markdown="1">
<summary><code>iterator begin(); const_iterator begin() const; const_iterator cbegin() const</code> — O(1)</summary>

注意点: 最小キーの要素を指す双方向反復子を返します。空ならend()と等しい値です。Map/Set共通。通常のfor(auto& [key,value]:m)で値を更新でき、Setの要素は変更できません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
for (auto& [key, value] : m) { (void)key; value += 1; }
for (const auto& key : s) { (void)key; }
```
{% endraw %}

</details>

<details class="api-operation" id="end" markdown="1">
<summary><code>iterator end(); const_iterator end() const; const_iterator cend() const</code> — O(1)</summary>

注意点: 末尾の次を示します。逆参照・++は禁止。非空のときだけ--して最大キーを参照できます。Map/Set共通。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
if (!m.empty()) { auto it = m.end(); --it; auto key = it->first; (void)key; }
```
{% endraw %}

</details>

<details class="api-operation" id="rbegin" markdown="1">
<summary><code>reverse_iterator rbegin(); const_reverse_iterator rbegin() const; const_reverse_iterator crbegin() const</code> — O(1)</summary>

注意点: 最大キーから小さい順に進む逆反復子を返します。空ならrend()と等しい値。Map/Set共通。参照のconst性は対応する順反復子と同じです。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
for (auto it = s.rbegin(); it != s.rend(); ++it) { auto key = *it; (void)key; }
```
{% endraw %}

</details>

<details class="api-operation" id="rend" markdown="1">
<summary><code>reverse_iterator rend(); const_reverse_iterator rend() const; const_reverse_iterator crend() const</code> — O(1)</summary>

注意点: 逆順走査の終端を返します。逆参照・++は禁止。非空なら--できます。Map/Set共通。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool same = s.crbegin() == s.crend(); // 空かどうか
(void)same;
```
{% endraw %}

</details>

<details class="api-operation" id="find" markdown="1">
<summary><code>iterator find(Key key); const_iterator find(Key key) const</code> — O(log(N+1))</summary>

注意点: 一致するキーの反復子を返します。なければend()。検索で挿入しません。getやlower_boundなどのoptionalコピーを返す操作とは返却型が異なります。Map/Set共通。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto it = m.find(4);
if (it != m.end()) it->second = 12;
```
{% endraw %}

</details>

<details class="api-operation" id="erase-iterator" markdown="1">
<summary><code>iterator erase(const_iterator pos)</code> — O(log(N+1))</summary>

注意点: posはこのコンテナの有効な要素反復子で、end()は禁止です。その要素を消して直後の要素を指す新しい反復子を返します。末尾を消したらend()。消去により既存の全反復子・参照が失効します。Map/Set共通。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
for (auto it = s.begin(); it != s.end();) it = s.erase(it);
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-traits" markdown="1">
<summary><code>It::value_type; difference_type; iterator_category; iterator_concept; reference; pointer</code> — O(1)</summary>

注意点: Itはiteratorまたはconst_iteratorです。value_typeは所属コンテナのvalue_type、difference_typeはptrdiff_t、category/conceptはbidirectional_iterator_tagです。reference/pointerは上記const性の参照/ポインタ型。整数差やランダムアクセスは提供しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
using D = std::iterator_traits<decltype(m.begin())>::difference_type;
D distance = std::distance(m.begin(), m.end()); // この距離計算はO(N)
(void)distance;
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-construct" markdown="1">
<summary><code>It(); It(const It&amp;); It(It&amp;&amp;); It&amp; operator=(const It&amp;); It&amp; operator=(It&amp;&amp;); const_iterator(iterator); ~It()</code> — O(1)</summary>

注意点: default構築はどのコンテナにも属さない特異値です。特異値どうしの比較、コピー・代入・破棄だけができます。コピー・移動は同じ位置を指し、移動元も同じ位置のままです。Mapはiteratorからconst_iteratorへ暗黙変換でき、逆は不可。Setは両型が同一です。代入は*thisへの参照を返します。反復子の破棄ではコンテナや要素を解放しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
decltype(m)::iterator singular;
auto it = m.begin();
decltype(m)::const_iterator ci = it;
auto copied = ci; copied = ci;
(void)singular;
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-dereference" markdown="1">
<summary><code>It::reference operator*() const; It::pointer operator-&gt;() const</code> — O(1)</summary>

注意点: 有効な要素反復子のみ逆参照できます。Mapは実際のpair<const Key,Value>への参照なので一時proxyではありません。firstの変更は禁止され、mutable iteratorのsecondだけ更新できます。Setはconst Key参照です。失効後の参照の使用は禁止。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
if (auto it = m.find(4); it != m.end()) { auto& entry = *it; entry.second += 1; }
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-increment" markdown="1">
<summary><code>It&amp; operator++(); It operator++(int)</code> — O(1)</summary>

注意点: 有効な要素からキー順で次へ進みます。最大キーからはend()へ進めます。end()や特異値の++は禁止。prefixは*this参照、postfixは更新前の反復子を返します。反復子を進めてもコンテナと要素参照は変更しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
if (!s.empty()) { auto it = s.begin(); auto old = it++; (void)old; }
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-decrement" markdown="1">
<summary><code>It&amp; operator--(); It operator--(int)</code> — O(1)</summary>

注意点: 直前のキーへ進みます。非空のend()から最大キーへ進めます。begin()・空のend()・特異値の--は禁止。prefixは*this参照、postfixは更新前の反復子を返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
if (!m.empty()) { auto it = m.end(); --it; }
```
{% endraw %}

</details>

<details class="api-operation" id="iterator-equality" markdown="1">
<summary><code>bool operator==(other) const; bool operator!=(other) const</code> — O(1)</summary>

注意点: 同じ所有コンテナの同じ位置なら等しく、Mapのconst/非const間でも比較できます。異なるコンテナは等しくありません。default構築の特異値どうしは等しい値です。失効した反復子は比較にも使わないでください。!=は==の否定です。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto it = m.begin();
decltype(m)::const_iterator ci = it;
bool same = it == ci;
(void)same;
```
{% endraw %}

</details>

<details class="api-operation" id="simd" markdown="1">
<summary><code>bool uses_simd() const; static bool simd_available()</code> — O(1)</summary>

注意点: AVX2の実行時CPU判定により利用可能かを返します。uses_simdはこのインスタンスの選択。Map/Set共通。FORCE_SCALAR指定時は両方false。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool enabled = m.uses_simd(); bool available = decltype(s)::simd_available();
```
{% endraw %}

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>OrderedMap(const OrderedMap&amp;); OrderedMap&amp; operator=(const OrderedMap&amp;); Setも同様</code> — O(Msrc+Mdst)</summary>

注意点: 内容とSIMD設定を独立にコピーします。コピー構築にはコピー元の量のみ、代入には破棄対象の量も含みます。自己代入可能。確保失敗時は代入先を保持します。代入は*thisへの参照を返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto copied = m; copied = m; auto set_copy = s; set_copy = s;
```
{% endraw %}

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>OrderedMap(OrderedMap&amp;&amp;) noexcept; OrderedMap&amp; operator=(OrderedMap&amp;&amp;) noexcept; Setも同様</code> — 構築 O(1)、代入 O(Mdst+1)</summary>

注意点: 所有メモリを移動します。移動元は空になり、再挿入できます。自己移動代入も可能。代入は*thisへの参照を返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto moved = std::move(m); m = std::move(moved);
```
{% endraw %}

</details>

<details class="api-operation" id="swap" markdown="1">
<summary><code>void OrderedMap::swap(OrderedMap&amp; other) noexcept</code> — O(1)</summary>

注意点: 内容・所有メモリ・SIMD設定を交換します。Setには公開swapメンバーはありません。参照を保持して交換する使い方は保証しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto other = m; m.swap(other);
```
{% endraw %}

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~OrderedMap(); ~OrderedSet()</code> — O(M+1)</summary>

注意点: 所有メモリを解放します。保存した参照は失効します。Valueがポインタなら参照先は解放しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
{ blueberry::extra_fast::OrderedSet<> temporary; }
```
{% endraw %}

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Library Checker ordered_set 409360](https://judge.yosupo.jp/submission/409360)のSIMD探索と、[Abseil B-tree設計](https://abseil.io/about/design/btree)のメモリ配置を調査し、独立に実装しました。元提出は30bitのdirectoryと適応bucketを使う別構造です。先読みI/Oやキー範囲制限は採用していません。
- `verify/extra-fast/ordered-set.test.cpp` と `ordered-map.test.cpp`、固定seedランダムテストで標準コンテナと照合します。
- 比較条件・改善しないケースは[測定記録]({{ "/docs/development/extra-fast-more.html" | relative_url }})を参照してください。

</details>
