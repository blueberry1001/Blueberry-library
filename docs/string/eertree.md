---
title: Eertree
documentation_of: //blueberry/string/eertree.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

文字を末尾へ追加しながら、異なる回文部分文字列を1ノードずつ管理する回文木です。
Manacherが各中心の長さを求めるのに対し、回文そのものの同一性・suffix link・出現数を扱います。

`Eertree<Alphabet = 26, First = 'a'>` の文字集合はunsigned byteの
`[First, First + Alphabet)`、`1 <= Alphabet`、`First + Alphabet <= 256` が必要。
既定は英小文字で、UTF-8を文字単位として扱いません。

ノードIDは長さ-1の特別rootが-1、空文字列が0、非空回文が発見順に1..Dです。
入力長Nに対しD<=N、`N <= INT_MAX-2`。σをAlphabet、Rを予約文字数とすると、
所有メモリはO(σ max(D,R)+max(N,R))、全N文字の構築時間はO(σN)です。
固定小文字種向けのdense遷移で、末尾削除・過去位置の更新は提供しません。
値コピーは独立し、時間・追加メモリは所有領域に比例します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/string/eertree.hpp"
int main() {
  blueberry::Eertree<> tree;
  int a = tree.add('a');
  tree.add('b');
  int aba = tree.add('a');
  assert(tree.size() == 3 && tree.length(aba) == 3);
  assert(tree.suffix() == aba && tree.link(aba) == a);
  auto counts = tree.count();
  assert(counts[a] == 2 && counts[aba] == 1 && counts[0] == 4);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `Eertree<σ,First> tree(int reserve_length = 0)` | O(σ)、予約領域O(σR) | [開く](#constructor) |
| `Eertree<σ,First> tree(string_view text)` | O(σN) | [開く](#from-string) |
| `int tree.size()` | O(1) | [開く](#size) |
| `int tree.add(char c)` | 償却O(σ)、最悪O(σN) | [開く](#add) |
| `int tree.suffix()` | O(1) | [開く](#suffix) |
| `int tree.length(int v)` | O(1) | [開く](#length) |
| `int tree.parent(int v)` | O(1) | [開く](#parent) |
| `int tree.link(int v)` | O(1) | [開く](#link) |
| `vector<long long> tree.count()` | O(D)、追加メモリO(D) | [開く](#count) |
| `Eertree(other); operator=(other)` | コピーO(σD+N)、ムーブO(1) ※代入先解放を除く | [開く](#copy-move) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>Eertree&lt;σ,First&gt; tree(int reserve_length = 0)</code> — O(σ)</summary>

2個のrootだけを初期化します。予約は0以上INT_MAX-2以下で、入力長が既知なら指定できます。

{% raw %}
```cpp
blueberry::Eertree<10, '0'> digits(1000);
assert(digits.size() == 0 && digits.suffix() == 0);
```
{% endraw %}

注意点: 予約数は上限ではありません。O(σR)の領域を確保しますが、R個のノードを初期化はしません。
</details>

<details class="api-operation" id="from-string" markdown="1">
<summary><code>Eertree&lt;σ,First&gt; tree(std::string_view text)</code> — O(σN)</summary>

textを左からaddして構築します。入力を借用せず、構築後も追加できます。

{% raw %}
```cpp
blueberry::Eertree<> tree("abba");
assert(tree.size() == 4 && tree.length(tree.suffix()) == 4);
tree.add('a');
```
{% endraw %}

注意点: 空文字列も使用できます。入力長分の容量を予約するため、回文の種類が少なくてもO(σN)の予約領域を使います。
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size()</code> — O(1)</summary>

非空回文の種類数Dを返します。rootを数えません。

{% raw %}
```cpp
blueberry::Eertree<> tree("aaa");
assert(tree.size() == 3); // a, aa, aaa
```
{% endraw %}

注意点: 出現数の合計ではありません。有効な非空回文IDは1..size()です。
</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>int tree.add(char c)</code> — 償却O(σ)、最悪O(σN)</summary>

末尾へcを追加し、新しい文字列の最長回文suffixのIDを返します。新しい回文は1文字につき高々1種類。
suffix linkを辿る回数は全追加で線形、ノード初期化・vector再確保を含めて償却O(σ)です。

{% raw %}
```cpp
blueberry::Eertree<> tree;
int a = tree.add('a');
tree.add('b');
int aba = tree.add('a');
assert(tree.link(aba) == a);
```
{% endraw %}

注意点: cは指定文字集合内。最悪1回では長いsuffix探索や再確保が起きます。IDは追加後も変わりません。
</details>

<details class="api-operation" id="suffix" markdown="1">
<summary><code>int tree.suffix()</code> — O(1)</summary>

現在の最長回文suffixのIDを返します。空文字列では0です。

{% raw %}
```cpp
blueberry::Eertree<> tree("abb");
assert(tree.length(tree.suffix()) == 2);
```
{% endraw %}

注意点: 最長の回文部分文字列全体ではなく、末尾に接する回文です。
</details>

<details class="api-operation" id="length" markdown="1">
<summary><code>int tree.length(int v)</code> — O(1)</summary>

ノードvの回文長を返します。

{% raw %}
```cpp
assert(tree.length(-1) == -1 && tree.length(0) == 0);
```
{% endraw %}

注意点: `-1 <= v <= size()`。特別root -1は実在する文字列ではありません。
</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int tree.parent(int v)</code> — O(1)</summary>

両端の1文字ずつを除いた回文のIDです。長さ1の親は-1、長さ2の親は0。

{% raw %}
```cpp
blueberry::Eertree<> tree("aba");
assert(tree.length(tree.parent(tree.suffix())) == 1);
```
{% endraw %}

注意点: `1 <= v <= size()`。rootには呼べません。suffix linkとは異なります。
</details>

<details class="api-operation" id="link" markdown="1">
<summary><code>int tree.link(int v)</code> — O(1)</summary>

最長の真の回文suffixを返します。非空suffixがなければ0です。

{% raw %}
```cpp
blueberry::Eertree<> tree("aaa");
assert(tree.length(tree.link(tree.suffix())) == 2);
assert(tree.link(0) == -1 && tree.link(-1) == -1);
```
{% endraw %}

注意点: `-1 <= v <= size()`。特別rootのlinkは-1なので、無条件のlink走査で無限ループにしないでください。
</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>std::vector&lt;long long&gt; tree.count()</code> — O(D)</summary>

ID vの回文が現在の全文に何回現れたかを返します。重なる出現を含み、返り値はsize()+1要素、
添字0の空文字列はN+1回です。-1のrootは含めません。

{% raw %}
```cpp
blueberry::Eertree<> tree("aaaa");
auto counts = tree.count();
assert(counts[1] == 4 && counts[4] == 1);
tree.add('a');
assert(tree.count()[1] == 5 && counts[1] == 4);
```
{% endraw %}

注意点: 木の内部を変更しないため何度でも集計できます。返り値は独立した値で、以後のaddを自動反映しません。
</details>

<details class="api-operation" id="copy-move" markdown="1">
<summary><code>Eertree(const Eertree&amp;); operator=(const Eertree&amp;); Eertree(Eertree&amp;&amp;); operator=(Eertree&amp;&amp;)</code> — コピーO(σD+N)、ムーブO(1)</summary>

コピーは回文ノード・本文・出現数を独立して保持します。ムーブは所有領域を引き継ぎます。

{% raw %}
```cpp
blueberry::Eertree<> original("ab");
auto copy = original;
copy.add('a');
assert(copy.size() == 3 && original.size() == 2);
```
{% endraw %}

注意点: ムーブ元には破棄・再代入のみ行ってください。ムーブ代入の代入先解放には既存領域分の処理が必要です。
コピー後も既存IDは同じ回文を表しますが、その後の追加で作られるIDは別々です。
</details>

## 出典・検証

- [Rubinchik–Shur, EERTREE](https://arxiv.org/abs/1506.04862)の回文木を独立実装。
- [Library Checker Eertree](https://judge.yosupo.jp/problem/eertree)で発見順、親、suffix linkと全prefixの最長回文を検証。
- Fastest調査: [394036](https://judge.yosupo.jp/submission/394036)は疎な子ノードpool、
  [400577](https://judge.yosupo.jp/submission/400577)は疎な辺と専用allocation/I/O。
  本実装は小さい固定文字種で単純な遷移配列を選択。文字種・メモリの交換条件を比較レポートに記録します。
- `tests/random/string-expansion.cpp` は小入力の全substring列挙と全ノードの出現数・親・linkを比較します。
