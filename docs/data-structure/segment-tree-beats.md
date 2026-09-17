---
title: Segment Tree Beats
documentation_of: //blueberry/data-structure/segment-tree-beats.hpp
---

[カテゴリへ戻る]({{ '/categories/data-structure.html' | relative_url }})

## 概要・前提

区間の上限制限・下限制限・加算と、和・最小値・最大値を扱います。単純な遅延作用として表せないchmin/chmaxを、最大・第2最大・最小・第2最小の情報で処理します。ACLの通常のlazy_segtreeとは対象が異なります。型は `blueberry::SegmentTreeBeats`、値・和の公開型はlong longです。

Nは配列長、Qは更新・問合せの総数です。0<=N<=INT_MAX/4。添字は0-index、区間は `[l,r)`。構築値、set/chmin/chmaxの引数、各更新後の要素は厳密に `-2^60 < x < 2^60` としてください。addの引数はlong longで、更新後の全要素がこの範囲に収まる必要があります。prod/all_prodで返す和はlong longに収まることが前提です。空の和は0、空の最小・最大はnulloptです。

内部はGNU拡張の符号付き128 bitを使い、clampで値を戻しても蓄積する遅延加算や、一時的に大きくなる子の値を保持します。128 bitの中間値もoverflowしてはいけません（実用上十分広い保守的条件として、これまでのadd引数の絶対値総和が2^80未満なら安全です）。最終値だけが小さい場合でも通常の64 bit lazyはoverflowし得るため、128 bitは意図的な選択です。

構築O(N)、メモリO(N)、再帰スタックO(log(N+1))。GCC/Clangの代表的な64 bit環境ではノード112 byteを最大4N個確保します。chmin/chmaxは混合更新列全体でO((N+Q)log²(N+1))、更新1回の最悪O(N)です。add/set/get/prod/min/maxは最悪O(log(N+1))、all_prod/sizeはO(1)。get等の問合せも遅延伝搬のためnonconstです。

vectorで値を所有し、入力vectorの寿命に依存しません。返り値は値で、内部参照・iteratorは公開しません。移動元は長さ0となり、構築した値の代入で再利用できます。不正な添字・範囲はassertの前提条件であり、releaseでも許可されるわけではありません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/data-structure/segment-tree-beats.hpp"
int main() {
  blueberry::SegmentTreeBeats tree(std::vector<long long>{1, 5, -2});
  tree.chmin(0, 3, 3); // {1,3,-2}
  tree.chmax(0, 3, 0); // {1,3,0}
  tree.add(1, 3, 2);   // {1,5,2}
  assert(tree.prod(0, 3) == 8);
  assert(tree.min(0, 3).value() == 1);
  assert(tree.max(0, 3).value() == 5);
  assert(!tree.min(1, 1));
  tree.set(0, 4);
  assert(tree.get(0) == 4);
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `SegmentTreeBeats(const vector<long long>& values = {})` | O(N) | [開く](#construct) |
| `SegmentTreeBeats(const SegmentTreeBeats& other)` | O(N) | [開く](#copy-construct) |
| `SegmentTreeBeats& operator=(const SegmentTreeBeats& other)` | O(N + Ndst) | [開く](#copy-assign) |
| `SegmentTreeBeats(SegmentTreeBeats&& other)` | O(1) | [開く](#move-construct) |
| `SegmentTreeBeats& operator=(SegmentTreeBeats&& other)` | O(Ndst) | [開く](#move-assign) |
| `~SegmentTreeBeats()` | O(N) | [開く](#destruct) |
| `int tree.size() const` | O(1) | [開く](#size) |
| `void tree.chmin(int l, int r, long long x)` | 償却 O(log²(N+1)) | [開く](#chmin) |
| `void tree.chmax(int l, int r, long long x)` | 償却 O(log²(N+1)) | [開く](#chmax) |
| `void tree.add(int l, int r, long long x)` | O(log(N+1)) | [開く](#add) |
| `long long tree.prod(int l, int r)` | O(log(N+1)) | [開く](#prod) |
| `optional<long long> tree.min(int l, int r)` | O(log(N+1)) | [開く](#min) |
| `optional<long long> tree.max(int l, int r)` | O(log(N+1)) | [開く](#max) |
| `long long tree.all_prod() const` | O(1) | [開く](#all-prod) |
| `long long tree.get(int p)` | O(log(N+1)) | [開く](#get) |
| `void tree.set(int p, long long x)` | O(log(N+1)) | [開く](#set) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>SegmentTreeBeats(const vector&lt;long long&gt;&amp; values = {})</code> — O(N)</summary>

valuesをコピーして構築します。省略時は長さ0です。

{% raw %}
```cpp
blueberry::SegmentTreeBeats tree(std::vector<long long>{1, 2, 3});
```
{% endraw %}

注意点: 長さ・数値の上限は概要の通りです。

</details>

<details class="api-operation" id="copy-construct" markdown="1">
<summary><code>SegmentTreeBeats(const SegmentTreeBeats&amp; other)</code> — O(N)</summary>

格納値・集約を独立に複製します。以降の更新は相互に影響しません。

{% raw %}
```cpp
auto copy = tree;
```
{% endraw %}

注意点: 各ノードの保持情報もコピーします。コピー元は変更しません。

</details>

<details class="api-operation" id="copy-assign" markdown="1">
<summary><code>SegmentTreeBeats&amp; operator=(const SegmentTreeBeats&amp; other)</code> — O(N + Ndst)</summary>

既存内容を破棄して独立コピーに置き換え、自身への参照を返します。

{% raw %}
```cpp
auto copy = tree;
copy = tree;
```
{% endraw %}

注意点: 自己代入も可能。Ndstは代入前の宛先の要素数です。

</details>

<details class="api-operation" id="move-construct" markdown="1">
<summary><code>SegmentTreeBeats(SegmentTreeBeats&amp;&amp; other)</code> — O(1)</summary>

格納領域の所有権を移動します。移動元は空となり再利用できます。

{% raw %}
```cpp
auto moved = std::move(tree);
```
{% endraw %}

注意点: 内部参照は公開しません。std::moveには<utility>をincludeします。

</details>

<details class="api-operation" id="move-assign" markdown="1">
<summary><code>SegmentTreeBeats&amp; operator=(SegmentTreeBeats&amp;&amp; other)</code> — O(Ndst)</summary>

宛先の内容を破棄して所有権を移動し、自身への参照を返します。移動元は空です。

{% raw %}
```cpp
auto moved = tree;
moved = std::move(tree);
```
{% endraw %}

注意点: 自己move代入は何もしません。Ndstは代入前の宛先の要素数です。

</details>

<details class="api-operation" id="destruct" markdown="1">
<summary><code>~SegmentTreeBeats()</code> — O(N)</summary>

所有する値と領域をすべて破棄します。

{% raw %}
```cpp
{ auto copy = tree; } // スコープ末尾で破棄
```
{% endraw %}

注意点: 通常は明示呼出ししません。共有所有権や外部バッファへの依存はありません。

</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int tree.size() const</code> — O(1)</summary>

配列長を返します。

{% raw %}
```cpp
assert(tree.size() >= 0);
```
{% endraw %}

注意点: 更新では長さは変わりません。

</details>

<details class="api-operation" id="chmin" markdown="1">
<summary><code>void tree.chmin(int l, int r, long long x)</code> — 償却 O(log²(N+1))</summary>

区間内の各値をmin(a[i],x)に置換します。

{% raw %}
```cpp
tree.chmin(0, tree.size(), 3);
```
{% endraw %}

注意点: 空区間は何もしません。xは厳密に±2^60の内側。 単一呼出しの最悪O(N)。

</details>

<details class="api-operation" id="chmax" markdown="1">
<summary><code>void tree.chmax(int l, int r, long long x)</code> — 償却 O(log²(N+1))</summary>

区間内の各値をmax(a[i],x)に置換します。

{% raw %}
```cpp
tree.chmax(0, tree.size(), 0);
```
{% endraw %}

注意点: 空区間は何もしません。xは厳密に±2^60の内側。 単一呼出しの最悪O(N)。

</details>

<details class="api-operation" id="add" markdown="1">
<summary><code>void tree.add(int l, int r, long long x)</code> — O(log(N+1))</summary>

区間内の全要素へxを加算します。

{% raw %}
```cpp
tree.add(0, tree.size(), 1);
```
{% endraw %}

注意点: 空区間は無変更。更新後の要素の範囲と内部中間値の条件を守ってください。

</details>

<details class="api-operation" id="prod" markdown="1">
<summary><code>long long tree.prod(int l, int r)</code> — O(log(N+1))</summary>

区間和を返します。

{% raw %}
```cpp
assert(tree.prod(0, 0) == 0);
```
{% endraw %}

注意点: 返す和がlong longに収まること。遅延伝搬を行うためnonconstです。

</details>

<details class="api-operation" id="min" markdown="1">
<summary><code>optional&lt;long long&gt; tree.min(int l, int r)</code> — O(log(N+1))</summary>

区間最小値。空ならnulloptを返します。

{% raw %}
```cpp
assert(!tree.min(0, 0));
```
{% endraw %}

注意点: 非空なら.value()で取得できます。nonconstです。

</details>

<details class="api-operation" id="max" markdown="1">
<summary><code>optional&lt;long long&gt; tree.max(int l, int r)</code> — O(log(N+1))</summary>

区間最大値。空ならnulloptを返します。

{% raw %}
```cpp
assert(!tree.max(0, 0));
```
{% endraw %}

注意点: 非空なら.value()で取得できます。nonconstです。

</details>

<details class="api-operation" id="all-prod" markdown="1">
<summary><code>long long tree.all_prod() const</code> — O(1)</summary>

全体の和を返します。長さ0なら0です。

{% raw %}
```cpp
auto total = tree.all_prod();
(void)total;
```
{% endraw %}

注意点: 返す和がlong longに収まること。

</details>

<details class="api-operation" id="get" markdown="1">
<summary><code>long long tree.get(int p)</code> — O(log(N+1))</summary>

位置pの値を返します。

{% raw %}
```cpp
if (tree.size()) { auto x = tree.get(0); (void)x; }
```
{% endraw %}

注意点: 0<=p<N。値で返し、遅延伝搬を行います。

</details>

<details class="api-operation" id="set" markdown="1">
<summary><code>void tree.set(int p, long long x)</code> — O(log(N+1))</summary>

位置pの値をxに置換します。

{% raw %}
```cpp
if (tree.size()) tree.set(0, 5);
```
{% endraw %}

注意点: 0<=p<N。xは厳密に±2^60の内側。

</details>

## 出典・検証

[公式Range Chmin Chmax Add Range Sum](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum)に対応。固定seedのvector愚直解で全操作・空区間・負値・コピー/移動・大きな遅延加算を確認します。[NyaanのBeats](https://nyaannyaan.github.io/library/segment-tree/segment-tree-beats.hpp.html)と[公開上位提出204482](https://judge.yosupo.jp/submission/204482)を設計比較に用い、標準的な第2極値のアルゴリズムを独立実装しています。公開ソースは転記していません。
