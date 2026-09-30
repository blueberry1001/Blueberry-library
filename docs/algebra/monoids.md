---
title: モノイド集 — ヘッダ版のAPI詳細
documentation_of: //blueberry/algebra/monoids.hpp
---

**[いろいろなモノイド：S・Fとコピー用コードの早見表]({{ '/monoids.html' | relative_url }})**

ACL に渡す定義を探す場合は早見表を使ってください。各例を単独でコピーでき、Blueberry のヘッダは不要です。
このページは既存の `blueberry::monoid` を使う場合の詳細仕様です。

<details markdown="1">
<summary>ヘッダ版の使い方・API詳細を開く</summary>

## 概要・前提

ACL の `segtree` / `lazy_segtree` にそのまま渡せる8種類のレシピです。
セグ木本体を再実装せず、`blueberry::monoid` 以下に状態を持たない型を置きます。
各型の `S` は区間集約、遅延作用がある型の `F` は更新です。関数は全て static、値渡し・値返しで、保持する参照や無効化規則はありません。

N を要素数とします。スカラー演算・コピーを O(1) とした各操作は最悪 O(1)、追加メモリ O(1)、動的確保なしです。
ACL 上では構築 O(N)、点変更・区間積・遅延区間更新 O(log N)、全体積 O(1)、格納メモリ O(N) です。
多倍長数や文字列などを T にする場合は、その演算・コピー費用を掛けます。

`AffineSum`、`IndexAffineSum`、`AffineSumSquares`、`AffineComposition` の T は整数から構築できる可換環を想定します（整数、ACL modint など）。
整数では全ての中間結果が型に収まることが前提です。符号付きオーバーフローは未定義動作で、飽和処理はありません。
浮動小数点は丸めにより厳密な結合則を満たしません。
`MaxSubarray` は加法と大小比較が整合する全順序加法群を想定し、modint やラップアラウンド整数には向きません。
`MaxCount` はデフォルト構築・コピーと厳密な全順序の `<` が必要です。NaN は使えません。
全ての long long の個数・残高・積も範囲内に収めてください。転倒数の最大値は floor(N²/4) です。

`S` は `leaf` と `op` から得られる有効な区間状態だけを使います。任意の不整合な集約値に結合則は保証しません。
空区間は `e()`。`S{}` は特に関数合成では単位元にならないので、初期化には `e()` を使ってください。
長さだけを渡す ACL コンストラクタは全葉が `e()` であり、ゼロ値 N 個とは異なります。
長さや添字を使う作用には必ず `leaf` で作ったベクトルを渡します。
区間は 0-indexed 半開区間 [l,r)、更新の合成は **古い g の後に新しい f**、すなわち `composition(f,g)=f∘g` です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include <atcoder/lazysegtree>
#include "blueberry/algebra/monoids.hpp"
int main() {
    using M = blueberry::monoid::IndexAffineSum<long long>;
    std::vector<M::S> v;
    for (int i = 0; i < 5; ++i) v.push_back(M::leaf(0, i));
    atcoder::lazy_segtree<M::S, M::op, M::e, M::F,
                         M::mapping, M::composition, M::id> seg(v);
    int l = 1, r = 5;
    long long first = 3, step = 2;
    seg.apply(l, r, M::F{1, step, first - step * l});
    assert(seg.prod(0, 5).sum == 24); // 0, 3, 5, 7, 9
    seg.apply(2, 4, M::F{0, 0, 10}); // 0, 3, 10, 10, 9
    assert(seg.all_prod().sum == 32);
    assert(seg.prod(2, 2).len == 0);
}
```
{% endraw %}

通常のセグ木への接続例:

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
// #include <atcoder/segtree>
atcoder::segtree<M::S, M::op, M::e> seg(
    std::vector<M::S>{M::leaf(-2), M::leaf(5), M::leaf(-1)});
assert(seg.prod(0, 3).best == 5);
```
{% endraw %}

## 操作一覧

| 型 | 用途 | 遅延更新 |
| --- | --- | --- |
| [`AffineSum`](#affinesum) | 区間 affine 更新・区間和 | 対応 |
| [`IndexAffineSum`](#indexaffinesum) | 等差数列加算・添字 affine 更新 | 対応 |
| [`AffineSumSquares`](#affinesumsquares) | affine 更新・和と二乗和 | 対応 |
| [`BinaryFlipInversions`](#binaryflipinversions) | 0/1 反転・転倒数 | 対応 |
| [`MaxSubarray`](#maxsubarray) | 最大部分区間和 | なし |
| [`Bracket`](#bracket) | 括弧列の正当性 | なし |
| [`AffineComposition`](#affinecomposition) | 一次関数の合成 | なし |
| [`MaxCount`](#maxcount) | 最大値と出現回数 | なし |

## AffineSum: 区間 affine 更新・区間和 {#affinesum}

`using M = blueberry::monoid::AffineSum<long long>;`

S::sum と F::a,b は T、S::len は long long。

`x → a*x+b`。加算 `{1,d}`、代入 `{0,v}`、乗算 `{k,0}` を同じ型で扱えます。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{3, 1}` | O(1) | [開く](#affinesum-aggregate) |
| `s.sum` | O(1) | [開く](#affinesum-sum) |
| `s.len` | O(1) | [開く](#affinesum-len) |
| `M::S M::leaf(T x)` | O(1) | [開く](#affinesum-leaf) |
| `M::S M::e()` | O(1) | [開く](#affinesum-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#affinesum-op) |
| `M::F f{2, 1}` | O(1) | [開く](#affinesum-f-aggregate) |
| `f.a` | O(1) | [開く](#affinesum-f-a) |
| `f.b` | O(1) | [開く](#affinesum-f-b) |
| `M::S M::mapping(M::F f, M::S x)` | O(1) | [開く](#affinesum-mapping) |
| `M::F M::composition(M::F f, M::F g)` | O(1) | [開く](#affinesum-composition) |
| `M::F M::id()` | O(1) | [開く](#affinesum-id) |

<details class="api-operation" id="affinesum-aggregate" markdown="1">
<summary><code>M::S s{3, 1}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は sum, len。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
M::S s = M::leaf(3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-sum" markdown="1">
<summary><code>s.sum</code> — O(1)</summary>

区間和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto s = M::leaf(3);
auto value = s.sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-len" markdown="1">
<summary><code>s.len</code> — O(1)</summary>

要素数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto s = M::leaf(3);
auto value = s.len;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-leaf" markdown="1">
<summary><code>M::S M::leaf(T x)</code> — O(1)</summary>

1要素の有効な状態を返します。値は前提の型・算術範囲を満たしてください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto s = M::leaf(3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto s = M::op(M::leaf(3), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-f-aggregate" markdown="1">
<summary><code>M::F f{2, 1}</code> — O(1)</summary>

作用型 F は公開 aggregate。フィールド順は a, b。明示的なコンストラクタはなく、コピー・ムーブは T に従います。F{} は恒等作用ではありません。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto f = M::F{2, 1};
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-f-a" markdown="1">
<summary><code>f.a</code> — O(1)</summary>

乗数。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto f = M::F{2, 1};
auto coefficient = f.a;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-f-b" markdown="1">
<summary><code>f.b</code> — O(1)</summary>

加算値。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto f = M::F{2, 1};
auto coefficient = f.b;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-mapping" markdown="1">
<summary><code>M::S M::mapping(M::F f, M::S x)</code> — O(1)</summary>

区間 x の全要素に f を適用した集約を返します。元の x は変わりません。空区間は空のまま、長さ・絶対添字は保持します。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto s = M::mapping(M::F{2, 1}, M::leaf(3));
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-composition" markdown="1">
<summary><code>M::F M::composition(M::F f, M::F g)</code> — O(1)</summary>

g の後に f を適用する作用を返します。両作用の対象となる区間が同じ場合の合成です。順序を逆にすると結果が変わることがあります。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto f = M::composition(M::F{2, 1}, M::id());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesum-id" markdown="1">
<summary><code>M::F M::id()</code> — O(1)</summary>

全ての値を変えない恒等作用を返します。F{} と混同しないでください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSum<long long>;
auto identity = M::id();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## IndexAffineSum: 等差数列加算・添字 affine 更新 {#indexaffinesum}

`using M = blueberry::monoid::IndexAffineSum<long long>;`

S::sum,index_sum と F::a,b,c は T、S::len は long long。

絶対添字 `i` で `x_i → a*x_i+b*i+c`。区間 `[l,r)` に初項 `s`・公差 `d` を加えるには `{1,d,s-d*l}`。代入なら `{0,d,s-d*l}`。`leaf` の添字は実際の配置と一致させます。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{3, 2, 1}` | O(1) | [開く](#indexaffinesum-aggregate) |
| `s.sum` | O(1) | [開く](#indexaffinesum-sum) |
| `s.index_sum` | O(1) | [開く](#indexaffinesum-index_sum) |
| `s.len` | O(1) | [開く](#indexaffinesum-len) |
| `M::S M::leaf(T x, long long i)` | O(1) | [開く](#indexaffinesum-leaf) |
| `M::S M::e()` | O(1) | [開く](#indexaffinesum-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#indexaffinesum-op) |
| `M::F f{1, 2, 3}` | O(1) | [開く](#indexaffinesum-f-aggregate) |
| `f.a` | O(1) | [開く](#indexaffinesum-f-a) |
| `f.b` | O(1) | [開く](#indexaffinesum-f-b) |
| `f.c` | O(1) | [開く](#indexaffinesum-f-c) |
| `M::S M::mapping(M::F f, M::S x)` | O(1) | [開く](#indexaffinesum-mapping) |
| `M::F M::composition(M::F f, M::F g)` | O(1) | [開く](#indexaffinesum-composition) |
| `M::F M::id()` | O(1) | [開く](#indexaffinesum-id) |

<details class="api-operation" id="indexaffinesum-aggregate" markdown="1">
<summary><code>M::S s{3, 2, 1}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は sum, index_sum, len。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
M::S s = M::leaf(3, 2);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-sum" markdown="1">
<summary><code>s.sum</code> — O(1)</summary>

区間和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::leaf(3, 2);
auto value = s.sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-index_sum" markdown="1">
<summary><code>s.index_sum</code> — O(1)</summary>

絶対添字の和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::leaf(3, 2);
auto value = s.index_sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-len" markdown="1">
<summary><code>s.len</code> — O(1)</summary>

要素数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::leaf(3, 2);
auto value = s.len;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-leaf" markdown="1">
<summary><code>M::S M::leaf(T x, long long i)</code> — O(1)</summary>

1要素の有効な状態を返します。i は実際の絶対添字（0 <= i < N）。和の計算時にも変更しません。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::leaf(3, 2);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::op(M::leaf(3, 2), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-f-aggregate" markdown="1">
<summary><code>M::F f{1, 2, 3}</code> — O(1)</summary>

作用型 F は公開 aggregate。フィールド順は a, b, c。明示的なコンストラクタはなく、コピー・ムーブは T に従います。F{} は恒等作用ではありません。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto f = M::F{1, 2, 3};
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-f-a" markdown="1">
<summary><code>f.a</code> — O(1)</summary>

値の乗数。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto f = M::F{1, 2, 3};
auto coefficient = f.a;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-f-b" markdown="1">
<summary><code>f.b</code> — O(1)</summary>

添字の係数。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto f = M::F{1, 2, 3};
auto coefficient = f.b;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-f-c" markdown="1">
<summary><code>f.c</code> — O(1)</summary>

定数項。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto f = M::F{1, 2, 3};
auto coefficient = f.c;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-mapping" markdown="1">
<summary><code>M::S M::mapping(M::F f, M::S x)</code> — O(1)</summary>

区間 x の全要素に f を適用した集約を返します。元の x は変わりません。空区間は空のまま、長さ・絶対添字は保持します。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto s = M::mapping(M::F{1, 2, 3}, M::leaf(3, 2));
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-composition" markdown="1">
<summary><code>M::F M::composition(M::F f, M::F g)</code> — O(1)</summary>

g の後に f を適用する作用を返します。両作用の対象となる区間が同じ場合の合成です。順序を逆にすると結果が変わることがあります。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto f = M::composition(M::F{1, 2, 3}, M::id());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="indexaffinesum-id" markdown="1">
<summary><code>M::F M::id()</code> — O(1)</summary>

全ての値を変えない恒等作用を返します。F{} と混同しないでください。

{% raw %}
```cpp
using M = blueberry::monoid::IndexAffineSum<long long>;
auto identity = M::id();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## AffineSumSquares: affine 更新・和と二乗和 {#affinesumsquares}

`using M = blueberry::monoid::AffineSumSquares<long long>;`

S::sum,square_sum と F::a,b は T、S::len は long long。

`(a*x+b)^2=a²*x²+2ab*x+b²` を利用。`len*square_sum-sum*sum` から分散の分子や全ての組の差の二乗和を計算できます。除算はこの型に含みません。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{3, 9, 1}` | O(1) | [開く](#affinesumsquares-aggregate) |
| `s.sum` | O(1) | [開く](#affinesumsquares-sum) |
| `s.square_sum` | O(1) | [開く](#affinesumsquares-square_sum) |
| `s.len` | O(1) | [開く](#affinesumsquares-len) |
| `M::S M::leaf(T x)` | O(1) | [開く](#affinesumsquares-leaf) |
| `M::S M::e()` | O(1) | [開く](#affinesumsquares-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#affinesumsquares-op) |
| `M::F f{2, 1}` | O(1) | [開く](#affinesumsquares-f-aggregate) |
| `f.a` | O(1) | [開く](#affinesumsquares-f-a) |
| `f.b` | O(1) | [開く](#affinesumsquares-f-b) |
| `M::S M::mapping(M::F f, M::S x)` | O(1) | [開く](#affinesumsquares-mapping) |
| `M::F M::composition(M::F f, M::F g)` | O(1) | [開く](#affinesumsquares-composition) |
| `M::F M::id()` | O(1) | [開く](#affinesumsquares-id) |

<details class="api-operation" id="affinesumsquares-aggregate" markdown="1">
<summary><code>M::S s{3, 9, 1}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は sum, square_sum, len。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
M::S s = M::leaf(3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-sum" markdown="1">
<summary><code>s.sum</code> — O(1)</summary>

区間和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::leaf(3);
auto value = s.sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-square_sum" markdown="1">
<summary><code>s.square_sum</code> — O(1)</summary>

要素の二乗和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::leaf(3);
auto value = s.square_sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-len" markdown="1">
<summary><code>s.len</code> — O(1)</summary>

要素数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::leaf(3);
auto value = s.len;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-leaf" markdown="1">
<summary><code>M::S M::leaf(T x)</code> — O(1)</summary>

1要素の有効な状態を返します。値は前提の型・算術範囲を満たしてください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::leaf(3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::op(M::leaf(3), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-f-aggregate" markdown="1">
<summary><code>M::F f{2, 1}</code> — O(1)</summary>

作用型 F は公開 aggregate。フィールド順は a, b。明示的なコンストラクタはなく、コピー・ムーブは T に従います。F{} は恒等作用ではありません。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto f = M::F{2, 1};
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-f-a" markdown="1">
<summary><code>f.a</code> — O(1)</summary>

乗数。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto f = M::F{2, 1};
auto coefficient = f.a;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-f-b" markdown="1">
<summary><code>f.b</code> — O(1)</summary>

加算値。更新を表す値として読み書きできます。係数の全中間演算が T に収まることが必要です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto f = M::F{2, 1};
auto coefficient = f.b;
(void)coefficient;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-mapping" markdown="1">
<summary><code>M::S M::mapping(M::F f, M::S x)</code> — O(1)</summary>

区間 x の全要素に f を適用した集約を返します。元の x は変わりません。空区間は空のまま、長さ・絶対添字は保持します。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto s = M::mapping(M::F{2, 1}, M::leaf(3));
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-composition" markdown="1">
<summary><code>M::F M::composition(M::F f, M::F g)</code> — O(1)</summary>

g の後に f を適用する作用を返します。両作用の対象となる区間が同じ場合の合成です。順序を逆にすると結果が変わることがあります。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto f = M::composition(M::F{2, 1}, M::id());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinesumsquares-id" markdown="1">
<summary><code>M::F M::id()</code> — O(1)</summary>

全ての値を変えない恒等作用を返します。F{} と混同しないでください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineSumSquares<long long>;
auto identity = M::id();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## BinaryFlipInversions: 0/1 反転・転倒数 {#binaryflipinversions}

`using M = blueberry::monoid::BinaryFlipInversions;`

S::zero,one,inversions は全て long long。F は bool。

反転後の転倒数は `zero*one-inversions`。非可換で、連結時に左の `one` と右の `zero` の積を加えます。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{0, 1, 0}` | O(1) | [開く](#binaryflipinversions-aggregate) |
| `s.zero` | O(1) | [開く](#binaryflipinversions-zero) |
| `s.one` | O(1) | [開く](#binaryflipinversions-one) |
| `s.inversions` | O(1) | [開く](#binaryflipinversions-inversions) |
| `M::S M::leaf(bool x)` | O(1) | [開く](#binaryflipinversions-leaf) |
| `M::S M::e()` | O(1) | [開く](#binaryflipinversions-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#binaryflipinversions-op) |
| `M::F f = false` | O(1) | [開く](#binaryflipinversions-f) |
| `M::S M::mapping(M::F f, M::S x)` | O(1) | [開く](#binaryflipinversions-mapping) |
| `M::F M::composition(M::F f, M::F g)` | O(1) | [開く](#binaryflipinversions-composition) |
| `M::F M::id()` | O(1) | [開く](#binaryflipinversions-id) |

<details class="api-operation" id="binaryflipinversions-aggregate" markdown="1">
<summary><code>M::S s{0, 1, 0}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は zero, one, inversions。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
M::S s = M::leaf(true);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-zero" markdown="1">
<summary><code>s.zero</code> — O(1)</summary>

0 の個数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::leaf(true);
auto value = s.zero;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-one" markdown="1">
<summary><code>s.one</code> — O(1)</summary>

1 の個数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::leaf(true);
auto value = s.one;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-inversions" markdown="1">
<summary><code>s.inversions</code> — O(1)</summary>

左の 1 と右の 0 の組数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::leaf(true);
auto value = s.inversions;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-leaf" markdown="1">
<summary><code>M::S M::leaf(bool x)</code> — O(1)</summary>

1要素の有効な状態を返します。入力が整数なら 0/1 であることを呼出側で確認してください。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::leaf(true);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::op(M::leaf(true), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-f" markdown="1">
<summary><code>M::F f = false</code> — O(1)</summary>

F は bool の別名。false は無操作、true は各ビット反転です。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
M::F f = true;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-mapping" markdown="1">
<summary><code>M::S M::mapping(M::F f, M::S x)</code> — O(1)</summary>

区間 x の全要素に f を適用した集約を返します。元の x は変わりません。空区間は空のまま、長さ・絶対添字は保持します。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto s = M::mapping(true, M::leaf(true));
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-composition" markdown="1">
<summary><code>M::F M::composition(M::F f, M::F g)</code> — O(1)</summary>

g の後に f を適用する作用を返します。両作用の対象となる区間が同じ場合の合成です。順序を逆にすると結果が変わることがあります。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto f = M::composition(true, M::id());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="binaryflipinversions-id" markdown="1">
<summary><code>M::F M::id()</code> — O(1)</summary>

全ての値を変えない恒等作用を返します。F{} と混同しないでください。

{% raw %}
```cpp
using M = blueberry::monoid::BinaryFlipInversions;
auto identity = M::id();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## MaxSubarray: 最大部分区間和 {#maxsubarray}

`using M = blueberry::monoid::MaxSubarray<long long>;`

S::sum,prefix,suffix,best は全て T。

区間を跨ぐ最適解は左 suffix + 右 prefix。全要素が負でも best は 0 です。区間加算にはこの4値だけでは足りず、遅延作用は提供しません。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{-3, 0, 0, 0}` | O(1) | [開く](#maxsubarray-aggregate) |
| `s.sum` | O(1) | [開く](#maxsubarray-sum) |
| `s.prefix` | O(1) | [開く](#maxsubarray-prefix) |
| `s.suffix` | O(1) | [開く](#maxsubarray-suffix) |
| `s.best` | O(1) | [開く](#maxsubarray-best) |
| `M::S M::leaf(T x)` | O(1) | [開く](#maxsubarray-leaf) |
| `M::S M::e()` | O(1) | [開く](#maxsubarray-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#maxsubarray-op) |

<details class="api-operation" id="maxsubarray-aggregate" markdown="1">
<summary><code>M::S s{-3, 0, 0, 0}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は sum, prefix, suffix, best。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
M::S s = M::leaf(-3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-sum" markdown="1">
<summary><code>s.sum</code> — O(1)</summary>

区間全体の和。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::leaf(-3);
auto value = s.sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-prefix" markdown="1">
<summary><code>s.prefix</code> — O(1)</summary>

最大接頭辞和（空を含む）。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::leaf(-3);
auto value = s.prefix;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-suffix" markdown="1">
<summary><code>s.suffix</code> — O(1)</summary>

最大接尾辞和（空を含む）。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::leaf(-3);
auto value = s.suffix;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-best" markdown="1">
<summary><code>s.best</code> — O(1)</summary>

最大部分区間和（空を含む）。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::leaf(-3);
auto value = s.best;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-leaf" markdown="1">
<summary><code>M::S M::leaf(T x)</code> — O(1)</summary>

1要素の有効な状態を返します。値は前提の型・算術範囲を満たしてください。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::leaf(-3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxsubarray-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::MaxSubarray<long long>;
auto s = M::op(M::leaf(-3), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## Bracket: 括弧列の正当性 {#bracket}

`using M = blueberry::monoid::Bracket;`

S::sum,min_prefix は全て long long。

prod(l,r) の sum==0 && min_prefix==0 で正しい括弧列。点変更・2点 swap に対応。空列も正しい括弧列です。min_prefix>=0 は max_right 用の単調な条件ですが、正しい括弧列かどうか自体は単調ではありません。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{1, 0}` | O(1) | [開く](#bracket-aggregate) |
| `s.sum` | O(1) | [開く](#bracket-sum) |
| `s.min_prefix` | O(1) | [開く](#bracket-min_prefix) |
| `M::S M::leaf(char c)` | O(1) | [開く](#bracket-leaf) |
| `M::S M::e()` | O(1) | [開く](#bracket-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#bracket-op) |

<details class="api-operation" id="bracket-aggregate" markdown="1">
<summary><code>M::S s{1, 0}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は sum, min_prefix。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
M::S s = M::leaf('(');
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="bracket-sum" markdown="1">
<summary><code>s.sum</code> — O(1)</summary>

開き括弧数から閉じ括弧数を引いた値。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
auto s = M::leaf('(');
auto value = s.sum;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="bracket-min_prefix" markdown="1">
<summary><code>s.min_prefix</code> — O(1)</summary>

空を含む接頭辞の最小残高。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
auto s = M::leaf('(');
auto value = s.min_prefix;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="bracket-leaf" markdown="1">
<summary><code>M::S M::leaf(char c)</code> — O(1)</summary>

1要素の有効な状態を返します。c は '(' または ')' のみ。違反は assert の対象です。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
auto s = M::leaf('(');
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="bracket-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="bracket-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::Bracket;
auto s = M::op(M::leaf('('), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## AffineComposition: 一次関数の合成 {#affinecomposition}

`using M = blueberry::monoid::AffineComposition<long long>;`

S::a,b は全て T。

葉を左から順に適用します。`op(x,y)=y∘x` なので、ACL の `composition(f,g)=f∘g` と引数の意味が異なります。結果 `z` の関数値は `z.a*t+z.b`。非可換です。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{2, 3}` | O(1) | [開く](#affinecomposition-aggregate) |
| `s.a` | O(1) | [開く](#affinecomposition-a) |
| `s.b` | O(1) | [開く](#affinecomposition-b) |
| `M::S M::leaf(T a, T b)` | O(1) | [開く](#affinecomposition-leaf) |
| `M::S M::e()` | O(1) | [開く](#affinecomposition-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#affinecomposition-op) |

<details class="api-operation" id="affinecomposition-aggregate" markdown="1">
<summary><code>M::S s{2, 3}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は a, b。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
M::S s = M::leaf(2, 3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinecomposition-a" markdown="1">
<summary><code>s.a</code> — O(1)</summary>

合成関数の乗数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
auto s = M::leaf(2, 3);
auto value = s.a;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinecomposition-b" markdown="1">
<summary><code>s.b</code> — O(1)</summary>

合成関数の定数項。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
auto s = M::leaf(2, 3);
auto value = s.b;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinecomposition-leaf" markdown="1">
<summary><code>M::S M::leaf(T a, T b)</code> — O(1)</summary>

1要素の有効な状態を返します。値は前提の型・算術範囲を満たしてください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
auto s = M::leaf(2, 3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinecomposition-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="affinecomposition-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::AffineComposition<long long>;
auto s = M::op(M::leaf(2, 3), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## MaxCount: 最大値と出現回数 {#maxcount}

`using M = blueberry::monoid::MaxCount<long long>;`

S::value は T、S::count は long long。

count==0 で空を表現するので、最小値の番兵は不要。数値以外にも厳密な全順序を持つ型で使えます。

### 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `M::S s{-3, 1}` | O(1) | [開く](#maxcount-aggregate) |
| `s.value` | O(1) | [開く](#maxcount-value) |
| `s.count` | O(1) | [開く](#maxcount-count) |
| `M::S M::leaf(T x)` | O(1) | [開く](#maxcount-leaf) |
| `M::S M::e()` | O(1) | [開く](#maxcount-e) |
| `M::S M::op(M::S x, M::S y)` | O(1) | [開く](#maxcount-op) |

<details class="api-operation" id="maxcount-aggregate" markdown="1">
<summary><code>M::S s{-3, 1}</code> — O(1)</summary>

集約型 S は公開 aggregate。フィールド順は value, count。明示的なコンストラクタはありません。値初期化・コピー・ムーブは T の対応操作に従います。通常は leaf/e を推奨します。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
M::S s = M::leaf(-3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxcount-value" markdown="1">
<summary><code>s.value</code> — O(1)</summary>

最大値（count が 0 なら意味を持たない）。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
auto s = M::leaf(-3);
auto value = s.value;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxcount-count" markdown="1">
<summary><code>s.count</code> — O(1)</summary>

最大値の出現回数。書き換えは他フィールドとの不変条件を保つ必要があります。返す値は所有値です。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
auto s = M::leaf(-3);
auto value = s.count;
(void)value;
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxcount-leaf" markdown="1">
<summary><code>M::S M::leaf(T x)</code> — O(1)</summary>

1要素の有効な状態を返します。値は前提の型・算術範囲を満たしてください。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
auto s = M::leaf(-3);
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxcount-e" markdown="1">
<summary><code>M::S M::e()</code> — O(1)</summary>

空区間の単位元を返します。左右どちらと op しても有効な状態を変えません。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
auto empty = M::e();
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

<details class="api-operation" id="maxcount-op" markdown="1">
<summary><code>M::S M::op(M::S x, M::S y)</code> — O(1)</summary>

左の区間 x に右の区間 y を連結した集約を返します。空区間も許可。非可換の型では引数の順序を保ってください。

{% raw %}
```cpp
using M = blueberry::monoid::MaxCount<long long>;
auto s = M::op(M::leaf(-3), M::e());
```
{% endraw %}

注意点: 冒頭の型・オーバーフロー条件に従います。参照は保持せず、他の集約値やセグ木を無効化しません。

</details>

## 出典・練習問題・検証

代数式から実装し、公開提出コードはコピーしていません。ACL 本体は外部依存です。

| レシピ | 使える問題・公式資料 | 適用方法 |
| --- | --- | --- |
| AffineSum | [Library Checker: Range Affine Range Sum](https://judge.yosupo.jp/problem/range_affine_range_sum) | T を modint998244353 にする |
| AffineComposition | [Library Checker: Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite) | prod の `a*x+b` を出力 |
| BinaryFlipInversions | [AtCoder practice2 L](https://atcoder.jp/contests/practice2/tasks/practice2_l) | 区間反転と転倒数を直接処理 |
| IndexAffineSum | [CSES Polynomial Queries](https://cses.fi/problemset/task/1736) | 0-indexed [l,r) には {1,1,1-l} を適用 |
| MaxSubarray | [CSES Subarray Sum Queries](https://cses.fi/problemset/task/1190) | 点変更の後に all_prod().best |
| Bracket | [AtCoder ABC223 F: Parenthesis Checking](https://atcoder.jp/contests/abc223/tasks/abc223_f) | 2点 swap と区間の sum/min_prefix を利用 |

[ACL の lazy_segtree 契約](https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html)で
作用の分配則と composition の順序を確認しています。BinaryFlipInversions は同資料の例にもある
「異なるビットの組の総数から転倒数を引く」性質を使います。
公式問題へのドライバと固定 seed の境界・ランダム比較をリポジトリの verify/tests に用意しています。
[性能調査・測定条件と結果]({{ '/docs/development/monoid-performance.html' | relative_url }})も参照してください。
問題リンクは利用例であり、全リンクについて提出済みを意味しません。

</details>
