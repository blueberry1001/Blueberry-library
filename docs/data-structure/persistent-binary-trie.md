---
title: Persistent Binary Trie
documentation_of: //blueberry/data-structure/persistent-binary-trie.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要

過去の任意の版から挿入・削除で分岐できる、符号なし整数の永続多重集合です。`PersistentBinaryTrie<UInt, Bits>` の値はBitsビット以内で、フル幅ではUIntの最大値も使えます。版0は空集合です。

B=Bits、Uは成功した挿入・削除回数。更新は償却O(B)、問合せは最悪O(B)、メモリはO(1+UB)です。通常のBinaryTrieと異なり、過去版への問合せや分岐ができます。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`PersistentBinaryTrie<UInt, Bits> t`](#constructor) | O(1) |
| [`t.size`](#size) | O(1) |
| [`t.empty`](#empty) | O(1) |
| [`t.insert`](#insert) | 償却 O(B) |
| [`t.erase`](#erase) | 償却 O(B) |
| [`t.count`](#count) | O(B) |
| [`t.rank`](#rank) | O(B) |
| [`t.kth`](#kth) | O(B) |
| [`t.xor_min`](#xor-min) | O(B) |

<details class="extra-operations" markdown="1">
<summary>その他の操作・型</summary>

| 操作 | 計算量 |
| --- | --- |
| [`PersistentBinaryTrie`](#copy-constructor) | O(M) |
| [`operator=`](#copy-assignment) | O(M + D) |
| [`PersistentBinaryTrie`](#move-constructor) | O(1) |
| [`operator=`](#move-assignment) | O(1 + D) |

</details>

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLに対応する機能はありません。既定のUIntはstd::uint32_t、Bitsはstd::numeric_limits<UInt>::digitsです。UIntはbool以外の符号なし整数で、`1 <= Bits <= UIntのビット数` が必要です。
- 木のパス上のノードだけをコピーし、更新ごとにB+1ノードを追加します。削除でも履歴のメモリは解放しません。vectorの再確保時には単一更新がO(UB)になり得ます。
- 各更新は元の版を変えず、成功した更新だけが新しい版を返します。versionは同じオブジェクトが返した版番号か、コピー・ムーブで受け継いだ版番号に限ります。
- 版番号は追加・再確保で無効化されず、破棄・代入による履歴の置換まで有効です。ムーブでは移動先へ引き継がれます。各版の要素数・全ノード数・版数はintの範囲内であることが前提です。
- 暗黙のコピーは全履歴を複製するためO(1+UB)です。ムーブ元は再代入または破棄してください。Mはコピー元・移動元の保持ノード数と版数の合計、Dは代入先の旧保持量の合計で、M=O(1+UB)です。
- 返り値は値で、参照の寿命・無効化の問題はありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/persistent-binary-trie.hpp"
int main() {
  blueberry::PersistentBinaryTrie<> t;
  int a = t.insert(0, 3), b = t.insert(a, 7);
  int c = t.insert(a, 3), d = t.erase(b, 3);
  assert(t.empty(0) && t.size(b) == 2);
  assert(t.count(c, 3) == 2 && t.count(a, 3) == 1);
  assert(t.kth(d, 0) == 7 && t.rank(b, 7) == 1);
  assert(t.xor_min(b, 6) == 1);
  assert(t.erase(d, 3) == d && !t.kth(0, 0));
}
```
{% endraw %}

<details class="api-operation" id="constructor" markdown="1">
<summary><code>PersistentBinaryTrie&lt;UInt, Bits&gt; t</code> — O(1)</summary>

空集合の版 0 のみを持つオブジェクトを作ります。

{% raw %}
```cpp
blueberry::PersistentBinaryTrie<> t; assert(t.empty(0));
```
{% endraw %}

注意点: UInt と Bits は概要の型・範囲を満たしてください。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>PersistentBinaryTrie t(const PersistentBinaryTrie&amp; other)</code> — O(M)</summary>

全履歴を独立に複製します。版番号はコピー元と同じ意味を持ち、以後の更新は互いに影響しません。
追加メモリは O(M) です。

{% raw %}
```cpp
blueberry::PersistentBinaryTrie<> original;
int a = original.insert(0, 3);
auto copied = original;
assert(copied.count(a, 3) == 1);
```
{% endraw %}

注意点: ノードを共有する安価なスナップショットではありません。履歴参照だけなら版番号を保存してください。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>PersistentBinaryTrie&amp; t.operator=(const PersistentBinaryTrie&amp; other)</code> — O(M + D)</summary>

代入先の全履歴をコピー元の履歴で置き換え、代入先への参照を返します。
旧データの破棄を含む上界を記載しています。追加容量は O(M)、コピー元は不変です。

{% raw %}
```cpp
blueberry::PersistentBinaryTrie<> original, copied;
int a = original.insert(0, 3);
copied = original;
assert(copied.count(a, 3) == 1);
```
{% endraw %}

注意点: 代入先の旧版番号は旧履歴を指さなくなります。同じ番号が存在しても意味が変わり得ます。
自己コピー代入は履歴を保持します。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>PersistentBinaryTrie t(PersistentBinaryTrie&amp;&amp; other)</code> — O(1)</summary>

全履歴の所有権を移動します。版番号は移動先で引き続き使用できます。
新たなノードコピー・要素ごとの移動は行いません。

{% raw %}
```cpp
#include <utility>
blueberry::PersistentBinaryTrie<> original;
int a = original.insert(0, 3);
auto moved = std::move(original);
assert(moved.count(a, 3) == 1);
```
{% endraw %}

注意点: ムーブ元では問い合わせ・更新を呼ばず、再代入または破棄してください。版 0 の空集合も保証しません。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>PersistentBinaryTrie&amp; t.operator=(PersistentBinaryTrie&amp;&amp; other)</code> — O(1 + D)</summary>

代入先の旧履歴を破棄し、移動元の全履歴を引き継ぎ、代入先への参照を返します。
計算量は旧データの破棄を含む上界です。所有権の受け渡し自体は O(1) です。

{% raw %}
```cpp
#include <utility>
blueberry::PersistentBinaryTrie<> original, moved;
int a = original.insert(0, 3);
moved = std::move(original);
assert(moved.count(a, 3) == 1);
```
{% endraw %}

注意点: 代入先の旧版番号は旧履歴を指さなくなります。移動元は再代入または破棄してください。
自己ムーブ代入後も履歴を利用できるとは限らないため、再代入または破棄してください。

</details>
<details class="api-operation" id="size" markdown="1">
<summary><code>int t.size(int version)</code> — O(1)</summary>

指定した版の要素数を、重複を含めて返します。

{% raw %}
```cpp
assert(t.size(0) == 0);
```
{% endraw %}

注意点: 空集合では 0。有効な版番号が必要です。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool t.empty(int version)</code> — O(1)</summary>

指定した版が空かを返します。

{% raw %}
```cpp
assert(t.empty(0));
```
{% endraw %}

注意点: 全要素を削除して作った版にも使えます。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>int t.insert(int version, UInt x)</code> — 償却 O(B)</summary>

x を 1 個追加した新しい版番号を返します。

{% raw %}
```cpp
int a = t.insert(0, 3); assert(t.count(a, 3) == 1);
```
{% endraw %}

注意点: 重複も追加します。元の版は不変。要素数 int 上限での追加は禁止です。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>int t.erase(int version, UInt x)</code> — 償却 O(B)</summary>

x を 1 個削除した新しい版番号を返します。

{% raw %}
```cpp
int a = t.insert(0, 3); int b = t.erase(a, 3); assert(t.empty(b));
```
{% endraw %}

注意点: x がなければ元の版番号を返し、メモリ確保しません。空集合でも呼べます。

</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>int t.count(int version, UInt x)</code> — O(B)</summary>

x の出現数を返します。

{% raw %}
```cpp
int a = t.insert(0, 3); assert(t.count(a, 3) == 1);
```
{% endraw %}

注意点: 存在しない値・空集合では 0。

</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int t.rank(int version, UInt x)</code> — O(B)</summary>

x 未満の要素数を重複込みで返します。

{% raw %}
```cpp
int a = t.insert(0, 3); assert(t.rank(a, 3) == 0);
```
{% endraw %}

注意点: 空集合では 0。x 自体は数えません。UInt 最大値を超えた上端を渡すことはできません。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>std::optional&lt;UInt&gt; t.kth(int version, int k)</code> — O(B)</summary>

昇順で 0-indexed の k 番目の値を返します。

{% raw %}
```cpp
int a = t.insert(0, 3); assert(t.kth(a, 0) == 3);
```
{% endraw %}

注意点: k < 0 または k >= size(version) では nullopt。重複も独立した要素として数えます。

</details>

<details class="api-operation" id="xor-min" markdown="1">
<summary><code>std::optional&lt;UInt&gt; t.xor_min(int version, UInt x)</code> — O(B)</summary>

集合の要素 y に対する x XOR y の最小値を返します。

{% raw %}
```cpp
int a = t.insert(0, 3); assert(t.xor_min(a, 6) == 5);
```
{% endraw %}

注意点: 返り値は y ではなく XOR 値です。空集合では nullopt。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Luzhiled の永続 Binary Trie](https://ei1333.github.io/luzhiled/snippets/structure/binary-trie.html) の
  パスコピー方式と [Nyaan の Binary Trie](https://nyaannyaan.github.io/library/data-structure/binary-trie.hpp.html)
  のポインタ木方式を調査しました。
- どちらもビット長に比例する操作です。
- 本実装はコードを転用せず、int 添字の連続プールと版配列で実装しています。
- ポインタ・要素 ID 配列を持たず、各ノードに子 2 個と個数だけを保持します。
- その代わりに版単位の部分解放は提供しません。
- 公式問題 [Set Xor-Min](https://judge.yosupo.jp/problem/set_xor_min) を
  `verify/data-structure/persistent-binary-trie.test.cpp` で検証します。
- `tests/random/persistent-binary-trie.cpp` は分岐履歴、重複、削除、空集合、1 / 30 / 64 ビット、
  UInt 最大値をソート済み配列と比較します。
- 速度の優劣は異なる条件の提出値から判断していません。
- 同条件で既存 BinaryTrie の全体コピーによる履歴保持と比較した測定を
  [測定ソース](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/persistent-binary-trie-benchmark.cpp) と
  [生ログ](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/persistent-binary-trie-benchmark.txt) に保存しています。
- GCC 13.3 / gnu++20 / -O2、20 ビット、固定 seed、20,000 回の分岐挿入と XOR 問い合わせ、5 回で、
  中央値は永続版 2.256 ms、全コピー 9.460 ms（最小 2.092 / 8.734 ms）でした。
- これは同一の全履歴を保持する用途の比較です。
- 通常の非永続操作に対する高速化を意味しません。

</details>
