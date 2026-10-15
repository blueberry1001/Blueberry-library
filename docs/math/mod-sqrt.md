---
title: Modular Square Root
documentation_of: //blueberry/math/mod-sqrt.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

素数pに対してx²≡a (mod p)を解き、存在すれば最小の非負の根を返します。modulusは引数として渡し、素数性は検査しません。

非剰余探索の候補数をzとして時間O(z log p+log² p)、追加メモリO(1)です。p%4==3はO(log p)、p==2と剰余0はO(1)です。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`mod_sqrt`](#mod-sqrt) | O(z log p + log² p)、追加O(1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- `2 <= prime_mod <= INT32_MAX` かつ素数が前提です。aは任意のint64_tで、負値も `[0,p)` へ正規化します。範囲内の積はint64に収まります。
- ACLにないTonelli–Shanks法の演算です。globalなmodint設定を変更せず、既存FPS内部の平方根処理とは独立しています。
- 非剰余候補は順次探索するため、zを無条件に定数とはみなしません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/math/mod-sqrt.hpp"
int main() {
  assert(blueberry::mod_sqrt(10, 13).value() == 6);
  assert(blueberry::mod_sqrt(-3, 13).value() == 6);
  assert(!blueberry::mod_sqrt(2, 13));
  assert(blueberry::mod_sqrt(0, 2).value() == 0);
}
```
{% endraw %}

<details class="api-operation" id="mod-sqrt" markdown="1">
<summary><code>optional&lt;int64_t&gt; mod_sqrt(int64_t a, int64_t prime_mod)</code> — O(z log p + log² p)</summary>

解がなければ `nullopt`。解があれば `[0,p)` の最小の根です。
奇素数と非零の平方剰余では根が2つあり、`min(r,p-r)` を返します。0の根は0です。

{% raw %}
```cpp
if (auto root = blueberry::mod_sqrt(4, 7)) {
  assert(*root == 2); // 5も根だが小さい方を返す
}
```
{% endraw %}

注意点: modulusの範囲外・合成数は前提違反です。範囲はassertで検査しますが、
合成数判定は行いません。合成数での停止・結果を保証しません。
`a == INT64_MIN` も正規化可能です。optionalは値を所有するため参照の寿命制限はありません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証・性能</summary>

## 出典・検証 {#出典検証}

- [NyaanのTonelli–Shanks](https://github.com/NyaanNyaan/library/blob/master/modulo/mod-sqrt.hpp) と
  [Cipollaの提出215966](https://judge.yosupo.jp/submission/215966) を比較し、
  global modulusに依存しないTonelli–Shanksを独立実装しました。外部コードは転記していません。
  Fastest提出の複数mod同時SIMD処理は単発APIへ持ち込んでいません。

- 公式問題: [Sqrt Mod](https://judge.yosupo.jp/problem/sqrt_mod)。
  小素数の全残余に対する全探索・負値・INT64_MIN・INT32_MAX素数も追加検証します。

</details>
