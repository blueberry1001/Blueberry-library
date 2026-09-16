---
title: Potential Union Find
documentation_of: //blueberry/data-structure/potential-union-find.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

群の要素で表した相対値を管理する Union Find。ACL の DSU にない重みと矛盾検出を提供する。
`merge(u,v,w)` は **P[u] = op(P[v],w)**、`diff(u,v)` は **op(inv(P[v]),P[u])** を意味する。非可換群にも対応する。

`PotentialUnionFind<T,Op=std::plus<T>,Inv=std::negate<T>>`。`T` はコピー可能で `==` を持ち、`op` は結合的、`identity` は両側単位元、`inv` は群の逆元であること。加算群は既定でよい。行列積では単位行列と逆行列関数を渡す。演算のオーバーフローや浮動小数点誤差は呼び出し側の責任。

頂点数 N は `0 <= N <= INT_MAX`、頂点番号は `[0,N)`。空構築は可能。メモリ O(N)、群演算・コピーが O(1) の場合、union by size と経路圧縮で通常操作は償却 O(α(N))、単発最悪 O(log N)。内部再帰の深さも O(log N)。公開フィールドはない。返り値は値で所有し、参照の無効化はない。コピーは O(N) で独立した構造を作る。ムーブ後のオブジェクトは破棄・再代入できる。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/data-structure/potential-union-find.hpp"
int main() {
  blueberry::PotentialUnionFind<long long> uf(3);
  assert(uf.merge(0, 1, 7));
  assert(uf.merge(2, 1, -3));
  assert(uf.diff(0, 2) == 10);
  assert(!uf.merge(0, 2, 11));
  assert(uf.comp_size(0) == 3);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `PotentialUnionFind(n, op={}, inv={}, identity={})` | O(N) | [開く](#construct) |
| `int size() const` | O(1) | [開く](#size) |
| `int leader(int v)` | 償却 O(α(N)) | [開く](#leader) |
| `bool same(int u, int v)` | 償却 O(α(N)) | [開く](#same) |
| `int comp_size(int v)` | 償却 O(α(N)) | [開く](#comp-size) |
| `bool merge(int u, int v, const T& w)` | 償却 O(α(N)) | [開く](#merge) |
| `std::optional<T> diff(int u, int v)` | 償却 O(α(N)) | [開く](#diff) |
| `PotentialUnionFind(other); operator=(other)` | コピー O(N)、ムーブ O(1) ※代入先解放 O(N) | [開く](#copy-move) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>PotentialUnionFind(n, op={}, inv={}, identity={})</code> — O(N)</summary>

N 個の独立成分を構築する。テンプレート型 T を指定する。既定は加算・符号反転・T{}。

{% raw %}
```cpp
blueberry::PotentialUnionFind<long long> uf(0);
```
{% endraw %}

注意点: N は非負。乗算群に T{} を単位元として使わない。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

構築時の頂点数を返す。

{% raw %}
```cpp
assert(uf.size() == 3);
```
{% endraw %}

注意点: 空なら 0。

</details>

<details class="api-operation" id="leader" markdown="1">
<summary><code>int leader(int v)</code> — 償却 O(α(N))</summary>

v の成分代表を返し経路圧縮する。

{% raw %}
```cpp
int root = uf.leader(0);
assert(uf.leader(root) == root);
```
{% endraw %}

注意点: v は有効頂点。代表の番号は merge で変わり得る。

</details>

<details class="api-operation" id="same" markdown="1">
<summary><code>bool same(int u, int v)</code> — 償却 O(α(N))</summary>

同じ成分かを返す。

{% raw %}
```cpp
assert(uf.same(0, 2));
```
{% endraw %}

注意点: 両頂点は有効。内部経路圧縮を行うため const ではない。

</details>

<details class="api-operation" id="comp-size" markdown="1">
<summary><code>int comp_size(int v)</code> — 償却 O(α(N))</summary>

v の成分の頂点数を返す。

{% raw %}
```cpp
assert(uf.comp_size(1) == 3);
```
{% endraw %}

注意点: 有効頂点なら 1 以上。

</details>

<details class="api-operation" id="merge" markdown="1">
<summary><code>bool merge(int u, int v, const T&amp; w)</code> — 償却 O(α(N))</summary>

P[u] = op(P[v],w) の制約を追加する。既存制約と矛盾しなければ true。同一成分でも整合すれば true。矛盾なら false で、制約は採用しない。

{% raw %}
```cpp
assert(uf.merge(0, 2, 10));
assert(!uf.merge(0, 0, 1));
```
{% endraw %}

注意点: 有効頂点が必要。false でも内部経路圧縮は起こる。演算順序は非可換の場合に特に重要。

</details>

<details class="api-operation" id="diff" markdown="1">
<summary><code>std::optional&lt;T&gt; diff(int u, int v)</code> — 償却 O(α(N))</summary>

同じ成分なら op(inv(P[v]),P[u])、別成分なら nullopt を返す。

{% raw %}
```cpp
assert(uf.diff(2, 0) == -10);
```
{% endraw %}

注意点: u==v なら単位元。加算群では P[u]-P[v]。結果は独立した値。

</details>

<details class="api-operation" id="copy-move" markdown="1">
<summary><code>PotentialUnionFind(other); operator=(other)</code> — コピー O(N)、ムーブ O(1) ※代入先解放 O(N)</summary>

標準のコピー・ムーブを持つ。コピーは制約を引き継ぐ独立したオブジェクト。

{% raw %}
```cpp
auto copy = uf;
assert(copy.diff(0, 2) == 10);
```
{% endraw %}

注意点: Op/Inv/T の特殊メンバ操作が可能であること。ムーブ後は破棄または再代入する。

</details>

## 出典・検証

[公式加算群](https://judge.yosupo.jp/problem/unionfind_with_potential)、[公式非可換群](https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group) で検証。固定 seed の小グラフ BFS による行列制約の独立比較を行う。

比較資料: [Nyaan の加算版](https://nyaannyaan.github.io/library/data-structure/union-find-with-potential.hpp.html)、[mtsaka の群版](https://mtsaka.github.io/library/ds/union-find/weighted-union-find.hpp)。群演算順序を整理して独立実装した。
Fastest [224108](https://judge.yosupo.jp/submission/224108)、[393949](https://judge.yosupo.jp/submission/393949) は高速入力も使う。公開提出の時刻を本ライブラリの性能と直接比較しない。比較候補と測定条件は `benchmark/data-structure-expansion.cpp` と `benchmark/results/lc-expansion-ds/` を参照。
