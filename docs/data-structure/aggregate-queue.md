---
title: Aggregate Queue
documentation_of: //blueberry/data-structure/aggregate-queue.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

キューの全要素を先頭から末尾への順序で集約します。`AggregateQueue<S, op, e>` のopは結合的な `S op(S,S)`、eは両側単位元 `S e()` です。可換性・逆元は不要です。ACLの固定長segment treeより短い操作列で、端点の挿入削除と全体集約を扱います。

Nは現在の要素数、Hはclear以降の最大同時要素数。0<=N<=INT_MAX。Sはcopy/move/assignment可能な値型で、default constructorや等値比較は不要です。op/e/値コピーをO(1)とみなす計算量であり、string等では演算コストを掛けてください。最小例のstring連結は順序の説明用です。途中のprefix/suffix集約を含め、整数のopでoverflowを避けることは呼出側の責任です。

入力・出力の2本の集約スタックを使います。出力側が空になったとき、入力側を逆順に移します。各要素の移動は1回で、push/popは償却O(1)、単発最悪O(N)。prod/front/back/size/emptyは最悪O(1)。メモリはvectorの保持容量を含めO(H)。clearは保持容量も解放します。これはSの大きさを一定とみなす空間計算量です。Sがstring等の領域を所有する場合はその合計も必要で、文字列連結の全prefixを保持すると最悪O(N²)文字になります。先頭から削除しても残りの集約の順序は変わりません。

空でのprodはe()です。空でのpop/front/backは禁止でassert対象です。返り値は値コピーで、参照・iteratorは公開しません。値と領域は自身が所有し、コピーは独立、移動元は空で再利用可能です。利用者の演算・値のコピー・allocationが例外を投げた後の強い例外保証は提供しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/data-structure/aggregate-queue.hpp"
std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return {}; }
int main() {
  blueberry::AggregateQueue<std::string, op, e> tree;
  tree.push("a"); tree.push("b");
  assert(tree.prod() == "ab");
  assert(tree.front() == "a" && tree.back() == "b");
  tree.pop();
  assert(tree.prod() == "b");
  tree.clear(); assert(tree.empty() && tree.prod().empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `AggregateQueue<S, op, e>()` | O(1) | [開く](#construct) |
| `AggregateQueue(const AggregateQueue& other)` | O(N) | [開く](#copy-construct) |
| `AggregateQueue& operator=(const AggregateQueue& other)` | O(N + Ndst) | [開く](#copy-assign) |
| `AggregateQueue(AggregateQueue&& other)` | O(1) | [開く](#move-construct) |
| `AggregateQueue& operator=(AggregateQueue&& other)` | O(Ndst) | [開く](#move-assign) |
| `~AggregateQueue()` | O(N) | [開く](#destruct) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `bool tree.empty() const` | O(1) | [開く](#empty) |
| `void tree.push(const S& value)` | 償却 O(1) | [開く](#push) |
| `void tree.pop()` | 償却 O(1) | [開く](#pop) |
| `S tree.front() const` | O(1) | [開く](#front) |
| `S tree.back() const` | O(1) | [開く](#back) |
| `S tree.prod() const` | O(1) | [開く](#prod) |
| `void tree.clear()` | O(N) | [開く](#clear) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>AggregateQueue&lt;S, op, e&gt;()</code> — O(1)</summary>

空のキューを構築します。

{% raw %}
```cpp
using Tree = blueberry::AggregateQueue<std::string, op, e>;
Tree tree;
```
{% endraw %}

注意点: op/eは上の最小例と同じ関数を使っています。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>AggregateQueue(const AggregateQueue&amp; other)</code> — O(N)</summary>

格納値・集約を独立に複製します。以降の更新は相互に影響しません。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: Sを使う版では値のコピーも行います。コピー元は変更しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>AggregateQueue&amp; operator=(const AggregateQueue&amp; other)</code> — O(N + Ndst)</summary>

既存内容を破棄して独立コピーに置き換え、自身への参照を返します。

{% raw %}
```cpp
auto copy = tree;
copy = tree;
```
{% endraw %}

注意点: 自己代入も可能。Ndstは代入前の宛先の要素数です。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>AggregateQueue(AggregateQueue&amp;&amp; other)</code> — O(1)</summary>

格納領域の所有権を移動します。移動元は空となり再利用できます。

{% raw %}
```cpp
auto moved = std::move(tree);
```
{% endraw %}

注意点: 内部参照は公開しません。std::moveには<utility>をincludeします。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>AggregateQueue&amp; operator=(AggregateQueue&amp;&amp; other)</code> — O(Ndst)</summary>

宛先の内容を破棄して所有権を移動し、自身への参照を返します。移動元は空です。

{% raw %}
```cpp
auto moved = tree;
moved = std::move(tree);
```
{% endraw %}

注意点: 自己move代入は何もしません。Ndstは代入前の宛先の要素数です。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~AggregateQueue()</code> — O(N)</summary>

所有する値と領域をすべて破棄します。

{% raw %}
```cpp
{ auto copy = tree; } // スコープ末尾で破棄
```
{% endraw %}

注意点: 通常は明示呼出ししません。共有所有権や外部バッファへの依存はありません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

現在の要素数を返します。

{% raw %}
```cpp
assert(tree.size() >= 0);
```
{% endraw %}

注意点: N<=INT_MAX。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool tree.empty() const</code> — O(1)</summary>

要素がなければtrueを返します。

{% raw %}
```cpp
if (tree.empty()) assert(tree.size() == 0);
```
{% endraw %}

注意点: 移動元も空です。

</details>

<details class="api-operation" id="push" markdown="1">
<summary><code>void tree.push(const S&amp; value)</code> — 償却 O(1)</summary>

末尾へvalueを1個追加します。

{% raw %}
```cpp
tree.push("x");
```
{% endraw %}

注意点: N<INT_MAX。vector再確保時は最悪O(N)。

</details>

<details class="api-operation" id="pop" markdown="1">
<summary><code>void tree.pop()</code> — 償却 O(1)</summary>

先頭の要素を1個削除します。

{% raw %}
```cpp
if (!tree.empty()) tree.pop();
```
{% endraw %}

注意点: 空は禁止。スタック間の移動時は最悪O(N)。削除値が必要なら先にfrontを呼びます。

</details>

<details class="api-operation" id="front" markdown="1">
<summary><code>S tree.front() const</code> — O(1)</summary>

先頭の値をコピーして返します。

{% raw %}
```cpp
if (!tree.empty()) { auto value = tree.front(); (void)value; }
```
{% endraw %}

注意点: 空は禁止。内部参照ではありません。

</details>

<details class="api-operation" id="back" markdown="1">
<summary><code>S tree.back() const</code> — O(1)</summary>

末尾の値をコピーして返します。

{% raw %}
```cpp
if (!tree.empty()) { auto value = tree.back(); (void)value; }
```
{% endraw %}

注意点: 空は禁止。内部参照ではありません。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>S tree.prod() const</code> — O(1)</summary>

先頭から末尾へopで畳み込んだ値を返します。空ならe()です。

{% raw %}
```cpp
auto aggregate = tree.prod();
(void)aggregate;
```
{% endraw %}

注意点: opは可換でなくても構いません。関数合成では右側の関数を後から適用するopを定義してください。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void tree.clear()</code> — O(N)</summary>

全要素と保持容量を破棄します。

{% raw %}
```cpp
tree.clear(); assert(tree.empty());
```
{% endraw %}

注意点: Hに比例する容量を解放しますが、破棄するSの数は現在のNに比例します。

</details>

## 出典・検証

[公式合成問題](https://judge.yosupo.jp/problem/queue_operate_all_composite)、固定seedのstd::deque愚直合成、コピー・移動・再配置境界で検証します。[公開上位提出403012](https://judge.yosupo.jp/submission/403012)は事前容量指定と連続配置を用います。こちらは動的vectorと汎用monoidを用い、必要な2方向の集約を独立実装しています。公開ソースは転記していません。
