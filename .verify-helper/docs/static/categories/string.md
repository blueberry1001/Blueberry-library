---
layout: category
title: String — 文字列
category: string
---

**Z Algorithm** は、文字列全体と各位置から始まる接尾辞が何文字一致するかを求めます。
文字列だけでなく整数列にも使えます。標準用途はACLのz_algorithmを優先してください。
suffix_array・lcp_arrayもACLを利用します。

| やりたいこと | 選ぶライブラリ | 入力・更新の条件 |
| --- | --- | --- |
| 各中心の最長回文 | Manacher | 静的な文字列・整数列 |
| 多数のパターンの出現数 | Aho–Corasick | パターンをすべて登録後にbuild。固定byte文字種 |
| 異なる回文の種類・出現数 | Eertree | 末尾へ文字追加。固定byte文字種 |
| 辞書順の非増加Lyndon分解 | Lyndon Factorization | `<`で比較できる静的な列 |

Aho–Corasickの登録順と、Eertreeの回文発見順にノードIDが定まり、公式問題の出力にも使えます。
