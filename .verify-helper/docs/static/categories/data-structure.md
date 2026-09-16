---
layout: category
title: Data Structure — データ構造
category: data-structure
---

巻き戻しが必要な連結性管理には **Rollback Union Find**、更新のない区間min/max/gcdには **Sparse Table** を使います。
区間和・一般の区間積・通常のUnion FindはACLを第一候補にしてください。

| やりたいこと | 選ぶライブラリ | 入力・更新の条件 |
| --- | --- | --- |
| 直線の最小値、線分の最小値 | Li Chao Tree | 問い合わせるx座標を事前登録 |
| 区間のk番目、値の頻度、前後要素 | Wavelet Matrix | 構築後の配列は変更しない |
| 点への加算、長方形の和 | Offline Fenwick Tree 2D | 更新座標を事前登録。問い合わせ境界は自由 |
| Union Findの操作を取り消す | Rollback Union Find | 保存した状態へ巻き戻す |
| 静的な区間min/max/gcd | Sparse Table | 結合的かつ冪等な演算 |

動的な木・永続データ構造などは今後の拡張対象です。旧構成のヘッダはトップページの自動生成一覧に残していますが、
このページには新構成で検証・API文書を整備したものを掲載しています。
