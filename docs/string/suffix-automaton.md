---
title: Suffix Automaton
documentation_of: //blueberry/string/suffix-automaton.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

`SuffixAutomaton<Symbol = char>` はオンライン末尾追加・部分文字列検索・出現回数・異なる部分文字列数・最長共通部分文字列を扱います。
ACLのsuffix_arrayは静的配列を作る別APIで、末尾追加の本機能を提供しません。既存LCS関数とは用途が異なります。
Nは元列長、Pはクエリ列長、σは異なる記号数。map遷移を使い、構築合計O(N log(σ+1))、メモリO(N)。
Symbolはコピー可能で比較が全順序を与え、比較・コピーはO(1)と仮定します。文字列とvectorなどのconst input_rangeを扱います。
空文字列のcontainsはtrue、countはN+1、distinct_substringsは非空だけを数えます。
countの集計は初回・append後の最初の存在する非空patternでO(N)再構築します。
appendとcountを交互に繰り返すと合計O(N²)になり得るため、出現数照会は追加をまとめた後が適します。
sizeはINT_MAX/2以下。状態参照を公開せず、入力の寿命に依存しません。countはconstでもキャッシュを更新します。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/string/suffix-automaton.hpp"
int main() {
  blueberry::SuffixAutomaton<> sam("ababa");
  assert(sam.count("aba") == 2 && sam.distinct_substrings() == 9);
  sam.append('b');
  assert(sam.count("ab") == 3 && sam.count("") == 7);
  auto [a,b,n] = sam.longest_common_substring("zzabab");
  assert(n == 4 && b == 2 && a >= 0);
}
```
{% endraw %}

## 操作一覧

以下 `Type` はこのページのクラス、`object` は有効な構築済みオブジェクトです。

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SuffixAutomaton<Symbol = char>()` | O(1) | [開く](#default) |
| `SuffixAutomaton<Symbol>(const Range& text)` | O(N log(σ+1)) | [開く](#range-constructor) |
| `SuffixAutomaton<Symbol>(basic_string_view<Symbol> text)` | O(N log(σ+1)) | [開く](#view-constructor) |
| `int object.size() const` | O(1) | [開く](#size) |
| `int object.states() const` | O(1) | [開く](#states) |
| `long long object.distinct_substrings() const` | O(1) | [開く](#distinct) |
| `void object.append(const Symbol& c)` | 償却 O(log(σ+1)) | [開く](#append) |
| `bool object.contains(const Range& pattern) const` | O(P log(σ+1)) | [開く](#contains-range) |
| `int object.count(const Range& pattern) const` | O(N + P log(σ+1)) | [開く](#count-range) |
| `tuple<int,int,int> object.longest_common_substring(const Range& pattern) const` | O(P log(σ+1)) | [開く](#longest_common_substring-range) |
| `bool object.contains(basic_string_view<Symbol> pattern) const` | O(P log(σ+1)) | [開く](#contains-view) |
| `int object.count(basic_string_view<Symbol> pattern) const` | O(N + P log(σ+1)) | [開く](#count-view) |
| `tuple<int,int,int> object.longest_common_substring(basic_string_view<Symbol> pattern) const` | O(P log(σ+1)) | [開く](#longest_common_substring-view) |
| `Type(const Type& other)` | O(M) | [開く](#copy-constructor) |
| `Type(Type&& other)` | O(1) | [開く](#move-constructor) |
| `Type& object.operator=(const Type& other)` | O(M + D) | [開く](#copy-assignment) |
| `Type& object.operator=(Type&& other)` | O(1 + D) | [開く](#move-assignment) |

<details class="api-operation" id="default" markdown="1">
<summary><code>SuffixAutomaton&lt;Symbol = char&gt;()</code> — O(1)</summary>

空文字列の状態を作ります。

{% raw %}
```cpp
blueberry::SuffixAutomaton<> sam;
```
{% endraw %}

注意点: 空の初期状態もstates()に数えます。

</details>

<details class="api-operation" id="range-constructor" markdown="1">
<summary><code>SuffixAutomaton&lt;Symbol&gt;(const Range&amp; text)</code> — O(N log(σ+1))</summary>

const input_rangeの文字列/整数列から構築します。

{% raw %}
```cpp
blueberry::SuffixAutomaton<int> sam(std::vector<int>{1,2,1});
```
{% endraw %}

注意点: C配列を除く入力rangeが対象です。文字リテラルは次のstring_view版を使います。

</details>

<details class="api-operation" id="view-constructor" markdown="1">
<summary><code>SuffixAutomaton&lt;Symbol&gt;(basic_string_view&lt;Symbol&gt; text)</code> — O(N log(σ+1))</summary>

文字列viewをコピーして構築します。

{% raw %}
```cpp
blueberry::SuffixAutomaton<> sam("ababa");
```
{% endraw %}

注意点: viewの寿命に依存しません。文字リテラルの終端NULは含めません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int object.size() const</code> — O(1)</summary>

元の列長を返します。

{% raw %}
```cpp
int n = object.size();
```
{% endraw %}

注意点: appendで1増えます。

</details>

<details class="api-operation" id="states" markdown="1">
<summary><code>int object.states() const</code> — O(1)</summary>

初期状態を含めた状態数を返します。

{% raw %}
```cpp
int n = object.states();
```
{% endraw %}

注意点: 列長と異なります。空なら1。

</details>

<details class="api-operation" id="distinct" markdown="1">
<summary><code>long long object.distinct_substrings() const</code> — O(1)</summary>

異なる非空部分文字列の数を返します。

{% raw %}
```cpp
auto n = object.distinct_substrings();
```
{% endraw %}

注意点: 空なら0。返り値は最大N(N+1)/2。

</details>

<details class="api-operation" id="append" markdown="1">
<summary><code>void object.append(const Symbol&amp; c)</code> — 償却 O(log(σ+1))</summary>

末尾に1記号追加します。

{% raw %}
```cpp
object.append('a');
```
{% endraw %}

注意点: N<INT_MAX/2。単独呼出しはO(N log(σ+1))になり得ます。出現数キャッシュは無効化されます。

</details>

<details class="api-operation" id="contains-range" markdown="1">
<summary><code>bool object.contains(const Range&amp; pattern) const</code> — O(P log(σ+1))</summary>

部分文字列として含まれるか返します。

{% raw %}
```cpp
bool yes = object.contains(std::string("aba"));
```
{% endraw %}

注意点: 空patternはtrue。

</details>

<details class="api-operation" id="count-range" markdown="1">
<summary><code>int object.count(const Range&amp; pattern) const</code> — O(N + P log(σ+1))</summary>

重なりを許した出現回数を返します。

{% raw %}
```cpp
int n = object.count(std::string("aba"));
```
{% endraw %}

注意点: 空patternはN+1。不在は0。初回・append後はO(N)の再集計があり、それ以外はO(P log(σ+1))。

</details>

<details class="api-operation" id="longest_common_substring-range" markdown="1">
<summary><code>tuple&lt;int,int,int&gt; object.longest_common_substring(const Range&amp; pattern) const</code> — O(P log(σ+1))</summary>

元列内の開始、pattern内の開始、最長共通部分文字列の長さを返します。

{% raw %}
```cpp
auto [a,b,n] = object.longest_common_substring(std::string("babc"));
```
{% endraw %}

注意点: 空/共通なしは(0,0,0)。同長の解の選び方には依存しないでください。P<=INT_MAX。

</details>

<details class="api-operation" id="contains-view" markdown="1">
<summary><code>bool object.contains(basic_string_view&lt;Symbol&gt; pattern) const</code> — O(P log(σ+1))</summary>

部分文字列として含まれるか返します。

{% raw %}
```cpp
bool yes = object.contains("aba");
```
{% endraw %}

注意点: 空patternはtrue。

</details>

<details class="api-operation" id="count-view" markdown="1">
<summary><code>int object.count(basic_string_view&lt;Symbol&gt; pattern) const</code> — O(N + P log(σ+1))</summary>

重なりを許した出現回数を返します。

{% raw %}
```cpp
int n = object.count("aba");
```
{% endraw %}

注意点: 空patternはN+1。不在は0。初回・append後はO(N)の再集計があり、それ以外はO(P log(σ+1))。

</details>

<details class="api-operation" id="longest_common_substring-view" markdown="1">
<summary><code>tuple&lt;int,int,int&gt; object.longest_common_substring(basic_string_view&lt;Symbol&gt; pattern) const</code> — O(P log(σ+1))</summary>

元列内の開始、pattern内の開始、最長共通部分文字列の長さを返します。

{% raw %}
```cpp
auto [a,b,n] = object.longest_common_substring("babc");
```
{% endraw %}

注意点: 空/共通なしは(0,0,0)。同長の解の選び方には依存しないでください。P<=INT_MAX。

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

- [suisen](https://suisen-cp.github.io/cp-library-cpp/library/string/suffix_automaton.hpp.html) の汎用map遷移と [Nyaan](https://nyaannyaan.github.io/library/string/suffix-automaton.hpp.html) の状態表現を比較し、独立実装。
- `verify/string/suffix-automaton-substrings.test.cpp`: [Number of Substrings](https://judge.yosupo.jp/problem/number_of_substrings)。
- `verify/string/suffix-automaton-lcs.test.cpp`: [Longest Common Substring](https://judge.yosupo.jp/problem/longest_common_substring)。
- `tests/random/suffix-automaton.cpp`: 空・反復・clone・整数alphabet・末尾追加後countを愚直なsubstring集合/走査と比較。
