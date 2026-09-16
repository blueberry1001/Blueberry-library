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
| 差分・行列などのポテンシャル制約 | Potential Union Find | 群の演算・逆元。非可換な演算も可 |
| 集合の順位・k番目・前後要素 | Ordered Set | オンライン挿入・削除。重複キーは1個 |
| 過去版を保持した区間積 | Persistent Segment Tree | モノイド、一点更新。任意のversionから分岐 |

動的な木・永続lazy区間更新は未収録です。このページには検証・API文書を整備したヘッダを掲載し、
旧スニペットの扱いと代替先はREADMEに記載しています。
