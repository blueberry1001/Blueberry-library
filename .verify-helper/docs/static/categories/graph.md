---
layout: category
title: Graph — グラフ・木
category: graph
---

**Dijkstra** は非負重みの最短路、**LCA** は静的な木の祖先・辺数距離のクエリに使います。
`LowestCommonAncestorRMQ` はLCAを O(1) で返し、**Heavy-Light Decomposition** は部分木・パス・頂点を
ACLのsegtree/lazy_segtreeへ分解します。Dijkstraには負辺を渡せません。LCA/HLDは連結な木を前提とし、
構築後の辺の追加・削除には対応しません。
強連結成分・最大流・最小費用流・2-SATはACLを利用します。
