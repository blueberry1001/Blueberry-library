---
title: Hopcroft–Karp
documentation_of: //blueberry/graph/hopcroft-karp.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

左右の頂点集合が既知の二部グラフについて、最大マッチングと最小頂点被覆を求めます。
ACLの `mf_graph` でも解けますが、本実装は残余容量グラフやsource/sinkを組み立てず、左右の対応と被覆を直接取得する専用APIです。
左頂点数を L、右頂点数を R、入力辺数を M、V=L+R とします。構築 O((V+M)√V)、メモリ O(V+M) です。
V=0 の構築は O(1) です。CSR隣接配列とBFS・明示的な増加路スタックを使い、深い増加路でもC++の再帰スタックを消費しません。

入力辺は `vector<pair<int,int>>` で `(左番号, 右番号)` を渡します。0<=L,R、L+R<INT_MAX、各辺は 0<=u<L、0<=v<R を満たしてください。
多重辺・孤立頂点・空の頂点集合を扱えます。多重辺は同じ頂点対を繰り返し指定したものとして扱い、辺数Mに数えます。
左右は別の集合なので、番号が同じ `(u,u)` も通常の二部辺です。入力を変更せず構築時に解き、後から辺を追加する操作はありません。
メモリが確保可能であることが前提です。頂点数とマッチ数はint、隣接配列の添字はsize_tで管理します。
最適解が複数ある場合の対応・被覆の選び方は保証しません。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/graph/hopcroft-karp.hpp"
int main() {
  std::vector<std::pair<int, int>> edges{{0, 0}, {0, 1}, {1, 0}};
  blueberry::HopcroftKarp matching(2, 2, edges);
  assert(matching.size() == 2);
  assert(matching.left_match(0) == 1);
  assert(matching.right_match(0) == 1);
  assert(matching.pairs().size() == 2);
  auto [left, right] = matching.min_vertex_cover();
  assert(left.size() + right.size() == 2);
}
```
{% endraw %}


## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `HopcroftKarp matching(L, R, edges)` | O((V+M)√V) | [開く](#construct) |
| `int matching.size() const` | O(1) | [開く](#size) |
| `int matching.left_match(u) const` | O(1) | [開く](#left-match) |
| `int matching.right_match(v) const` | O(1) | [開く](#right-match) |
| `vector<pair<int,int>> matching.pairs() const` | O(L) | [開く](#pairs) |
| `pair<vector<int>,vector<int>> matching.min_vertex_cover() const` | O(V+M) | [開く](#cover) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>HopcroftKarp matching(L, R, edges)</code> — O((V+M)√V)</summary>

左右のサイズintと辺列const参照から最大マッチングを計算します。通常のHopcroft–Karpの最短増加路の層をBFSで作り、互いに頂点素な経路を反復DFSで増加させます。

{% raw %}
```cpp
blueberry::HopcroftKarp matching(2, 3, {{0, 1}, {1, 2}});
```
{% endraw %}

注意点: L,R>=0、L+R<INT_MAX。辺の左右番号はそれぞれ範囲内である必要があります。空辺列ならマッチ数0。入力の寿命に依存しません。

</details>


<details class="api-operation" id="size" markdown="1">
<summary><code>int matching.size() const</code> — O(1)</summary>

最大マッチングの辺数を返します。頂点数ではありません。返り値は0以上min(L,R)以下です。

{% raw %}
```cpp
int matched = matching.size();
```
{% endraw %}

注意点: 左右のどちらかが空、または辺がなければ0。

</details>


<details class="api-operation" id="left-match" markdown="1">
<summary><code>int matching.left_match(u) const</code> — O(1)</summary>

左頂点uに対応する右頂点番号を返します。未使用なら-1です。

{% raw %}
```cpp
int right = matching.left_match(0);
```
{% endraw %}

注意点: 0<=u<L。右頂点番号が0でも有効なマッチなので、未使用判定は-1と比較してください。

</details>


<details class="api-operation" id="right-match" markdown="1">
<summary><code>int matching.right_match(v) const</code> — O(1)</summary>

右頂点vに対応する左頂点番号を返します。未使用なら-1です。

{% raw %}
```cpp
int left = matching.right_match(0);
```
{% endraw %}

注意点: 0<=v<R。マッチする両側の取得結果は互いに対応します。

</details>


<details class="api-operation" id="pairs" markdown="1">
<summary><code>vector&lt;pair&lt;int,int&gt;&gt; matching.pairs() const</code> — O(L)</summary>

選んだ辺を(左番号,右番号)で返します。左番号昇順で、同一頂点は二度現れません。追加メモリはO(K)、K=size()です。

{% raw %}
```cpp
auto chosen_edges = matching.pairs();
```
{% endraw %}

注意点: 返り値は独立した所有値です。空マッチングなら空vector。多重辺の入力辺IDを区別するAPIではありません。

</details>


<details class="api-operation" id="cover" markdown="1">
<summary><code>pair&lt;vector&lt;int&gt;,vector&lt;int&gt;&gt; matching.min_vertex_cover() const</code> — O(V+M)</summary>

全辺の少なくとも片端を含む最小頂点被覆を左右別に返します。firstが左番号、secondが右番号で、それぞれ昇順です。要素数合計はsize()に等しくなります。未マッチ左頂点から交互路で到達する集合Zを求め、左の非到達頂点と右の到達頂点を選びます。追加メモリO(V)。

{% raw %}
```cpp
auto [cover_left, cover_right] = matching.min_vertex_cover();
```
{% endraw %}

注意点: 呼ぶたびに到達性を計算します。返り値は所有値で、マッチングを変更しません。空辺列なら両方空。

</details>

## 出典・検証

- [Library Checker: Matching on Bipartite Graph](https://judge.yosupo.jp/problem/bipartitematching) で最大マッチングを検証。
- [KACTL HopcroftKarp](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/HopcroftKarp.h) と [hitonanode](https://hitonanode.github.io/cplib-cpp/graph/bipartite_matching.hpp.html) のAPI・層別探索を調査し、反復探索で独自実装しました。
- [Fastest調査対象396318](https://judge.yosupo.jp/submission/396318) のCSRやI/Oを分析しました。提出の名称だけで標準HKの計算量を仮定せず、層別増加路方式を採用しています。
- 小入力では独立した部分集合DPでマッチ数を比較し、被覆のサイズと全辺の被覆も確認します。長い増加路を含むテストを `tests/random/graph-expansion.cpp` に収録しています。
- 公開コードのコピーは行っていません。比較条件・観測は `benchmark/results/lc-expansion-graph/` を参照してください。
