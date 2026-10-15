---
title: Longest Common Substring
documentation_of: //blueberry/string/longest-common-substring.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要

2文字列の最長共通連続部分文字列を求めます。バイト列として扱い、NUL・128以上のバイトも使用できます。UTF-8の文字境界は考慮しません。

N=s.size()+t.size()として、時間O(N)、追加メモリO(N)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`longest_common_substring`](#longest-common-substring) | O(N) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- ACLのsuffix arrayとLCPを利用する薄いラッパーです。`N < INT_MAX` が必要です。
- 入力を変更せず、返り値は整数のコピーです。入力への参照を保持しません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include "blueberry/string/longest-common-substring.hpp"
int main() {
  std::string s = "xabcy", t = "zabcw";
  auto [l, r, u, v] = blueberry::longest_common_substring(s, t);
  assert(r - l == 3 && s.substr(l, r - l) == t.substr(u, v - u));
}
```
{% endraw %}

<details class="api-operation" id="longest-common-substring" markdown="1">
<summary><code>array&lt;int,4&gt; longest_common_substring(string_view s, string_view t)</code> — O(N)</summary>

`{l,r,u,v}` を返す。0-indexed半開区間 `s[l,r)` と `t[u,v)` は等しく、長さが最大。
同長の候補の選択は未規定。空入力・共通するバイトがない場合は `{0,0,0,0}`。
各string_viewの参照先は呼び出し中有効であること。バイトを整数1..256へ変換し、
区切り0を追加するため文字コードの加算によるcharのオーバーフローはない。

{% raw %}
```cpp
auto result = blueberry::longest_common_substring("a", "b");
assert(result[0] == 0 && result[1] == 0);
```
{% endraw %}

注意点: 上記の入力制約と空入力の規約を守ること。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [ACL string](https://atcoder.github.io/ac-library/master/document_en/string.html): SA-IS / LCPを利用。
- [ei1333 longest-common-substring](https://github.com/ei1333/library/blob/master/string/longest-common-substring.hpp): 隣接する異なる文字列由来suffixのLCP。
- [suisen suffix automaton検証](https://github.com/suisen-cp/cp-library-cpp/blob/main/test/src/string/suffix_automaton/longest_common_substring.test.cpp): 別候補のautomaton方式。
- [Library Checker](https://judge.yosupo.jp/problem/longest_common_substring)、NUL・高位バイト・空文字列を含むランダム愚直比較。
- [比較・測定](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/sequence-batch-four.md)。参照実装のコードは転載していない。

</details>
