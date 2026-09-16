---
layout: category
title: Graph — グラフ・木
category: graph
---

**Dijkstra** は非負重みの最短路、**LCA** は静的な木の祖先・辺数距離、
**Heavy Light Decomposition** は頂点・辺のパスや部分木を配列区間へ変換する用途に使います。
Dijkstraには負辺を渡せません。LCAとHLDは連結な木を前提とし、構築後の辺の追加・削除には対応しません。
強連結成分・最大流・最小費用流・2-SATはACLを利用します。

| やりたいこと | 選ぶライブラリ | 入力・更新の条件 |
| --- | --- | --- |
| 最短距離と経路復元 | Dijkstra | 非負重み |
| 共通祖先・k個上の祖先・距離 | Lowest Common Ancestor | 静的な木。大量のLCAにはRMQ版も選べる |
| 頂点・辺パス、部分木の区間クエリ | Heavy Light Decomposition | 静的な木。ACLのsegtree等を組み合わせる |
| 橋・関節点・二辺連結成分 | Low Link | 無向グラフ。多重辺・自己ループも扱える |
| 各頂点を根にした木DP | Rerooting DP | 木。結合順序と頂点・辺の変換を指定する |
