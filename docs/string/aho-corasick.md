---
title: Aho–Corasick
documentation_of: //blueberry/string/aho-corasick.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

複数パターンをTrieへ登録し、本文中の重なる出現もまとめて数えます。ACLのZ Algorithmや
suffix arrayとは異なる、固定辞書の多パターン検索です。

`AhoCorasick<Alphabet = 26, First = 'a'>` の文字集合は、unsigned byteとして
`[First, First + Alphabet)`。`1 <= Alphabet`、`First + Alphabet <= 256` が必要です。
既定は英小文字。本文もこの範囲内に限ります。UTF-8の文字単位処理ではありません。
`add`をすべて済ませてから`build`し、その後は登録できません。

σをAlphabet、Vをrootを含むノード数、Rを予約ノード数、Lを登録文字列長、Mを本文長とします。
空TrieはV=1、空文字列のIDは0。IDは初めてprefixが現れた順で、追加・buildで変わりません。
`V <= INT_MAX`、`M < LLONG_MAX` が必要です。所有メモリはO(σ max(V,R))、
build用にO(V)。denseな遷移表で検索を1文字O(1)にするため、文字種が大きい用途には不向きです。
通常のコピーは独立した値コピー（時間・追加メモリO(σ max(V,R))以内）です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/string/aho-corasick.hpp"
int main() {
  blueberry::AhoCorasick<> ac;
  int he = ac.add("he"), she = ac.add("she"), empty = ac.add("");
  assert(ac.add("he") == he);
  ac.build();
  auto counts = ac.count("shehe");
  assert(counts[he] == 2 && counts[she] == 1 && counts[empty] == 6);
  int state = 0;
  for (char c : {'s', 'h', 'e'}) state = ac.next(state, c);
  assert(state == she && ac.link(she) == he);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `AhoCorasick<σ,First> ac(int reserve_nodes = 0)` | O(σ)、予約領域O(σR) | [開く](#constructor) |
| `int ac.size()` | O(1) | [開く](#size) |
| `int ac.add(string_view pattern)` | 償却O(σL) | [開く](#add) |
| `void ac.build()` | 初回O(σV)、以後O(1) | [開く](#build) |
| `int ac.parent(int v)` | O(1) | [開く](#parent) |
| `int ac.link(int v)` | O(1) | [開く](#link) |
| `int ac.next(int v, char c)` | O(1) | [開く](#next) |
| `vector<long long> ac.count(string_view text)` | O(M+V)、追加メモリO(V) | [開く](#count) |
| `AhoCorasick(other); operator=(other)` | コピーO(σV)、ムーブO(1) ※代入先解放を除く | [開く](#copy-move) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>AhoCorasick&lt;σ,First&gt; ac(int reserve_nodes = 0)</code> — O(σ)</summary>

rootのみを持つ未buildの辞書を作ります。Rは0以上の予約容量で、入力総長+1を指定すると
追加中の再確保を避けられます。予約だけではR個のノードを初期化しません。

{% raw %}
```cpp
blueberry::AhoCorasick<10, '0'> digits(1000);
int id = digits.add("123");
```
{% endraw %}

注意点: Rは上限ではなく予約の目安です。大きな予約はO(σR)のメモリを必要とします。
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int ac.size()</code> — O(1)</summary>

rootを含むノード数を返します。パターン数ではありません。

{% raw %}
```cpp
blueberry::AhoCorasick<> ac;
assert(ac.size() == 1);
ac.add("ab");
assert(ac.size() == 3);
```
{% endraw %}

注意点: 重複パターンや既存prefixの登録ではノードが増えません。
</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>int ac.add(std::string_view pattern)</code> — 償却O(σL)</summary>

文字列の終端ノードIDを返します。すべてのprefixも登録されます。既存辺だけを辿る場合はO(L)。
新ノードのσ遷移初期化とvector再確保を含めて償却O(σL)、1回の最悪はO(σ(V+L))です。

{% raw %}
```cpp
int a = ac.add("aba");
assert(ac.add("aba") == a);
assert(ac.add("") == 0);
```
{% endraw %}

注意点: build後は禁止。文字列は借用せず内容をTrieに反映するため、呼び出し後に入力を破棄できます。
同じ文字列を複数登録しても出現数は倍になりません。
</details>

<details class="api-operation" id="build" markdown="1">
<summary><code>void ac.build()</code> — O(σV)</summary>

幅優先順でsuffix linkと全遷移を確定します。空辞書でも呼べます。

{% raw %}
```cpp
ac.build();
ac.build(); // 2回目以降は何もしない
```
{% endraw %}

注意点: 一度buildした辞書にはaddできません。別の辞書が必要なら新しいオブジェクトを作ります。
</details>

<details class="api-operation" id="parent" markdown="1">
<summary><code>int ac.parent(int v)</code> — O(1)</summary>

Trieで最後の1文字を除いたノードを返します。rootの親は-1です。build前にも使えます。

{% raw %}
```cpp
blueberry::AhoCorasick<> ac;
int a = ac.add("a");
assert(ac.parent(a) == 0 && ac.parent(0) == -1);
```
{% endraw %}

注意点: `0 <= v < size()` が必要。suffix linkとは異なります。
</details>

<details class="api-operation" id="link" markdown="1">
<summary><code>int ac.link(int v)</code> — O(1)</summary>

vが表す文字列の最長の真のsuffixで、Trieに存在するもののIDです。該当する非空文字列がなければ0。

{% raw %}
```cpp
blueberry::AhoCorasick<> ac;
int ab = ac.add("ab"), b = ac.add("b");
ac.build();
assert(ac.link(ab) == b && ac.link(0) == 0);
```
{% endraw %}

注意点: build済み、`0 <= v < size()` が必要です。
</details>

<details class="api-operation" id="next" markdown="1">
<summary><code>int ac.next(int v, char c)</code> — O(1)</summary>

状態vへcを追加した後の最長Trie suffixの状態を返します。本文を逐次処理する入口です。

{% raw %}
```cpp
int state = 0;
for (char c : std::string_view("abc")) state = ac.next(state, c);
```
{% endraw %}

注意点: build済みかつ文字cが指定文字集合内であること。Trieに直接の辺がなくてもsuffix linkを
考慮した状態へ進むため、戻り値だけからパターン全件の一致は列挙できません。
</details>

<details class="api-operation" id="count" markdown="1">
<summary><code>std::vector&lt;long long&gt; ac.count(std::string_view text)</code> — O(M+V)</summary>

返り値のv番目は、そのTrieノードが表す文字列の重複を含む出現数です。
addの戻り値を添字に使います。rootは空文字列としてM+1回です。

{% raw %}
```cpp
blueberry::AhoCorasick<> ac;
int aa = ac.add("aa");
ac.build();
assert(ac.count("aaaa")[aa] == 3);
assert(ac.count("")[0] == 1);
```
{% endraw %}

注意点: build済みであること。辞書は変更しません。返り値は独立したvectorで、
別の本文の集計や元の辞書の破棄でも無効になりません。
</details>

<details class="api-operation" id="copy-move" markdown="1">
<summary><code>AhoCorasick(const AhoCorasick&amp;); operator=(const AhoCorasick&amp;); AhoCorasick(AhoCorasick&amp;&amp;); operator=(AhoCorasick&amp;&amp;)</code> — コピーO(σV)、ムーブO(1)</summary>

コピーはノードとbuild状態を複製します。ムーブは領域を引き継ぎ、代入では元の代入先領域を解放します。

{% raw %}
```cpp
auto copy = ac;
assert(copy.size() == ac.size());
```
{% endraw %}

注意点: コピーのIDはコピー時点で同じ文字列を表し、各オブジェクトは独立します。
ムーブ元は破棄・再代入のみを行ってください。代入先の既存IDは新しい辞書に対して解釈されます。
</details>

## 出典・検証

- [Aho–Corasick原論文](https://doi.org/10.1145/360825.360855)のfailure automaton。
- [Library Checker Aho Corasick](https://judge.yosupo.jp/problem/aho_corasick)でノード順・parent・linkを検証。
- Fastest調査: [307744](https://judge.yosupo.jp/submission/307744)は疎な辺とfailure探索、
  [380150](https://judge.yosupo.jp/submission/380150)はdense表とまとめた遷移コピー。
  本実装は検索も1文字O(1)にする固定配列方式を独立実装。提出ソースは転載していません。
- `tests/random/string-expansion.cpp` は全ノードのsuffix、遷移、空・重複パターン、重なる出現を愚直解と比較します。
