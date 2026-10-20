---
title: Hash Map / Set (SIMD)
documentation_of: //blueberry/extra-fast/hash-map.hpp
---

[さらに高速なライブラリへ戻る]({{ "/extra-fast.html" | relative_url }})

## 概要

整数キー用の動的ハッシュ表です。16スロットのタグをSSE2で同時に比較します。挿入・削除・値の更新をオンラインで扱い、順序は保持しません。

HashMapと、同じ構造を使うHashSetを提供します。標準コンテナとの互換ラッパーではありません。

要素数をN、スロット数をCとすると、検索・削除は平均O(1)で最悪O(C)、挿入は平均償却O(1)、再配置時の最悪はO(Cnew+N²)です。メモリはO(C)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`HashMap(use_simd) / HashSet(use_simd)`](#construct) | O(1) |
| [`m.insert(key, value) / s.insert(key)`](#insert) | 平均償却 O(1)、最悪 O(C+N²) |
| [`m.insert_or_assign(key, value)`](#insert-or-assign) | 平均償却 O(1)、最悪 O(C+N²) |
| [`m[key]`](#subscript) | 平均償却 O(1)、最悪 O(C+N²) |
| [`m.get(key)`](#get) | 平均 O(1)、最悪 O(C) |
| [`contains(key)`](#contains) | 平均 O(1)、最悪 O(C) |
| [`erase(key)`](#erase) | 平均 O(1)、最悪 O(C) |
| [`reserve(count)`](#reserve) | 平均 O(Cnew+N)、最悪 O(Cnew+N²) |
| [`capacity()`](#capacity) | O(1) |
| [`for_each(f)`](#for-each) | O(C) |
| [`size() / empty()`](#size) | O(1) |
| [`clear()`](#clear) | O(C) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`uses_simd() / simd_available()`](#simd) | O(1) |
| [`HashMap(other) / HashSet(other) / = other`](#copy) | O(Csrc+Cdst) |
| [`HashMap(std::move(other)) / HashSet(std::move(other)) / = std::move(other)`](#move) | 構築 O(1)、代入 O(Cdst+1) |
| [`m.swap(other)`](#swap) | O(1) |
| [`~HashMap() / ~HashSet()`](#destruct) | O(C+1) |
</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

Keyはboolを除く64bit以下の整数型。予約キーはありません。Nは現在の要素数、Cは確保したスロット数です。メモリはO(C)。splitmix64による固定ハッシュを使い、平均計算量はハッシュが十分分散する場合です。探索・削除の最悪はO(C)、再配置を含む挿入・reserveの最悪はO(Cnew+N²)。キーの順序に関するAPIはありません。

Valueはtrivialかつtrivialなコピー構築・コピー代入が可能な型です。文字列などリソースを所有する非trivial型は対象外。Value{}を初期値に使います。キー・値は所有コピーで、返却optionalもコピーです。公開フィールドはありません。イテレータはありません。

operator[]の参照は再配置・clear・代入・swap・その要素のeraseで失効します。reserveや挿入は再配置を起こし得るためstd::unordered_mapの参照安定性は提供しません。

キーの最小値・最大値を含む全範囲を扱い、キー加算は行いません。内部ハッシュの符号なし演算のwrapは意図的です。整数型の変換や利用側の値更新で起きるoverflowは呼出側の責任です。

x86のGCC/Clangでは対応命令を利用し、それ以外はスカラーにフォールバックします。`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`で強制スカラー、`BLUEBERRY_EXTRA_FAST_USE_ASM`で比較命令をGNUインラインasmに切り替えます。既定はintrinsicsで、asmを指定しても速度向上は保証しません。マクロ設定は全翻訳単位で一致させてください。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <utility>
#include "blueberry/extra-fast/hash-map.hpp"
int main() {
  blueberry::extra_fast::HashMap<> m;
  blueberry::extra_fast::HashSet<> s;
  assert(m.insert(4, 9));
  assert(!m.insert(4, 2));
  assert(m.get(4) == 9);
  m[4] = 12;
  assert(m.get(4) == 12);
  assert(s.insert(4));
  assert(s.contains(4));
  m.reserve(100);
  assert(m.capacity() >= 100);
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
<summary><code>HashMap&lt;Key, Value&gt;(bool use_simd = true); HashSet&lt;Key&gt;(bool use_simd = true)</code> — O(1)</summary>

注意点: 空で構築します。falseなら常にスカラー実装。trueでも対応環境でのみSIMDを使用します。Mapは `Value`、Setはキーのみを保持します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
blueberry::extra_fast::HashMap<std::uint64_t, unsigned> m(false);
blueberry::extra_fast::HashSet<std::uint64_t> s;
```
{% endraw %}

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>bool m.insert(Key key, Value value); bool s.insert(Key key)</code> — 平均償却 O(1)、最悪 O(C+N²)</summary>

注意点: 未登録なら挿入してtrue。既存キーには変更せずfalseを返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.insert(4, 9); s.insert(4);
```
{% endraw %}

</details>

<details class="api-operation" id="insert-or-assign" markdown="1">
<summary><code>bool m.insert_or_assign(Key key, Value value)</code> — 平均償却 O(1)、最悪 O(C+N²)</summary>

注意点: 挿入または値を上書きします。新規キーのときだけtrue。Setにはありません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.insert_or_assign(4, 12);
```
{% endraw %}

</details>

<details class="api-operation" id="subscript" markdown="1">
<summary><code>Value&amp; m.operator[](Key key)</code> — 平均償却 O(1)、最悪 O(C+N²)</summary>

注意点: 既存の値への参照を返します。なければValue{}で挿入します。Setにはありません。参照の失効条件は共通制約を参照してください。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m[4] += 1;
```
{% endraw %}

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>std::optional&lt;Value&gt; m.get(Key key) const</code> — 平均 O(1)、最悪 O(C)</summary>

注意点: 値のコピーを返します。未登録ならnullopt。検索で挿入されません。Setにはありません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto value = m.get(4);
```
{% endraw %}

</details>

<details class="api-operation" id="contains" markdown="1">
<summary><code>bool m.contains(Key key) const; bool s.contains(Key key) const</code> — 平均 O(1)、最悪 O(C)</summary>

注意点: キーがあればtrue。空のコンテナではfalse。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool found = s.contains(4);
```
{% endraw %}

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool m.erase(Key key); bool s.erase(Key key)</code> — 平均 O(1)、最悪 O(C)</summary>

注意点: キーを削除し、存在していたときだけtrueを返します。空や未登録キーは変更しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool erased = m.erase(4);
```
{% endraw %}

</details>

<details class="api-operation" id="reserve" markdown="1">
<summary><code>void m.reserve(std::size_t count); void s.reserve(std::size_t count)</code> — 平均 O(Cnew+N)、最悪 O(Cnew+N²)</summary>

注意点: count要素を負荷率7/8以下で保持できる容量を確保します。count<size()でも縮小せず、reserve(0)は空でも16スロット確保します。消去履歴のある表では再配置が起こり得ます。容量が表現できなければlength_error、確保失敗ならbad_alloc。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.reserve(1000); s.reserve(1000);
```
{% endraw %}

</details>

<details class="api-operation" id="capacity" markdown="1">
<summary><code>std::size_t m.capacity() const; std::size_t s.capacity() const</code> — O(1)</summary>

注意点: キーを置くスロット数C。初期状態は0。負荷率上限は7/8で、clear後も容量を保持します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto slots = m.capacity();
```
{% endraw %}

</details>

<details class="api-operation" id="for-each" markdown="1">
<summary><code>template&lt;class F&gt; void m.for_each(F&amp;&amp; f) const; template&lt;class F&gt; void s.for_each(F&amp;&amp; f) const</code> — O(C)</summary>

注意点: 順序を保証せずに全要素を訪問します。Mapはf(const Key&, const Value&)、Setはf(const Key&)を呼びます。コールバックの時間を除きます。空なら呼び出しません。呼出中の同じコンテナの変更は禁止。引数参照を呼出後に保存しないでください。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.for_each([](const auto& key, const auto& value) { (void)key; (void)value; });
s.for_each([](const auto& key) { (void)key; });
```
{% endraw %}

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>std::size_t m.size() const; bool m.empty() const; s.size(); s.empty()</code> — O(1)</summary>

注意点: 重複しないキーの個数と、空かどうかを返します。Setも同じです。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto n = m.size(); bool is_empty = s.empty();
```
{% endraw %}

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void m.clear(); void s.clear()</code> — O(C)</summary>

注意点: 全要素を消去してsizeを0にします。確保した容量は保持します。既存の参照はすべて失効します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
m.clear(); s.clear();
```
{% endraw %}

</details>

<details class="api-operation" id="simd" markdown="1">
<summary><code>bool uses_simd() const; static bool simd_available()</code> — O(1)</summary>

注意点: コンパイル対象のSSE2対応により利用可能かを返します。uses_simdはこのインスタンスの選択。Map/Set共通。FORCE_SCALAR指定時は両方false。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
bool enabled = m.uses_simd(); bool available = decltype(s)::simd_available();
```
{% endraw %}

</details>

<details class="api-operation" id="copy" markdown="1">
<summary><code>HashMap(const HashMap&amp;); HashMap&amp; operator=(const HashMap&amp;); Setも同様</code> — O(Csrc+Cdst)</summary>

注意点: 内容とSIMD設定を独立にコピーします。コピー構築にはコピー元の量のみ、代入には破棄対象の量も含みます。自己代入可能。確保失敗時は代入先を保持します。代入は*thisへの参照を返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto copied = m; copied = m; auto set_copy = s; set_copy = s;
```
{% endraw %}

</details>

<details class="api-operation" id="move" markdown="1">
<summary><code>HashMap(HashMap&amp;&amp;) noexcept; HashMap&amp; operator=(HashMap&amp;&amp;) noexcept; Setも同様</code> — 構築 O(1)、代入 O(Cdst+1)</summary>

注意点: 所有メモリを移動します。移動元は空になり、再挿入できます。自己移動代入も可能。代入は*thisへの参照を返します。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto moved = std::move(m); m = std::move(moved);
```
{% endraw %}

</details>

<details class="api-operation" id="swap" markdown="1">
<summary><code>void HashMap::swap(HashMap&amp; other) noexcept</code> — O(1)</summary>

注意点: 内容・所有メモリ・SIMD設定を交換します。Setには公開swapメンバーはありません。参照を保持して交換する使い方は保証しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
auto other = m; m.swap(other);
```
{% endraw %}

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~HashMap(); ~HashSet()</code> — O(C+1)</summary>

注意点: 所有メモリを解放します。保存した参照は失効します。Valueがポインタなら参照先は解放しません。

{% raw %}
```cpp
// mとsは上の型で構築済みとする。
{ blueberry::extra_fast::HashSet<> temporary; }
```
{% endraw %}

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Abseil Swiss Table設計](https://abseil.io/about/design/swisstables)のタグ同時比較と、[Library Checker associative_array 362188](https://judge.yosupo.jp/submission/362188)の表配置を調査し、動的容量・削除を含めて独立に実装しました。
- `verify/extra-fast/hash-map.test.cpp`、固定seedランダムテストでunordered_map/setと照合します。HashSetの公式問題への直接verifyはなく、同じ表とランダムテストで検証します。
- 比較条件・改善しないケースは[測定記録]({{ "/docs/development/extra-fast-more.html" | relative_url }})を参照してください。

</details>
