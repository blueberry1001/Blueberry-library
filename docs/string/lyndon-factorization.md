---
title: Lyndon Factorization
documentation_of: //blueberry/string/lyndon-factorization.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }})

## 概要・前提

列を辞書順で非増加なLyndon語へ一意に分解します。Lyndon語は、非空で、どの非自明な巡回シフトよりも
辞書順で小さい語です。文字列の辞書順・周期を扱う問題で使用します。ACLにはこの分解操作はありません。

長さNの`std::string`、`std::string_view`、`std::vector<T>`など、`size()`とO(1)の添字アクセスが必要です。
要素の`<`は厳密弱順序を満たすこと。等値は`<`から判定するので`==`は不要です。
`N <= INT_MAX`。比較がO(1)なら時間O(N)、返り値O(N)、それ以外の作業メモリO(1)。
stringの要素もそのchar型の`<`で比較するため、非ASCII byteの順序を統一したければ`vector<unsigned char>`を使います。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <string>
#include <vector>
#include "blueberry/string/lyndon-factorization.hpp"
int main() {
  auto bounds = blueberry::lyndon_factorization(std::string("banana"));
  assert((bounds == std::vector<int>{0, 1, 3, 5, 6})); // b | an | an | a
  assert((blueberry::lyndon_factorization(std::vector<int>{}) == std::vector<int>{0}));
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> lyndon_factorization(const Sequence& sequence)` | O(N) | [開く](#factorization) |

<details class="api-operation" id="factorization" markdown="1">
<summary><code>std::vector&lt;int&gt; lyndon_factorization(const Sequence&amp; sequence)</code> — O(N)</summary>

境界列`{0,a1,...,N}`を返します。連続する境界`[a[i],a[i+1])`が1つの因子で、入力を変更しません。
空列では`{0}`、すべて同じ文字なら各1文字が因子です。Duval法の線形走査を用います。

{% raw %}
```cpp
std::string s = "abab";
auto bounds = blueberry::lyndon_factorization(s);
assert((bounds == std::vector<int>{0, 2, 4}));
```
{% endraw %}

注意点: 返り値は文字列片の借用ではなく整数境界の独立したvectorです。以後入力を書き換えれば、
その境界が新しい入力のLyndon分解になる保証はありません。
</details>

## 出典・検証

- [Duval, Factorizing words over an ordered alphabet](https://doi.org/10.1016/0196-6774(83)90017-2)の線形アルゴリズム。
- [Library Checker Lyndon Factorization](https://judge.yosupo.jp/problem/lyndon_factorization)で検証。
- [Fastest提出394259](https://judge.yosupo.jp/submission/394259)もDuval型の線形走査を採用。
  大量出力ではI/Oも時間に影響するため、提出時間を関数本体の速度と混同しません。コードは独立実装です。
- `tests/random/string-expansion.cpp` は小文字列の全分割と巡回シフト比較により、結果の一意性まで検証します。
