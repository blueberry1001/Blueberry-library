---
title: Persistent Union Find
documentation_of: //blueberry/data-structure/persistent-union-find.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

任意の過去版からオンラインで分岐できるUnion Findです。版0はN個の孤立点で、mergeは同一成分でも新しい版を返します。オフラインの版木DFSにはRollbackUnionFindも使えます。

Nは頂点数、Uはmerge回数。leader/same/comp_size/mergeはO(log²(N+1))、mergeの確保時間は償却評価です。メモリはO(1+U log(N+1))です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`PersistentUnionFind`](#constructor) | O(1) |
| [`object.size`](#size) | O(1) |
| [`object.versions`](#versions) | O(1) |
| [`object.leader`](#leader) | O(log²(N+1)) |
| [`object.same`](#same) | O(log²(N+1)) |
| [`object.comp_size`](#comp-size) | O(log²(N+1)) |
| [`object.components`](#components) | O(1) |
| [`object.merge`](#merge) | 償却 O(log²(N+1)) |
| [`Type`](#copy-constructor) | O(M) |
| [`Type`](#move-constructor) | O(1) |
| [`operator=`](#copy-assignment) | O(M + D) |
| [`operator=`](#move-assignment) | O(1 + D) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACL DSUは非永続のため対象が異なります。union by sizeで親鎖がO(log(N+1))、永続配列の1読出しがO(log(N+1))となり、経路圧縮はしません。
- mergeのノード・版vector再確保により、単一更新が保持量に比例する場合があります。
- `0 <= N <= INT_MAX`、頂点は0-indexedです。版・内部ノード数はINT_MAX未満で、版番号は同じオブジェクト内でのみ有効です。再確保では無効化されません。公開フィールド・内部参照はありません。

## 操作の補足 {#操作の補足}

- 以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/persistent-union-find.hpp"
int main() {
  blueberry::PersistentUnionFind uf(4);
  int a = uf.merge(0, 0, 1), b = uf.merge(a, 1, 2), c = uf.merge(0, 2, 3);
  assert(!uf.same(0, 0, 1) && uf.same(b, 0, 2));
  assert(uf.comp_size(b, 0) == 3 && uf.components(c) == 3);
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>PersistentUnionFind(int n = 0)</code> — O(1)</summary>

N個の孤立点の版0を構築します。

{% raw %}
```cpp
blueberry::PersistentUnionFind uf(5);
```
{% endraw %}

注意点: n>=0。疎な一様初期配列を使います。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

頂点数を返します。

{% raw %}
```cpp
int n = object.size();
```
{% endraw %}

注意点: 空なら0。

</details>

<details class="api-operation" id="versions" markdown="1">
<summary><code>int object.versions() const</code> — O(1)</summary>

版数を返します。

{% raw %}
```cpp
int n = object.versions();
```
{% endraw %}

注意点: 有効な版番号は[0,versions())。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int object.leader(int version, int p) const</code> — O(log²(N+1))</summary>

版内の成分代表を返します。

{% raw %}
```cpp
int r = object.leader(0, 0);
```
{% endraw %}

注意点: 0<=p<N。同じ成分なら同じ代表。代表の選び方には依存しないでください。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool object.same(int version, int a, int b) const</code> — O(log²(N+1))</summary>

2頂点の連結性を返します。

{% raw %}
```cpp
bool connected = object.same(0, 0, 1);
```
{% endraw %}

注意点: 0<=a,b<N。

</details>

<details class="api-operation" id="comp-size" markdown="1">
<summary><code>int object.comp_size(int version, int p) const</code> — O(log²(N+1))</summary>

頂点pの成分サイズを返します。

{% raw %}
```cpp
int count = object.comp_size(0, 0);
```
{% endraw %}

注意点: 0<=p<N。値は1以上N以下。

</details>

<details class="api-operation" id="components" markdown="1">
<summary><code>int object.components(int version) const</code> — O(1)</summary>

その版の成分数を返します。

{% raw %}
```cpp
int count = object.components(0);
```
{% endraw %}

注意点: 空のUFなら0。有効な版が必要です。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>int object.merge(int version, int a, int b)</code> — 償却 O(log²(N+1))</summary>

2成分を併合した新しい版番号を返します。

{% raw %}
```cpp
int version = object.merge(0, 0, 1);
```
{% endraw %}

注意点: 0<=a,b<N。返り値は代表ではなく版番号。同一成分でも新しい版です。旧版は不変。

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

- [Luzhiled](https://ei1333.github.io/library/structure/union-find/persistent-union-find.hpp.html)、[Nyaan](https://nyaannyaan.github.io/library/data-structure/persistent-union-find.hpp.html) の永続配列方式を比較。コード転用なし。
- `verify/data-structure/online-persistent-unionfind.test.cpp`: [Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) をオンラインに処理。
- `tests/random/persistence-and-range-union.cpp`: 分岐、同一成分merge、連結性・成分サイズ・成分数を配列コピーと比較。

</details>
