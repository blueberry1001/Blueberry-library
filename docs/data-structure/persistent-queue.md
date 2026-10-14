---
title: Persistent Queue
documentation_of: //blueberry/data-structure/persistent-queue.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

任意の過去版から分岐できる永続FIFOです。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`PersistentQueue<T>`](#op-0) | O(1) |
| [`q.versions`](#op-1) | O(1) |
| [`q.size`](#op-2) | O(1) |
| [`q.empty`](#op-3) | O(1) |
| [`q.push`](#op-4) | O(1) 償却 |
| [`q.pop`](#op-5) | O(1) 償却 |
| [`q.front`](#op-6) | O(log(N+1)) 最悪 |
| [`PersistentQueue`](#value-0) | O(Q) |
| [`PersistentQueue`](#value-1) | O(1) |
| [`operator=`](#value-2) | O(Q) + O(旧状態の破棄) |
| [`operator=`](#value-3) | O(1) + O(旧状態の破棄) |

<!-- library-implementation -->

## 概要・前提

任意の過去版から分岐できる永続FIFOです。ACLには永続queueがないため追加します。版0は空で、push/popは連番の新しい版を返します。Qを累積更新数、Nを参照版までのappend経路長とし、メモリはO(Q)。親と1本のskip linkで先頭を探索します。Tはコピー可能で、既定構築や大小比較は不要です。以下の計算量はTのコピーをO(1)とします。版数はINT_MAX以下。オブジェクトは全版を所有し、入力への参照を保持せず、戻り値もコピーです。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/persistent-queue.hpp"
int main() {
  blueberry::PersistentQueue<int> q;
  int a=q.push(0, 7), b=q.push(a, 9), c=q.pop(b);
  assert(q.front(a)==7 && q.front(c)==9);
  assert(q.empty(0) && q.size(b)==2 && q.versions()==4);
}
```
{% endraw %}

<details class="api-operation" id="op-0" markdown="1">
<summary><code>PersistentQueue&lt;T&gt;()</code> — O(1)</summary>

空の版0を作ります。

{% raw %}
```cpp
blueberry::PersistentQueue<int> q;
```
{% endraw %}

注意点: 各queueの版IDは独立しています。

</details>

<details class="api-operation" id="op-1" markdown="1">
<summary><code>int q.versions() const</code> — O(1)</summary>

版0を含む版数を返します。

{% raw %}
```cpp
int count=q.versions();
```
{% endraw %}

注意点: 有効なIDは0以上count未満。

</details>

<details class="api-operation" id="op-2" markdown="1">
<summary><code>int q.size(int v) const</code> — O(1)</summary>

版vの要素数を返します。

{% raw %}
```cpp
int n=q.size(0);
```
{% endraw %}

注意点: vは有効な版。空なら0。

</details>

<details class="api-operation" id="op-3" markdown="1">
<summary><code>bool q.empty(int v) const</code> — O(1)</summary>

版vが空ならtrue。

{% raw %}
```cpp
bool e=q.empty(0);
```
{% endraw %}

注意点: vは有効な版。

</details>

<details class="api-operation" id="op-4" markdown="1">
<summary><code>int q.push(int v, const T&amp; value)</code> — O(1) 償却</summary>

vの末尾にvalueを追加した新しい版IDを返します。

{% raw %}
```cpp
int a=q.push(0, 3);
```
{% endraw %}

注意点: 元の版は不変。vector再確保の単一操作は最悪O(Q)。Tのコピー費用は別途必要。

</details>

<details class="api-operation" id="op-5" markdown="1">
<summary><code>int q.pop(int v)</code> — O(1) 償却</summary>

vの先頭を除いた新しい版IDを返します。

{% raw %}
```cpp
int b=q.pop(a);
```
{% endraw %}

注意点: vは非空。有効な版は全て残ります。vector再確保で単一操作は最悪O(Q)。

</details>

<details class="api-operation" id="op-6" markdown="1">
<summary><code>T q.front(int v) const</code> — O(log(N+1)) 最悪</summary>

vの先頭要素をコピーして返します。

{% raw %}
```cpp
int x=q.front(a);
```
{% endraw %}

注意点: vは非空。コピーした値は後の更新でも無効化されません。

</details>

<details class="api-operation" id="value-0" markdown="1">
<summary><code>PersistentQueue(const PersistentQueue&amp; other)</code> — O(Q)</summary>

otherの全状態を独立にコピーします。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
blueberry::PersistentQueue<int> copy(original);
```
{% endraw %}

注意点: コピー先は独立した状態を所有し、コピー元の変更・破棄に影響されません。入力への参照は保持しません。

</details>

<details class="api-operation" id="value-1" markdown="1">
<summary><code>PersistentQueue(PersistentQueue&amp;&amp; other)</code> — O(1)</summary>

otherの記憶領域を移動します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
blueberry::PersistentQueue<int> moved(std::move(original));
```
{% endraw %}

注意点: 移動元は破棄または再代入のみ行ってください。移動先が状態を所有します。

</details>

<details class="api-operation" id="value-2" markdown="1">
<summary><code>PersistentQueue&amp; operator=(const PersistentQueue&amp; other)</code> — O(Q) + O(旧状態の破棄)</summary>

otherの全状態で置き換え、自分への参照を返します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
copy = original;
```
{% endraw %}

注意点: コピー元は不変で、代入後も両者は独立しています。代入前のコピー先の版IDは、元の版を表すものとしては使えません。

</details>

<details class="api-operation" id="value-3" markdown="1">
<summary><code>PersistentQueue&amp; operator=(PersistentQueue&amp;&amp; other)</code> — O(1) + O(旧状態の破棄)</summary>

otherの記憶領域を移して置き換え、自分への参照を返します。

{% raw %}
```cpp
// originalは構築済み。std::moveには<utility>が必要。
copy = std::move(original);
```
{% endraw %}

注意点: 移動元は破棄または再代入のみ行ってください。代入前の移動先の版IDは、元の版を表すものとしては使えません。

</details>

## 出典・検証

[Library Checker: Persistent Queue](https://judge.yosupo.jp/problem/persistent_queue) と分岐・長い経路のランダム比較で検証します。[Nyaanのdoubling設計](https://nyaannyaan.github.io/library/data-structure/persistent-queue.hpp.html) とskip-link方式を比較し、O(Q)空間を採用。コードは独立実装。測定条件・結果は [batch-three DS report](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/batch-three-ds.md)。
