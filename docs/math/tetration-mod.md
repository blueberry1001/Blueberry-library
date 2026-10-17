---
title: Tetration Mod
documentation_of: //blueberry/math/tetration-mod.hpp
---

[カテゴリへ戻る]({{ '/categories/math.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要

同じ底の累乗を右から積み重ねたテトレーションの剰余を求めます。
高さ0を1、$0^0$を1と定め、底と法が互いに素でない場合にも対応します。
ACLに対応する公開APIはありません。

底を$a$、法を$m$、内部で行う素因数分解の合計時間を$F$とすると、時間はO($F+\log^2 m+\log(a+1)$)、
追加メモリはO($\log m$)です。高さをそのまま走査せず、64bitの大きな高さも扱います。
素因数分解には最悪時間の保証がありません。

## 操作一覧

| 操作 | 計算量 |
| --- | --- |
| [`tetration_mod(base, height, mod)`](#tetration-mod) | O($F+\log^2 m+\log(a+1)$)、素因数分解の最悪時間保証なし |

<!-- library-implementation -->

<details class="library-notes" markdown="1">
<summary>補足・制約</summary>

## 概要・前提 {#概要前提}

- 底と高さは`std::uint64_t`全域、法は`1 <= mod <= UINT32_MAX`です。
  負整数や範囲外の整数は渡さないでください。引数への暗黙変換でもこの条件を満たす必要があります。
- $T(a,0)=1$、$T(a,h+1)=a^{T(a,h)}$を計算します。
  底0では高さが偶数なら1、奇数なら0です。法1での戻り値は常に0です。
- 固定幅整数の剰余演算を定数時間とみなします。素因数分解の詳細は
  [Factorize]({{ '/blueberry/math/factorize.hpp.html' | relative_url }})を参照してください。依存先の128bit整数にはGCC/Clangを使います。
- 法と底に共通する素因子をgcdで除き、残った部分のEuler関数で指数を縮めます。
  法は再帰の2段階以内ごとに半分以下になり、再帰の深さはO($\log m$)です。
- 剰余積は64bit、指数の周期調整も64bitで計算します。公開範囲内で整数のoverflowは起きません。
  テトレーション自体の巨大な値は構築しません。
- 入力を変更せず、参照や共有キャッシュを保持しません。内部の`tetration_mod_detail`は公開APIではありません。

</details>

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <cstdint>
#include <limits>
#include "blueberry/math/tetration-mod.hpp"
int main() {
  assert(blueberry::tetration_mod(2, 3, 100) == 16);
  assert(blueberry::tetration_mod(2, 4, 100) == 36);
  assert(blueberry::tetration_mod(6, 3, 72) == 0);
  assert(blueberry::tetration_mod(0, 0, 7) == 1);
  assert(blueberry::tetration_mod(0, 2, 7) == 1);
  assert(blueberry::tetration_mod(0, 3, 7) == 0);
  assert(blueberry::tetration_mod(2, std::numeric_limits<std::uint64_t>::max(), 8) == 0);
}
```
{% endraw %}

<details class="api-operation" id="tetration-mod" markdown="1">
<summary><code>uint32_t tetration_mod(uint64_t base, uint64_t height, uint32_t mod)</code> — O(F + log² m + log(a+1))</summary>

`base`を`height`段積み重ねた累乗の、`mod`による最小非負剰余を返します。
戻り値は`0 <= result < mod`です。法と底の互いに素という前提はありません。
高さ0では`1 % mod`、高さ1では`base % mod`を返します。
法1、底0・1、高さ0・1はO(1)です。高さ2は素因数分解を行わず、O($\log(a+1)$)時間で計算します。

{% raw %}
```cpp
auto value = blueberry::tetration_mod(3, 3, 1000); // 3^27 mod 1000 = 987
auto empty = blueberry::tetration_mod(123, 0, 17); // 1
auto one_mod = blueberry::tetration_mod(0, 0, 1);  // 0
```
{% endraw %}

注意点: `mod == 0`は前提違反でassertの対象です。
高さや底を法で先に縮めると結果が変わるため、元の値を渡してください。
計算量は、素因数分解にかかった時間$F$を含む1回の呼び出しに対するものです。
事前構築や呼び出し順への制限、参照の寿命・無効化の条件はありません。

</details>

<details class="library-notes" markdown="1">
<summary>出典・検証</summary>

## 出典・検証 {#出典検証}

- 公式問題: [Tetration Mod](https://judge.yosupo.jp/problem/tetration_mod)。
  公式制約を超える最大64bitの底・高さ、最大32bitの法は境界テストで補います。
- [Fastest提出150339](https://judge.yosupo.jp/submission/150339)、
  [suisenの提出108768](https://judge.yosupo.jp/submission/108768)のgcdによる共通因子除去と、
  [Nyaanの解説](https://nyaannyaan.github.io/library/modulo/tetration.hpp)、
  [maspypyの実装](https://maspypy.github.io/library/mod/tetration.hpp)のEuler関数列を調査しました。
  外部ソースは転記せず独立実装しました。
- 小さい累乗の多倍長整数による全探索と、別実装のEuler関数列・多倍長modular powerによる
  固定seedのランダム比較で検証します。

</details>
