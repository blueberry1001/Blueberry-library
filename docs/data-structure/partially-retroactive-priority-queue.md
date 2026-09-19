---
title: Partially Retroactive Priority Queue
documentation_of: //blueberry/data-structure/partially-retroactive-priority-queue.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`PartiallyRetroactivePriorityQueue<T>` は過去の insert/delete-min 操作を追加・削除し、その変更を反映した**現在**の最小値を取得します。
完全永続とは異なり履歴を分岐しません。過去時点の問い合わせを提供する fully retroactive 版ではありません。
構築時に M 個の整数時刻 [0,M) を用意し、各時刻には最大1操作。座標圧縮すれば任意の既知時刻を使用できます。
構築・メモリ O(M)、各履歴変更 O(log(M+1))、現在の min/size O(1)。入力時刻数と操作数を混同しないでください。
元論文の bridge を ACL lazy_segtree の prefix 最小値で管理し、生存要素・消費済み要素の極値を ACL segtree に保持します。
T はコピー・代入可能で `<` が strict weak ordering。等価な優先度は挿入時刻の小さい順に削除します。値の算術は行わず、比較・コピーを O(1) とします。
0<=M<INT_MAX/4。各 prefix で insert 数が delete-min 数以上である履歴だけを受理します。不正な編集は false を返し、元の論理状態を保ちます。
返り値はコピーで参照の無効化はありません。例外発生時の強い保証は提供しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/partially-retroactive-priority-queue.hpp"
int main(){
  blueberry::PartiallyRetroactivePriorityQueue<int> q(8);
  assert(q.insert_op(1,5));
  assert(q.insert_op(4,2));
  assert(q.pop_op(3));
  assert(q.min()==2);
  assert(q.erase_op(3));
  assert(q.size()==2);
  assert(!q.pop_op(0));
  assert(q.min()==2);
}
```
{% endraw %}

## 操作一覧

以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `PartiallyRetroactivePriorityQueue<T>(int slots=0)` | O(M) | [開く](#constructor) |
| `int object.time_slots() const` | O(1) | [開く](#time-slots) |
| `int object.size() const` | O(1) | [開く](#size) |
| `bool object.empty() const` | O(1) | [開く](#empty) |
| `bool object.has_op(int t) const` | O(1) | [開く](#has-op) |
| `optional<T> object.min() const` | O(1) | [開く](#min) |
| `bool object.insert_op(int t, const T& value)` | O(log(M+1)) | [開く](#insert-op) |
| `bool object.pop_op(int t)` | O(log(M+1)) | [開く](#pop-op) |
| `bool object.erase_op(int t)` | O(log(M+1)) | [開く](#erase-op) |
| `Type(const Type& other)` | O(M) | [開く](#copy-constructor) |
| `Type(Type&& other)` | O(1) | [開く](#move-constructor) |
| `Type& object.operator=(const Type& other)` | O(M + D) | [開く](#copy-assignment) |
| `Type& object.operator=(Type&& other)` | O(1 + D) | [開く](#move-assignment) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>PartiallyRetroactivePriorityQueue&lt;T&gt;(int slots=0)</code> — O(M)</summary>

空の操作履歴を構築します。

{% raw %}
```cpp
blueberry::PartiallyRetroactivePriorityQueue<int> q(10);
```
{% endraw %}

注意点: slots が M。0 なら時刻への更新は不可。

</details>

<details class="api-operation" id="time-slots" markdown="1">
<summary><code>int object.time_slots() const</code> — O(1)</summary>

利用できる時刻数を返します。

{% raw %}
```cpp
int m=object.time_slots();
```
{% endraw %}

注意点: 構築後の変更はありません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

現在残っている要素数を返します。

{% raw %}
```cpp
int n=object.size();
```
{% endraw %}

注意点: 登録操作数ではありません。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool object.empty() const</code> — O(1)</summary>

現在の queue が空か返します。

{% raw %}
```cpp
bool empty=object.empty();
```
{% endraw %}

注意点: 履歴が非空でも queue は空になれます。

</details>

<details class="api-operation" id="has-op" markdown="1">
<summary><code>bool object.has_op(int t) const</code> — O(1)</summary>

時刻 t に操作があるか返します。

{% raw %}
```cpp
bool occupied=object.has_op(0);
```
{% endraw %}

注意点: 0<=t<M。insert/pop のいずれでも true。

</details>

<details class="api-operation" id="min" markdown="1">
<summary><code>optional&lt;T&gt; object.min() const</code> — O(1)</summary>

現在の最小値をコピーして返します。

{% raw %}
```cpp
auto value=object.min();
```
{% endraw %}

注意点: 空なら nullopt。取り除きません。

</details>

<details class="api-operation" id="insert-op" markdown="1">
<summary><code>bool object.insert_op(int t, const T&amp; value)</code> — O(log(M+1))</summary>

過去時刻 t に insert(value) を追加します。

{% raw %}
```cpp
bool ok=object.insert_op(0,7);
```
{% endraw %}

注意点: 0<=t<M。埋まっていれば false で不変。重複値は可能。

</details>

<details class="api-operation" id="pop-op" markdown="1">
<summary><code>bool object.pop_op(int t)</code> — O(log(M+1))</summary>

過去時刻 t に delete-min を追加します。

{% raw %}
```cpp
bool ok=object.pop_op(1);
```
{% endraw %}

注意点: 0<=t<M。埋まった時刻または途中に空 queue からの削除が発生する履歴なら false で不変。

</details>

<details class="api-operation" id="erase-op" markdown="1">
<summary><code>bool object.erase_op(int t)</code> — O(log(M+1))</summary>

時刻 t の操作を削除します。

{% raw %}
```cpp
bool ok=object.erase_op(1);
```
{% endraw %}

注意点: 0<=t<M。操作なし、または後続 delete-min が不正になる場合 false で不変。

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

## 出典・検証

- [Demaine・Iacono・Langerman: Retroactive Data Structures, §5.4](https://people.csail.mit.edu/edemaine/papers/Retroactive_TALG/paper.pdf) の bridge 補題に基づく独立実装。
- `tests/random/partially-retroactive-priority-queue.cpp`: 毎回全履歴を multiset で再生し、任意位置の追加・削除、同値、拒否された編集を比較。
- **公式問題による過去編集の検証は未対応**です。通常の優先度キュー問題だけでは過去編集を検証できないため、上記の全履歴再生とのランダム比較で検証します。
