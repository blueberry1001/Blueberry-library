---
layout: category
title: Math — 数学
category: math
---

**Formal Power Series** は、ACLの `static_modint` と `convolution` を基盤に、
逆元・log・exp・sqrt・powなどのNTT-friendlyな形式的冪級数演算を提供します。
変換長と素数modulusの前提は個別ドキュメントを確認してください。

**Prime Sieve** は、比較的小さい上限以下の素数をまとめて前計算するときに使います。
大きな整数を数個だけ素数判定する用途には適しません。
modint・畳み込みなどACLにある基本機能はACLを利用し、その上に必要な機能を組み合わせる方針です。
