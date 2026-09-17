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

| やりたいこと | 選ぶライブラリ | 主な前提 |
| --- | --- | --- |
| 64bit整数の素数判定・素因数分解 | Factorize | `uint64_t`、分解する数は1以上 |
| 素数を法とした平方根 | Modular Square Root | 素数modがINT_MAX以下。根がなければoptional空 |
| 数列から最短の線形漸化式を求める | Linear Recurrence: Berlekamp–Massey | 体の演算、O(N²) |
| 線形漸化式の遠い項を求める | Linear Recurrence: Bostan–Mori | ACL static_modint、NTT可能な変換長 |
| 分数の厳密な比較・四則演算 | Fraction | 既約な分子・正の分母がint64に収まること |

modint・畳み込みなどACLにある基本機能はACLを利用し、その上に必要な機能を組み合わせる方針です。
