---
title: Linear RMQ
documentation_of: //blueberry/data-structure/linear-rmq.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

`blueberry::LinearRMQ<T, Compare = std::less<T>>` は静的配列の最小値・最小位置を返す。N 要素、`0 <= N <= INT_MAX`。比較は const 呼び出し可能な strict weak ordering、T と比較器はコピー可能とする。既定の比較は最小値、`std::greater<T>` なら最大値。同値は常に最左位置を返す。

B = max(1, floor(bit_width(N)/2)) のブロック内に単調スタックを unsigned のビットマスクで保存し、ブロック最小値だけに Sparse Table を構築する。単語長が log N 以上で bit_width/countr_zero を O(1) とする word RAM で、構築・メモリ O(1+N)、クエリ O(1)。固定ブロック幅の Sparse Table を線形と呼ぶ方式ではない。比較・コピー・破棄は O(1) とする。

値は所有コピーされ、入力の変更・破棄は影響しない。値の更新・空区間の集約は不可。空配列は構築できるが size 以外の問い合わせはできない。返り値は値であり内部参照を公開しない。ACL には静的線形 RMQ がない。D は代入先が保持する旧データの要素数。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/linear-rmq.hpp"
int main() {
  blueberry::LinearRMQ<int> r(std::vector<int>{5,2,2,7});
  assert(r.size()==4 && r.argmin(0,4)==1);
  assert(r.prod(2,4)==2 && r.get(0)==5);
  blueberry::LinearRMQ<int> empty;
  assert(empty.size()==0);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `LinearRMQ(const std::vector<T>& values = {}, Compare compare = Compare{})` | O(1 + N) | [開く](#constructor) |
| `LinearRMQ copy(const LinearRMQ& other)` | O(1 + N) | [開く](#copy-constructor) |
| `LinearRMQ& operator=(const LinearRMQ& other)` | O(1 + N) + D | [開く](#copy-assignment) |
| `LinearRMQ moved(LinearRMQ&& other)` | O(1) | [開く](#move-constructor) |
| `LinearRMQ& operator=(LinearRMQ&& other)` | O(1 + D) | [開く](#move-assignment) |
| `int size() const` | O(1) | [開く](#size) |
| `T get(int p) const` | O(1) | [開く](#get) |
| `int argmin(int l, int r) const` | O(1) | [開く](#argmin) |
| `T prod(int l, int r) const` | O(1) | [開く](#prod) |

<details class="api-operation" id="constructor" markdown="1">
<summary><code>LinearRMQ(const std::vector&lt;T&gt;&amp; values = {}, Compare compare = Compare{})</code> — O(1 + N)</summary>

値と比較器をコピーして前計算する。

{% raw %}
```cpp
blueberry::LinearRMQ<int> r(std::vector<int>{3,1,4});
```
{% endraw %}

注意点: 空入力も有効。Compare が既定構築できない場合は明示指定する。

</details>

<details class="api-operation" id="copy-constructor" markdown="1">
<summary><code>LinearRMQ copy(const LinearRMQ&amp; other)</code> — O(1 + N)</summary>

全データを独立に複製する。コピー元の更新・破棄はコピー先に影響しない。

{% raw %}
```cpp
LinearRMQ copy(other);
```
{% endraw %}

注意点: N はコピー元の要素数。追加メモリは元の構造と同じオーダー。

</details>

<details class="api-operation" id="copy-assignment" markdown="1">
<summary><code>LinearRMQ&amp; operator=(const LinearRMQ&amp; other)</code> — O(1 + N) + D</summary>

元データを破棄・置換し、自身の参照を返す。

{% raw %}
```cpp
object = other;
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己コピー代入は有効。既存の内部参照は無効化される。

</details>

<details class="api-operation" id="move-constructor" markdown="1">
<summary><code>LinearRMQ moved(LinearRMQ&amp;&amp; other)</code> — O(1)</summary>

所有権を移動する。

{% raw %}
```cpp
LinearRMQ moved(std::move(other));
```
{% endraw %}

注意点: <utility> が必要。移動元は再代入または破棄のみとする。

</details>

<details class="api-operation" id="move-assignment" markdown="1">
<summary><code>LinearRMQ&amp; operator=(LinearRMQ&amp;&amp; other)</code> — O(1 + D)</summary>

旧データを破棄し所有権を移動、自身の参照を返す。

{% raw %}
```cpp
object = std::move(other);
```
{% endraw %}

注意点: D は代入先の旧保持要素数。自己ムーブ後も再代入または破棄のみとする。内部参照は無効化される。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int size() const</code> — O(1)</summary>

配列長を返す。

{% raw %}
```cpp
assert(r.size()==3);
```
{% endraw %}

注意点: 空入力では0。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T get(int p) const</code> — O(1)</summary>

入力の p 番目の値を返す。

{% raw %}
```cpp
assert(r.get(0)==3);
```
{% endraw %}

注意点: 0 <= p < N。

</details>

<details class="api-operation" id="argmin" markdown="1">
<summary><code>int argmin(int l, int r) const</code> — O(1)</summary>

半開区間の比較最小値の最左添字を返す。

{% raw %}
```cpp
assert(r.argmin(0,3)==1);
```
{% endraw %}

注意点: 0 <= l < r <= N。空区間は禁止。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>T prod(int l, int r) const</code> — O(1)</summary>

argmin の位置の値を返す。

{% raw %}
```cpp
assert(r.prod(0,3)==1);
```
{% endraw %}

注意点: 一般のモノイド積ではなく、比較最小値専用。

</details>

## 出典・検証

[noshi91 の線形 RMQ の解説](https://noshi91.hatenablog.com/entry/2018/08/16/125415) と [Luzhiled の実装](https://ei1333.github.io/library/structure/others/linear-rmq.hpp) の小区間ビット管理を比較し、可変ブロック幅で独立実装した。公式 `staticrmq` を `verify/data-structure/linear-rmq.test.cpp` で検証。`tests/random/static-tree-expansion.cpp` で重複・最左優先・逆比較・2冪前後を愚直比較する。
