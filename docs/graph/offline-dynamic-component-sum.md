---
title: Offline Dynamic Component Sum
documentation_of: //blueberry/graph/offline-dynamic-component-sum.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }})

## 概要・前提

無向グラフの辺の追加・削除、頂点の値への加算、連結成分の値の和を扱います。
操作と問い合わせを順に記録し、`solve()` で問い合わせの答えをまとめて求めます。
`query(v)` の返り値は答えの配列に使う添字で、和そのものではありません。
ACLのDSUが扱わない辺の削除と、時系列の連結成分集約を補うオフラインの機能です。

Nは初期値配列の長さ、Kは記録済みの問い合わせ数です。
頂点番号は `[0,N)`、`N <= INT_MAX` を前提とします。
初期状態には辺がなく、各頂点の値は初期値配列の対応する要素です。
辺は向きを区別しない多重集合として扱い、`(u,v)` と `(v,u)` は同じ辺です。
同じ辺を2回追加した場合、1回削除しても接続は残ります。
自己ループも追加数と削除数を数えますが、連結成分やその和には影響しません。

Qを、成功した記録操作の総数とします。
`add_edge`・成功した `remove_edge`・`add_value`・`query` はそれぞれ1回と数え、
存在しない辺に対する `remove_edge` は含めません。`size`・`solve` も含めません。
内部の添字が収まるように、`Q <= INT_MAX / 4` を前提とします。
同じ辺の追加や自己ループの操作もこの上限に含まれます。
頂点数は構築後に変わりません。N=0では `size()` と `solve()` だけを呼べます。

### 値の型と数値範囲

Tはコピー構築・コピー代入ができ、`a + b` の結果をTへ暗黙に変換できる型です。
Tへ変換した値に対して、加算は結合的かつ可換である必要があります。
デフォルト構築、数値の0、`+=`、減算、等値比較、大小比較は要求しません。
各連結成分には少なくとも1個の初期値があるため、加算の単位元を渡す必要もありません。
以下の計算量はTのコピー・代入・加算がO(1)である場合です。
通常の浮動小数点加算は厳密には結合的ではなく、丸め誤差を含む結果はこの代数的保証の対象外です。

符号付き整数では、答えだけでなく、**内部で加算されるすべての部分和**がTの範囲に収まる必要があります。
辺の併合と加算の適用順は記録順とは異なり、後で相殺される正負の値も一時的に別々に集計されます。
記録順の頂点値と最終的な答えが収まるだけでは十分ではありません。
安全性を確認する十分条件は、全初期値と全 `add_value` の加算量について、
正の寄与の総和がTの最大値以下、負の寄与の総和がTの最小値以上となることです。
この確認自体も、より広い整数型などでオーバーフローせずに行ってください。
負の加算量は利用できますが、型Tの減算演算子は使いません。

### 記録と再実行

`solve()` は記録を消費せず、同じ記録に対して何度でも呼べます。
その後に操作を追加してから再び `solve()` すると、過去の問い合わせも含めて全件を計算します。
既に発行した問い合わせIDと、その時点の答えの意味は変わりません。
操作は末尾への追加だけで、過去の操作の挿入・変更や、問い合わせ直後のオンライン回答は行いません。
引数の値はコピーして保持し、呼び出し元の配列や加算量への参照を保存しません。

### 計算量とメモリ

構築はO(N)、記録の保持にO(N+Q)のメモリを使います。
`add_edge` は最悪O(log(Q+2))、成功した `remove_edge` は償却O(log(Q+2))、
`add_value` と `query` は償却O(1)です。
vectorの再確保を伴う1回の記録操作にはO(Q)かかる場合があります。
K>0のとき、1回の `solve()` は最悪O(N + Q log(Q+1) log(N+1))時間、
作業用メモリはO(N + Q log(Q+1))、返却配列はO(K)です。
K=0なら、更新の記録数にかかわらずO(1)時間・O(1)作業用メモリで空配列を返します。
N=0の構築もO(1)です。複数回の `solve()` の計算結果はキャッシュしません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <vector>
#include "blueberry/graph/offline-dynamic-component-sum.hpp"
int main() {
  blueberry::OfflineDynamicComponentSum<long long> graph({2, 3, 5});
  assert(graph.size() == 3);
  const int a = graph.query(0);
  graph.add_edge(0, 1);
  graph.add_edge(1, 0);  // 同じ無向辺を2本保持する。
  graph.add_value(1, 7);
  const int b = graph.query(0);
  const bool removed_once = graph.remove_edge(0, 1);
  const int c = graph.query(1);  // まだ0と1は連結している。
  const bool removed_twice = graph.remove_edge(1, 0);
  const int d = graph.query(1);
  const bool missing = graph.remove_edge(0, 1);
  assert(removed_once && removed_twice && !missing);
  assert(a == 0 && b == 1 && c == 2 && d == 3);
  const auto first = graph.solve();
  assert(first == std::vector<long long>({2, 12, 12, 10}));
  assert(graph.solve() == first);

  graph.add_edge(1, 2);
  const int e = graph.query(2);
  const auto second = graph.solve();
  assert(e == 4);
  assert(second == std::vector<long long>({2, 12, 12, 10, 15}));
  assert(first.size() == 4);  // 過去に受け取った配列は独立している。

  blueberry::OfflineDynamicComponentSum<long long> empty;
  assert(empty.size() == 0 && empty.solve().empty());
}
```
{% endraw %}

## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `OfflineDynamicComponentSum<T> graph(const vector<T>& values = {})` | O(N)、N=0ならO(1) | [開く](#construct) |
| `int graph.size() const` | O(1) | [開く](#size) |
| `void graph.add_edge(int u, int v)` | 最悪O(log(Q+2)) | [開く](#add-edge) |
| `bool graph.remove_edge(int u, int v)` | 成功時は償却O(log(Q+2))、失敗時は最悪O(log(Q+2)) | [開く](#remove-edge) |
| `void graph.add_value(int v, const T& delta)` | 償却O(1) | [開く](#add-value) |
| `int graph.query(int v)` | 償却O(1) | [開く](#query) |
| `vector<T> graph.solve() const` | 最悪O(N + Q log(Q+1) log(N+1))、K=0ならO(1) | [開く](#solve) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>OfflineDynamicComponentSum&lt;T&gt; graph(const vector&lt;T&gt;&amp; values = {})</code> — O(N)</summary>

長さNの初期値配列をコピーし、N個の孤立した頂点を持つ記録を作ります。
引数を省略するとN=0です。初期辺が必要なら、構築後に `add_edge` で記録します。

{% raw %}
```cpp
std::vector<long long> values{4, -2, 8};
blueberry::OfflineDynamicComponentSum<long long> graph(values);
values[0] = 100;  // graphの初期値4は変わらない。
```
{% endraw %}

注意点: `values.size() <= INT_MAX` が必要です。
元の配列への参照は保持しないため、構築後は元の配列を変更・破棄できます。
右辺値の配列を渡してもconst参照からコピーし、要素をムーブして消費するコンストラクタではありません。
Tのデフォルト構築や、N個の暗黙のゼロ初期値は使いません。
</details>

<details class="api-operation" id="size" markdown="1">
<summary><code>int graph.size() const</code> — O(1)</summary>

頂点数Nを返します。辺の数、連結成分数、記録数ではありません。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({2, 3, 5});
assert(graph.size() == 3);
```
{% endraw %}

注意点: 返り値はオブジェクトの構築後に変わらず、空の場合は0です。
記録の追加や答えの計算は行いません。
</details>

<details class="api-operation" id="add-edge" markdown="1">
<summary><code>void graph.add_edge(int u, int v)</code> — 最悪O(log(Q+2))</summary>

無向辺を1本追加する操作を記録します。
同じ端点の辺が既にあれば多重度を1増やし、端点の順序は区別しません。
後の問い合わせに対して、多重度が1以上である間は両端点を接続します。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({2, 3});
graph.add_edge(0, 1);
graph.add_edge(1, 0);
const bool removed = graph.remove_edge(0, 1);
const int id = graph.query(0);
assert(removed && graph.solve()[id] == 5);
```
{% endraw %}

注意点: 両端点とも `[0,N)` が必要です。自己ループも1本として記録・削除できます。
多重度が既に正でもQを1増やすため、記録数の上限に含まれます。
この操作だけでは連結成分の和を計算しません。端点の組をキーとした連想配列で多重度と開始位置を管理します。
</details>

<details class="api-operation" id="remove-edge" markdown="1">
<summary><code>bool graph.remove_edge(int u, int v)</code> — 成功時は償却O(log(Q+2))</summary>

指定した無向辺が存在すれば1本だけ削除し、`true` を返します。
多重度が0になるまで両端点の接続は残ります。
存在しなければ `false` を返し、記録・多重度・問い合わせIDを変更しません。
自己ループにも同じ追加数・削除数の規則を適用します。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({9});
graph.add_edge(0, 0);
const bool first = graph.remove_edge(0, 0);
const bool second = graph.remove_edge(0, 0);
const int id = graph.query(0);
assert(first && !second && graph.solve()[id] == 9);
```
{% endraw %}

注意点: 辺が存在しない場合も、両端点は `[0,N)` でなければなりません。
失敗時は最悪O(log(Q+2))、成功時は記録配列の再確保によりO(Q)かかる場合があります。
成功した場合だけQを1増やします。
</details>

<details class="api-operation" id="add-value" markdown="1">
<summary><code>void graph.add_value(int v, const T&amp; delta)</code> — 償却O(1)</summary>

頂点vの値に `delta` を加える操作を記録します。
加算の効果は、その後のすべての問い合わせで頂点vが属する連結成分に引き継がれます。
辺を削除して成分が分かれた後も、加算量は頂点vの側に残ります。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({2, 3});
graph.add_edge(0, 1);
graph.add_value(1, -1);
const bool removed = graph.remove_edge(0, 1);
const int id = graph.query(1);
assert(removed && graph.solve()[id] == 2);
```
{% endraw %}

注意点: `0 <= v < N` が必要です。代入や成分全体への加算ではありません。
`delta` をコピーして保持するので、呼び出し後に元の値を変更・破棄できます。
加算量の正負を判定したり、ゼロを比較して記録を省いたりしません。
記録配列の再確保時はO(Q)かかります。内部で順序を変えて計算する部分和にも、前述の数値範囲の条件が適用されます。
</details>

<details class="api-operation" id="query" markdown="1">
<summary><code>int graph.query(int v)</code> — 償却O(1)</summary>

この呼び出しより前に記録した更新を適用した状態で、頂点vの連結成分の和を求める問い合わせを記録します。
0から順に増える問い合わせIDを返します。答えは `solve()[id]` で取得します。
同じ頂点に連続して問い合わせても、それぞれ別のIDを発行します。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({4});
const int before = graph.query(0);
graph.add_value(0, 3);
const int after = graph.query(0);
const auto answers = graph.solve();
assert(answers[before] == 4 && answers[after] == 7);
```
{% endraw %}

注意点: `0 <= v < N` が必要です。返り値は和ではなく、同じ記録の答え配列に対する添字です。
後から更新・問い合わせを追加しても既存のIDは変わりません。
過去に取得した答え配列には、その取得後に発行したIDでアクセスできません。
記録配列の再確保時はO(Q)かかる場合があります。
</details>

<details class="api-operation" id="solve" markdown="1">
<summary><code>vector&lt;T&gt; graph.solve() const</code> — 最悪O(N + Q log(Q+1) log(N+1))</summary>

記録済みのK個の問い合わせに対し、ID順に並んだ長さKの答え配列を返します。
K=0なら、更新を記録していてもO(1)で空配列を返します。
辺・頂点値・問い合わせの記録を消費せず、呼び出すたびに全件を計算します。

{% raw %}
```cpp
blueberry::OfflineDynamicComponentSum<long long> graph({6});
graph.add_value(0, 2);
assert(graph.solve().empty());
const int first = graph.query(0);
const auto old_answers = graph.solve();
graph.add_value(0, 3);
const int second = graph.query(0);
const auto answers = graph.solve();
assert(first == 0 && second == 1);
assert(old_answers == std::vector<long long>({8}));
assert(answers == std::vector<long long>({8, 11}));
```
{% endraw %}

注意点: 返却配列は独立した値を所有し、オブジェクトへの参照を含みません。
後続の操作や `solve()` で過去の返却配列は変更・無効化されません。
Tのコピー・代入・加算に関する前提と、並べ替えられた途中の部分和の数値範囲を満たす必要があります。
前提違反をassertで検出するビルドでも、assertを無効にして制約外の入力を渡してよいわけではありません。

内部の時刻は更新の回数ではなく、問い合わせの位置 `[0,K)` に圧縮します。
各無向辺の多重度が正である区間と、各 `add_value` の効果が続く接尾区間 `[開始位置,K)` を、
時間方向のセグメント木に分配します。問い合わせを含まない区間は処理しません。
各節点の処理を連続配列にまとめ、
[Rollback Union Find]({{ '/blueberry/data-structure/rollback-union-find.hpp.html' | relative_url }})
と成分の以前の和を保存した履歴で、子から戻るたびに状態を復元します。
各区間は互いに重ならない節点区間に分解されるので、問い合わせの葉への経路には、
その時点で有効な各辺区間と各加算量がちょうど1回現れます。
辺を併合するときに成分和を足し、加算量をその頂点の現在の代表へ足すことで、
各葉では問い合わせ時点の成分和が得られます。適用順を変えられるのは加算の結合性と可換性によります。
子の処理後は成分和の保存先を逆順に戻してからUnion Findをrollbackし、兄弟の区間へ変更を持ち越しません。
復元は減算ではなく保存値の代入で行います。
最後の問い合わせより後の更新も記録には残るため、問い合わせを追加して再実行したときに反映されます。
union by sizeと経路圧縮をしない代表検索によるO(log(N+1))、
区間をO(log(K+1))個に分解する処理から、上記の全体計算量が得られます。
</details>

## 出典・検証

時間区間をセグメント木へ分配してrollbackする方法を、
[NyaanNyaanのOffline Dynamic Connectivity](https://github.com/NyaanNyaan/library/blob/master/graph/offline-dynamic-connectivity.hpp)と
[KACTLのUnion Find Rollback](https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/UnionFindRollback.h)で調査しました。
これらはアルゴリズムの調査資料で、実装コードは転用していません。
本実装では既存のRollback Union Findを再利用し、問い合わせ位置への時間圧縮と成分和の保存・復元を組み合わせています。

対応する公式問題は
[Library Checker: Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum)です。
[公式解](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/graph/dynamic_graph_vertex_add_component_sum/sol/correct.cpp)
はEuler Tour Treeを用いたオンラインの動的連結性を扱い、本APIとは処理契約が異なります。
公式問題だけでは多重辺・自己ループ・空入力・再実行・独自型の契約をすべて検証できないため、
これらは境界テストと小さいグラフの愚直な連結成分集計との比較で確認する対象です。
