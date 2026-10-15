---
title: Persistent Array
documentation_of: //blueberry/data-structure/persistent-array.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

集約演算を要求しない完全永続配列です。`PersistentArray<T>` は任意の版への一点代入から新しい版を作り、版0が初期配列です。区間積が必要ならPersistentSegmentTreeを使います。

Nは長さ、Uはset回数、Vは版数。get/setはO(log(N+1))、setの確保時間は償却評価です。メモリはO(N+U log(N+1)+V)で、一様初期値の初期ノードはO(1)です。空列でもforkできます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`PersistentArray<T>`](#default) | O(1) |
| [`PersistentArray<T>`](#fill) | O(1) |
| [`PersistentArray<T>`](#vector) | O(N) |
| [`object.size`](#size) | O(1) |
| [`object.versions`](#versions) | O(1) |
| [`object.fork`](#fork) | 償却 O(1) |
| [`object.get`](#get) | O(log(N+1)) |
| [`object.set`](#set) | 償却 O(log(N+1)) |
| [`Type`](#copy-constructor) | O(M) |
| [`Type`](#move-constructor) | O(1) |
| [`operator=`](#copy-assignment) | O(M + D) |
| [`operator=`](#move-assignment) | O(1 + D) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLに永続配列はありません。既存の永続構造のAPIは変更しません。
- Tは既定構築・コピー・代入可能で、演算は要求しません。計算量は各T操作がO(1)の場合です。
- 版は同じオブジェクト内でのみ有効で、更新・再確保で無効化されません。返り値は値、入力はコピーです。
- 版・ノード数はINT_MAX未満です。vectorの再確保を伴う単一更新は、保持ノード数に比例し得ます。
- 空列のget/setはできません。

## 操作の補足 {#操作の補足}

- 以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/persistent-array.hpp"
int main() {
  blueberry::PersistentArray<int> a(3, -1);
  int v = a.set(0, 1, 7);
  int w = a.set(0, 1, 9);
  assert(a.get(0, 1) == -1 && a.get(v, 1) == 7 && a.get(w, 1) == 9);
  assert(a.get(a.fork(v), 1) == 7);
}
```
{% endraw %}

<details class="api-operation" id="default" markdown="1">
<summary><code>PersistentArray&lt;T&gt;()</code> — O(1)</summary>

空配列の版0を構築します。

{% raw %}
```cpp
blueberry::PersistentArray<int> a;
```
{% endraw %}

注意点: size()==0、versions()==1。

</details>

<details class="api-operation" id="fill" markdown="1">
<summary><code>PersistentArray&lt;T&gt;(int n, const T&amp; initial = T{})</code> — O(1)</summary>

長さ n の一様な初期配列を構築します。

{% raw %}
```cpp
blueberry::PersistentArray<int> a(5, -1);
```
{% endraw %}

注意点: 0<=n<=INT_MAX。初期値を共有する疎な木です。

</details>

<details class="api-operation" id="vector" markdown="1">
<summary><code>PersistentArray&lt;T&gt;(const vector&lt;T&gt;&amp; a)</code> — O(N)</summary>

配列をコピーして版0を構築します。

{% raw %}
```cpp
blueberry::PersistentArray<int> a(std::vector<int>{1,2});
```
{% endraw %}

注意点: 0<=N<=INT_MAX/2。空入力も有効です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

配列長を返します。

{% raw %}
```cpp
int n = object.size();
```
{% endraw %}

注意点: 版によって長さは変わりません。

</details>

<details class="api-operation" id="versions" markdown="1">
<summary><code>int object.versions() const</code> — O(1)</summary>

有効な版の数を返します。

{% raw %}
```cpp
int v = object.versions();
```
{% endraw %}

注意点: 有効な版番号は [0,v)。

</details>

<details class="api-operation" id="fork" markdown="1">
<summary><code>int object.fork(int version)</code> — 償却 O(1)</summary>

同じ内容の新しい版を作り、その番号を返します。

{% raw %}
```cpp
int v = object.fork(0);
```
{% endraw %}

注意点: 有効な version が必要です。空配列も可。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T object.get(int version, int p) const</code> — O(log(N+1))</summary>

版 version の p 番目を値で返します。

{% raw %}
```cpp
auto x = object.get(0, 0);
```
{% endraw %}

注意点: 0<=p<N。有効な版番号が必要です。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>int object.set(int version, int p, const T&amp; value)</code> — 償却 O(log(N+1))</summary>

一点代入した新しい版番号を返します。

{% raw %}
```cpp
int v = object.set(0, 0, 7);
```
{% endraw %}

注意点: 元の版は不変。0<=p<N。同値代入でも新しい版を作ります。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>Type(const Type&amp; other)</code> — O(M)</summary>

全履歴・補助索引を独立にコピーします。

{% raw %}
```cpp
Type copied(object);
```
{% endraw %}

注意点: 以後の更新は互いに影響しません。M は保持する全要素・ノード・版数です。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>Type(Type&amp;&amp; other)</code> — O(1)</summary>

内部領域の所有権を移します。

{% raw %}
```cpp
Type moved(std::move(object));
```
{% endraw %}

注意点: <utility> が必要です。ムーブ元は破棄または再代入してから使用してください。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(const Type&amp; other)</code> — O(M + D)</summary>

コピーで置き換え、*this を返します。

{% raw %}
```cpp
Type copied; copied = object;
```
{% endraw %}

注意点: D は代入前の宛先保持量。旧領域の破棄も含みます。自己代入は有効です。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>Type&amp; object.operator=(Type&amp;&amp; other)</code> — O(1 + D)</summary>

旧領域を破棄し所有権を移し、*this を返します。

{% raw %}
```cpp
Type moved; moved = std::move(object);
```
{% endraw %}

注意点: D は旧宛先保持量。<utility> が必要です。自己ムーブを含めムーブ元は破棄または再代入してください。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Luzhiled の永続配列](https://ei1333.github.io/library/structure/others/persistent-array.hpp) と [Nyaan の永続UF](https://nyaannyaan.github.io/library/data-structure/persistent-union-find.hpp.html) を設計比較し、明示的版番号・所有arenaで独立実装。
- `verify/data-structure/persistent-array.test.cpp`: [Persistent Queue](https://judge.yosupo.jp/problem/persistent_queue) を版ごとの配列添字で実装。
- `tests/random/persistence-and-range-union.cpp`: 分岐、初期値、空列、fork、コピーを愚直な配列コピーと比較。

</details>
