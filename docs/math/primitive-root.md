---
title: Primitive Root
documentation_of: //blueberry/math/primitive-root.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

素数pを法とする乗法群の原始根を1つ求めます。`uint64_t`全域の素数を扱い、最小の原始根は保証しません。ACLの原始根計算は内部の32bit用機能なので、大きな素数を実行時に扱う場合に使います。

p−1の分解時間をF、異なる素因数の個数をd、試した候補数をKとすると、時間はO(F + K d log p)、追加メモリはO(log p)です。分解と候補探索には実用的な最悪時間保証がありません。p=2はO(1)時間・メモリです。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`uint64_t g = primitive_root(p)`](#primitive-root) | O(F + K d log p)、p=2はO(1) |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 入力は2以上の素数です。0・1・合成数には対応しません。`assert`は`p >= 2`だけを確認し、素数判定は行いません。必要なら呼び出し前に`is_prime(p)`を使います。
- 返り値gは`1 <= g < p`を満たし、gの位数はp−1です。p=2では1を返します。空入力に相当する引数はありません。
- p−1を`factorize`で分解し、各相異なる素因数qについて`g^((p-1)/q) != 1 (mod p)`を確かめます。Fには分解結果の整列を含みます。64bit整数と128bit剰余演算を定数時間として数えます。
- 最初に候補2を調べ、その後は入力から初期化した局所SplitMix64状態で候補を選びます。同じpには同じgを返し、共有乱数状態を変更しません。Kは実際の試行回数で、乱数分布を仮定した期待値を最悪時間の保証には使いません。
- 中間の剰余積は`unsigned __int128`で計算し、最大の64bit素数でもオーバーフローを避けます。GCC/Clangを対象とし、MSVC単体には対応しません。
- 引数は値渡し、返り値も値です。入力の変更、参照の無効化、呼び出し後に残る内部状態はありません。負整数を渡すとC++の符号なし整数への変換が起きます。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include "blueberry/math/primitive-root.hpp"
int main() {
  assert(blueberry::primitive_root(2) == 1);
  const std::uint64_t p = 17;
  const auto g = blueberry::primitive_root(p);
  std::uint64_t x = 1;
  bool seen[17]{};
  for (int i = 0; i < 16; ++i) {
    assert(!seen[x]);
    seen[x] = true;
    x = x * g % p;
  }
  assert(x == 1);
}
```
{% endraw %}

<details class="api-operation" id="primitive-root" markdown="1">
<summary><code>uint64_t primitive_root(uint64_t p)</code> — O(F + K d log p)</summary>

素数pを法とする原始根を返します。p−1の分解は呼び出すたびに行い、既知の分解や固定の法を保持しません。返り値を累乗すると、pで割った余りが0以外のすべての値を一巡します。

{% raw %}
```cpp
const auto g = blueberry::primitive_root(1'000'000'007);
assert(1 <= g && g < 1'000'000'007);
```
{% endraw %}

注意点: pは2以上の素数である必要があります。最小の原始根が必要な用途には使えません。内部の積は全`uint64_t`範囲に対応しますが、利用者が`g * g`などを計算するときは自身で中間積の範囲を確認してください。F・d・Kの定義は概要のとおりで、償却時間の保証はありません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- [Library Checker Primitive Root](https://judge.yosupo.jp/problem/primitive_root)で検証します。公式上限の10^18を超える素数、最大64bit素数、小さい素数の全剰余の列挙を追加テストで確認します。
- [Nyaanの64bit原始根](https://nyaannyaan.github.io/library/math/primitive-root-ll.hpp.html)、[ACLの内部原始根計算](https://github.com/atcoder/ac-library/blob/master/atcoder/internal_math.hpp)、[公開提出138249](https://judge.yosupo.jp/submission/138249)と[410843](https://judge.yosupo.jp/submission/410843)を調査し、素因数ごとの位数判定を独立に実装しました。外部ソースの転記はしていません。

</details>
