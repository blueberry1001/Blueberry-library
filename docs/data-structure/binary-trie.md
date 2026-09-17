---
title: Binary Trie
documentation_of: //blueberry/data-structure/binary-trie.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

固定bit長の符号なし整数の多重集合です。最小XOR値、重複込みの順位とk番目を扱います。ACLには対応する構造がありません。`BinaryTrie<UInt = uint32_t, Bits = numeric_limits<UInt>::digits>` を使います。UIntはbool以外の符号なし整数型、1<=Bits<=UIntの値bit数です。格納値と問合せ値は `[0,2^Bits)`。Bitsが型の全幅と等しくても、幅と同じbit数のshiftは行いません。

Nは重複を含む要素数、B=Bits、Dは異なる値の数、Hはclear以降のpool slot数の最大値です。N<=INT_MAXかつノード数<=INT_MAXが必要です。ノードには子2個と個数をintで持ちます。メモリO(H)=O(1+B·Dmax)で、Dmaxは過去最大の異なる値の数。削除された経路を再利用し、clearは容量も解放します。重複挿入は新しい経路を作りません。

count/rank/kth/xor_minは最悪O(B)。insert/eraseはvectorの増設を含め償却O(B)、単発最悪O(H+B)です。free-listの拡張によりeraseでも再確保が起こり得ます。size/emptyはO(1)。eraseは同じ値を1個だけ削除し、存在しなければfalseです。xor_minは格納要素ではなく **min(x xor a[i])の値** を返します。

所有権はvector内で完結し、コピーは独立、移動元は空で再利用可能です。返り値は所有値で内部参照を公開しません。再帰はなく、eraseの一時経路はO(B)。allocation例外後の強い例外保証は提供しません。範囲違反はassertの前提条件です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include "blueberry/data-structure/binary-trie.hpp"
int main() {
  blueberry::BinaryTrie<std::uint64_t> tree;
  tree.insert(2); tree.insert(7); tree.insert(7);
  assert(tree.count(7) == 2);
  assert(tree.rank(7) == 1);
  assert(tree.kth(2).value() == 7);
  assert(tree.xor_min(6).value() == 1); // 6 xor 7
  assert(tree.erase(7) && tree.count(7) == 1);
  assert(!tree.erase(9));
  tree.clear();
  assert(!tree.kth(0) && !tree.xor_min(0));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `BinaryTrie<UInt, Bits>()` | O(1) | [開く](#construct) |
| `BinaryTrie(const BinaryTrie& other)` | O(H) | [開く](#copy-construct) |
| `BinaryTrie& operator=(const BinaryTrie& other)` | O(H + Hdst) | [開く](#copy-assign) |
| `BinaryTrie(BinaryTrie&& other)` | O(1) | [開く](#move-construct) |
| `BinaryTrie& operator=(BinaryTrie&& other)` | O(Hdst) | [開く](#move-assign) |
| `~BinaryTrie()` | O(H) | [開く](#destruct) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `bool tree.empty() const` | O(1) | [開く](#empty) |
| `void tree.insert(UInt x)` | 償却 O(B) | [開く](#insert) |
| `bool tree.erase(UInt x)` | 償却 O(B) | [開く](#erase) |
| `int tree.count(UInt x) const` | O(B) | [開く](#count) |
| `int tree.rank(UInt x) const` | O(B) | [開く](#rank) |
| `optional<UInt> tree.kth(int k) const` | O(B) | [開く](#kth) |
| `optional<UInt> tree.xor_min(UInt x) const` | O(B) | [開く](#xor-min) |
| `void tree.clear()` | O(H) | [開く](#clear) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>BinaryTrie&lt;UInt, Bits&gt;()</code> — O(1)</summary>

空の多重集合を作ります。ノード領域は最初の挿入時に確保します。

{% raw %}
```cpp
blueberry::BinaryTrie<unsigned, 30> tree;
```
{% endraw %}

注意点: テンプレート引数と値の範囲は概要を参照。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>BinaryTrie(const BinaryTrie&amp; other)</code> — O(H)</summary>

格納値・集約を独立に複製します。以降の更新は相互に影響しません。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: 各ノードの保持情報もコピーします。コピー元は変更しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>BinaryTrie&amp; operator=(const BinaryTrie&amp; other)</code> — O(H + Hdst)</summary>

既存内容を破棄して独立コピーに置き換え、自身への参照を返します。

{% raw %}
```cpp
auto copy = tree;
copy = tree;
```
{% endraw %}

注意点: 自己代入も可能。Hdstは代入前の宛先のpool slot数です。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>BinaryTrie(BinaryTrie&amp;&amp; other)</code> — O(1)</summary>

格納領域の所有権を移動します。移動元は空となり再利用できます。

{% raw %}
```cpp
auto moved = std::move(tree);
```
{% endraw %}

注意点: 内部参照は公開しません。std::moveには<utility>をincludeします。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>BinaryTrie&amp; operator=(BinaryTrie&amp;&amp; other)</code> — O(Hdst)</summary>

宛先の内容を破棄して所有権を移動し、自身への参照を返します。移動元は空です。

{% raw %}
```cpp
auto moved = tree;
moved = std::move(tree);
```
{% endraw %}

注意点: 自己move代入は何もしません。Hdstは代入前の宛先のpool slot数です。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~BinaryTrie()</code> — O(H)</summary>

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

重複を含む要素数を返します。

{% raw %}
```cpp
assert(tree.size() >= 0);
```
{% endraw %}

注意点: int上限を超える挿入は禁止。

</details>

<details class="api-operation" id="empty" markdown="1">
<summary><code>bool tree.empty() const</code> — O(1)</summary>

要素数が0ならtrueです。

{% raw %}
```cpp
if (tree.empty()) assert(tree.size() == 0);
```
{% endraw %}

注意点: 移動元も空になります。

</details>

<details class="api-operation" id="insert" markdown="1">
<summary><code>void tree.insert(UInt x)</code> — 償却 O(B)</summary>

xを1個挿入します。同じ値も重複して数えます。

{% raw %}
```cpp
tree.insert(7); tree.insert(7);
```
{% endraw %}

注意点: Nとノード数はINT_MAX以下。単発最悪O(H+B)。

</details>

<details class="api-operation" id="erase" markdown="1">
<summary><code>bool tree.erase(UInt x)</code> — 償却 O(B)</summary>

xを1個削除できればtrue、存在しなければfalseです。

{% raw %}
```cpp
bool removed = tree.erase(7);
(void)removed;
```
{% endraw %}

注意点: 最後の1個が消えると不要な経路を再利用候補に戻します。単発最悪O(H+B)。

</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>int tree.count(UInt x) const</code> — O(B)</summary>

xと等しい要素の個数を返します。

{% raw %}
```cpp
auto copies = tree.count(7);
(void)copies;
```
{% endraw %}

注意点: 空集合なら0。

</details>

<details class="api-operation" id="rank" markdown="1">
<summary><code>int tree.rank(UInt x) const</code> — O(B)</summary>

xより小さい値の個数を重複込みで返します。

{% raw %}
```cpp
assert(tree.rank(0) == 0);
```
{% endraw %}

注意点: x自体の重複数は含みません。

</details>

<details class="api-operation" id="kth" markdown="1">
<summary><code>optional&lt;UInt&gt; tree.kth(int k) const</code> — O(B)</summary>

昇順で0-indexのk番目を返します。

{% raw %}
```cpp
assert(!tree.kth(-1));
```
{% endraw %}

注意点: k<0またはk>=Nならnullopt。重複もそれぞれ1要素と数えます。

</details>

<details class="api-operation" id="xor-min" markdown="1">
<summary><code>optional&lt;UInt&gt; tree.xor_min(UInt x) const</code> — O(B)</summary>

xとのXORが最小となるXOR値を返します。

{% raw %}
```cpp
if (!tree.empty()) { auto distance = tree.xor_min(0).value(); (void)distance; }
```
{% endraw %}

注意点: 空ならnullopt。相手の要素が必要ならx xor 返り値で復元できます。

</details>

<details class="api-operation" id="clear" markdown="1">
<summary><code>void tree.clear()</code> — O(H)</summary>

全要素と保持容量を解放し、空にします。

{% raw %}
```cpp
tree.clear(); assert(tree.empty());
```
{% endraw %}

注意点: 以前の最大ノード数Hに依存します。その後も挿入できます。

</details>

## 出典・検証

[公式Set Xor Min](https://judge.yosupo.jp/problem/set_xor_min)ではcountで重複挿入を抑えて集合として使います。重複・rank/kth・64 bitの最上位・1 bit型・削除slot再利用は独立sorted-vector oracleで検証します。[公開上位提出403013](https://judge.yosupo.jp/submission/403013)のbitmap階層を用いた集合と比較し、こちらは固定bit経路で多重集合と順位を扱う設計です。公開ソースは転記していません。
