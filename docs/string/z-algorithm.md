---
title: Z Algorithm
documentation_of: //blueberry/string/z-algorithm.hpp
---

[カテゴリへ戻る]({{ '/categories/string.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

列全体と各接尾辞の最長共通接頭辞長を求めます。通常はACLのz_algorithmを優先してください。Nは列長。添字アクセス・比較 O(1) の前提で時間・追加メモリ O(N)。

## 最小使用例

```cpp
#include <cassert>
#include <string>
#include <vector>
#include "blueberry/string/z-algorithm.hpp"
int main() {
  auto z = blueberry::z_algorithm(std::string("ababa"));
  assert((z == std::vector<int>{5, 0, 3, 0, 1}));
  assert(blueberry::z_algorithm(std::string{}).empty());
  auto numbers = blueberry::z_algorithm(std::vector<int>{1, 1, 1});
  assert((numbers == std::vector<int>{3, 2, 1}));
}
```

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `vector<int> blueberry::z_algorithm(sequence)` | O(N) | [開く](#z-algorithm) |

以下の操作を開くと返り値・使用例・注意点を確認できます。断片の使用例は、必要なヘッダと有効な引数・オブジェクトがある前提です。

<details class="api-operation" id="z-algorithm" markdown="1">
<summary><code>vector&lt;int&gt; blueberry::z_algorithm(sequence)</code> — O(N)</summary>

z[i]は全体とiからの接尾辞の一致長です。空列には空vector、非空ならz[0]=Nです。

```cpp
auto z = blueberry::z_algorithm(std::string("aaaa"));
// {4, 3, 2, 1}
```

注意点: size()・添字アクセス・等値比較が必要です。文字列リテラルはstd::stringにしてください。UTF-8のstringは文字ではなくバイト単位。検索用の区切りは入力中にない値を選びます。

</details>
