---
title: Fenwick Tree
documentation_of: //blueberry/data-structure/fenwick-tree.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

一点加算と区間和を扱う Fenwick Tree です。要素数を $N$ とすると、構築は $O(N)$、
`add`・`sum`・`prefix_sum`・`pref`・`get`・`lower_bound` は $O(\log N)$、`size` は
$O(1)$ です。区間は半開区間 `[l,r)`、添字は `[0,N)` です。

型 `T` の `get` は `+=` と二項 `-` を持つ従来の加法型で利用でき、`-=` は要求しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/fenwick-tree.hpp"
int main() {
  blueberry::FenwickTree<int> bit(std::vector<int>{1, 2, 3, 4});
  bit.add(1, 5);
  assert(bit.sum(1, 3) == 10);
  assert(bit.pref(2) == 8);
  assert(bit.lower_bound(8) == 2);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `FenwickTree<T> bit(n)` | O(N) | [開く](#construct-size) |
| `FenwickTree<T> bit(values)` | O(N) | [開く](#construct-values) |
| `void bit.add(i, value)` | O(log N) | [開く](#add) |
| `T bit.prefix_sum(r)` | O(log N) | [開く](#prefix-sum) |
| `T bit.pref(r)` | O(log N) | [開く](#pref) |
| `T bit.sum(l, r)` | O(log N) | [開く](#sum) |
| `T bit.get(i)` | O(log N) | [開く](#get) |
| `int bit.size()` | O(1) | [開く](#size) |
| `int bit.lower_bound(target)` | O(log N) | [開く](#lower-bound) |

<details class="api-operation" id="construct-size" markdown="1">
<summary><code>FenwickTree&lt;T&gt; bit(n)</code> — O(N)</summary>

全要素を `T{}` として初期化します。

{% raw %}
```cpp
blueberry::FenwickTree<long long> bit(100);
```
{% endraw %}

注意点: `n` は0以上。`T` は値の加算・減算とデフォルト構築が必要です。

</details>

<details class="api-operation" id="construct-values" markdown="1">
<summary><code>FenwickTree&lt;T&gt; bit(values)</code> — O(N)</summary>

与えた配列を初期値として線形時間で構築します。

{% raw %}
```cpp
blueberry::FenwickTree<int> bit(std::vector<int>{2, 4, 1});
```
{% endraw %}

注意点: 入力vectorの後の変更は反映されません。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void bit.add(i, value)</code> — O(log N)</summary>

位置 `i` に `value` を加算します。

{% raw %}
```cpp
bit.add(2, 7);
```
{% endraw %}

注意点: `i` は `[0,N)`。任意の符号の値に使えますが、`lower_bound` を使う場合は累積値が単調になるよう非負値を保ちます。

</details>

<details class="api-operation" id="prefix-sum" markdown="1">
<summary><code>T bit.prefix_sum(r)</code> — O(log N)</summary>

半開区間 `[0,r)` の和を返します。

{% raw %}
```cpp
int prefix = bit.prefix_sum(3);
```
{% endraw %}

注意点: `r` は `[0,N]`。空prefixの値は `T{}` です。

</details>

<details class="api-operation" id="pref" markdown="1">
<summary><code>T bit.pref(r)</code> — O(log N)</summary>

`prefix_sum` の短い別名です。

{% raw %}
```cpp
int prefix = bit.pref(3);
```
{% endraw %}

注意点: 引数・返り値・境界は `prefix_sum` と同じです。

</details>

<details class="api-operation" id="sum" markdown="1">
<summary><code>T bit.sum(l, r)</code> — O(log N)</summary>

半開区間 `[l,r)` の和を返します。

{% raw %}
```cpp
int total = bit.sum(1, 4);
```
{% endraw %}

注意点: `0<=l<=r<=N`。内部では二つのprefix和の差を計算します。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>T bit.get(i)</code> — O(log N)</summary>

位置 `i` の現在値を返します。

{% raw %}
```cpp
int value = bit.get(1);
```
{% endraw %}

注意点: `i` は `[0,N)`。一点取得を大量に行う場合もこの計算量です。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int bit.size()</code> — O(1)</summary>

管理している要素数 $N$ を返します。

{% raw %}
```cpp
int n = bit.size();
```
{% endraw %}

注意点: 構築後に変化しません。

</details>

<details class="api-operation" id="lower-bound" markdown="1">
<summary><code>int bit.lower_bound(target)</code> — O(log N)</summary>

`prefix_sum(r) >= target` となる最小の `r` を返します。該当がなければ `N` です。

{% raw %}
```cpp
int first = bit.lower_bound(10);
```
{% endraw %}

注意点: 追加値が非負でprefix sumが単調非減少である必要があります。`target<=0` は0を返します。

</details>
