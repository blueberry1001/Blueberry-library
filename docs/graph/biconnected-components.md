---
title: Biconnected Components
documentation_of: //blueberry/graph/biconnected-components.hpp
---

[カテゴリへ戻る]({{ '/categories/graph.html' | relative_url }}) · [共通ガイド]({{ '/guide.html' | relative_url }})

## 概要・前提

自己ループのない無向多重グラフを二重頂点連結成分（block）へ分解し、block-cut forestを構築します。
Nを頂点数、Mを入力辺数、Bをblock数とすると、構築時間・メモリは O(N+M) です。明示的なDFSスタックと辺スタックを使い、再帰は行いません。
ACLにはない機能です。既存の `LowLink::groups()` は橋を削除した**二辺連結成分の頂点分割**であり、本クラスとは異なります。
本クラスでは関節点が複数blockに現れます。既存LowLinkの非公開探索情報を変更せず、必要な辺スタックを持つ独立した探索で計算します。

`vector<pair<int,int>> edges` の要素は無向辺の両端です。N>=0、M<=INT_MAX、0<=u,v<N、u!=v を満たしてください。
多重辺を扱えます。親の**辺ID**だけを読み飛ばすので、親への別の辺は逆辺として処理されます。
孤立頂点は単一頂点block、橋は2頂点blockです。N=0なら辺列も空で、block列は空になります。
group内の頂点は重複しませんが、頂点順・block順の安定性や昇順は保証しません。頂点集合の総要素数は O(N+M) です。
入力を変更せず所有データを構築します。返したconst参照はオブジェクトの破棄・代入・移動で無効になり得ます。
必要なメモリを確保できることが前提です。block-cut forestを取得するときは N+B<=INT_MAX が必要です。

## 最小使用例

{% raw %}
```cpp
#include <cassert>
#include "blueberry/graph/biconnected-components.hpp"
int main() {
  blueberry::BiconnectedComponents blocks(4, {{0, 1}, {1, 2}, {2, 0}, {2, 3}});
  assert(blocks.groups().size() == 2);
  auto forest = blocks.block_cut_tree();
  assert(forest.size() == 6);
  assert(forest[2].size() == 2); // 関節点2は両blockに所属
  blueberry::BiconnectedComponents isolated(2, {});
  assert(isolated.groups().size() == 2);
}
```
{% endraw %}


## 操作一覧

| 呼び出し方 | 計算量 | 詳細 |
| --- | --- | --- |
| `BiconnectedComponents blocks(n, edges)` | O(N+M) | [開く](#construct) |
| `const vector<vector<int>>& blocks.groups() const` | O(1) | [開く](#groups) |
| `vector<vector<int>> blocks.block_cut_tree() const` | O(N+M) | [開く](#forest) |

<details class="api-operation" id="construct" markdown="1">
<summary><code>BiconnectedComponents blocks(n, edges)</code> — O(N+M)</summary>

頂点数intと無向辺列const参照からblockを構築します。子から戻ったときlow[child]>=order[parent]なら、その木辺までの辺スタックを1つのblockとして取り出します。

{% raw %}
```cpp
blueberry::BiconnectedComponents blocks(3, {{0, 1}, {0, 1}, {1, 2}});
```
{% endraw %}

注意点: 自己ループu==vは禁止です。多重辺は許可します。入力列を保持する参照は残さないため、一時的な辺列も渡せます。

</details>


<details class="api-operation" id="groups" markdown="1">
<summary><code>const vector&lt;vector&lt;int&gt;&gt;&amp; blocks.groups() const</code> — O(1)</summary>

blockごとの頂点集合をconst参照で返します。孤立頂点を含むすべての頂点が少なくとも1blockに所属します。関節点は複数blockに属します。

{% raw %}
```cpp
const auto& groups = blocks.groups();
for (const auto& vertices : groups) {
  assert(!vertices.empty());
}
```
{% endraw %}

注意点: 単一のcomponent(v)番号で表せる頂点分割ではありません。N=0なら空。参照はオブジェクトに従属し、コピーを作る場合は総要素数に比例する時間・メモリが必要です。

</details>


<details class="api-operation" id="forest" markdown="1">
<summary><code>vector&lt;vector&lt;int&gt;&gt; blocks.block_cut_tree() const</code> — O(N+M)</summary>

元頂点[0,N)とblock頂点[N,N+B)からなる無向二部forestを隣接リストで返します。元頂点vがgroups()[b]に属するときvとN+bを接続します。非関節点も葉として残す拡張されたblock-cut表現です。追加メモリO(N+M)。

{% raw %}
```cpp
auto forest = blocks.block_cut_tree();
int first_block_node = 3; // 上のn=3の例ではblock頂点が3から始まる
```
{% endraw %}

注意点: N+B<=INT_MAX。非連結入力なら複数の木なので、連結木を前提とするLCA/HLDへそのまま全体を渡さないでください。空入力なら空。返り値は独立した所有値です。

</details>

## 出典・検証

- [Library Checker: Biconnected Components](https://judge.yosupo.jp/problem/biconnected_components) で検証します（多重辺あり・自己ループなし）。
- [KACTL](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/BiconnectedComponents.h)、[Nyaan](https://nyaannyaan.github.io/library/graph/biconnected-components.hpp.html)、[公開提出287412](https://judge.yosupo.jp/submission/287412) のblock抽出・API・メモリ配置を調査しました。KACTLの橋を除外する辺callbackとは出力規約が異なります。
- Tarjanの標準的なlowlinkと辺スタックを独自に反復実装し、頂点の重複除去にはstamp配列を使います。公開コードのコピーは行っていません。
- 小グラフの全頂点部分集合について連結性と頂点除去後の連結性から極大blockを求める独立テスト、長いパス、多重辺、孤立点を検証します。比較条件は `benchmark/results/lc-expansion-graph/` を参照してください。
