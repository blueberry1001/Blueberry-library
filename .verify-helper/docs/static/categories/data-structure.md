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
| 未知のxで直線・線分の最小値 | Dynamic Li Chao Tree | 整数領域を指定。問い合わせ座標の事前登録不要 |
| 区間のk番目、値の頻度、前後要素 | Wavelet Matrix | 構築後の配列は変更しない |
| 点への加算、長方形の和 | Offline Fenwick Tree 2D | 更新座標を事前登録。問い合わせ境界は自由 |
| 未知の点への加算、長方形の和 | Dynamic Fenwick Tree 2D | 巨大な整数領域、オンライン。既知座標ならオフライン版を優先 |
| 巨大な添字の点加算・区間和 | Dynamic Fenwick Tree | 疎な整数配列。通常の小配列はACLを優先 |
| Union Findの操作を取り消す | Rollback Union Find | 保存した状態へ巻き戻す |
| 静的な区間min/max/gcd | Sparse Table | 結合的かつ冪等な演算 |
| 差分・行列などのポテンシャル制約 | Potential Union Find | 群の演算・逆元。非可換な演算も可 |
| 集合の順位・k番目・前後要素 | Ordered Set | オンライン挿入・削除。重複キーは1個 |
| 重複を含む集合・順位区間の集約 | Ordered Multiset | 同値キーを挿入順に保持、モノイド集約 |
| 列の挿入・削除・反転・区間作用 | Implicit Treap | 位置で管理する動的列。モノイドと作用を指定 |
| 過去版を保持した区間積 | Persistent Segment Tree | モノイド、一点更新。任意のversionから分岐 |

動的な木・永続lazy区間更新は未収録です。このページには検証・API文書を整備したヘッダを掲載し、
削除した旧スニペットの代替先は[移行ガイド]({{ '/migration.html' | relative_url }})に記載しています。
