---
title: Library expansion batch four (2026-09-18)
---

## 追加内容

収録数を優先し、ACLにない9種類を追加。既存APIの変更・削除はありません。

| 分類 | 新規ライブラリ |
| --- | --- |
| Data Structure | Static Range Inversions / Static Range Mode |
| Graph | Rooted Tree Isomorphism / Clique Enumeration / Maximum Independent Set |
| Math | Multipoint Evaluation / Polynomial Interpolation / Sample Point Shift |
| String | Longest Common Substring |

カタログは66→75種類、一括入口を含む公開ヘッダは67→76、verify driverは96→105。
253問の公式問題スナップショットに対し、専用verify付き対応は85→94、未対応は158→149。
ACL推奨9、補助verifyのみ1は自作ライブラリ数に含めません。

## 設計と比較

- 区間転倒数と最頻値はクエリをまとめて渡すオフラインAPI。最頻値の返す添字は値を識別する代表位置であり、区間外の場合があります。
- 最大独立集合は40頂点以下の厳密解。木同型は完全な子ラベルを比較し、ハッシュ衝突がありません。
- 多項式演算は既存FPSとACL畳み込み、最長共通部分文字列はACL suffix array / LCPを利用。
- 全公開操作、空入力、サイズ上限、計算量、所有権を各ライブラリページに記載。

一次資料・独立実装した比較候補・同一入力の測定結果は
[Graph](graph-batch-four.md)、[Math](math-batch-four.md)、[Sequence](sequence-batch-four.md) に保存。
小規模・値域の狭い区間最頻値では愚直法が速く、その結果も記録しています。
外部実装や公開judgeの異なる環境の時間から最速性を主張しません。

## 検証記録

- 新規9 driverの公式160ケースを各1回、公式checkerで通過（Graph56、Math52、Sequence52）。
- `make check COMPILE_JOBS=3` を通過（Python45テスト、75文書・79例、76ヘッダ、105driver、29random各20seed）。
- Clang18 / gnu++23の全体compileと29本のrandom test各20seedを通過。
- 新規9ヘッダはGCC13 / Clang18 × gnu++20 / gnu++23のrelease単独・重複includeを通過。
- 新規3ランダムテストはClang ASan+UBSan、seed20260918を通過。
- 75ライブラリのAPI文書と79個のC++最小例がコンパイル・実行を通過。
- 実装担当とは別の担当による独立レビュー、統合側の差分・境界条件レビューを実施。

初期チェックではrange modeの公式問題ID誤記と文書の注意点ラベル漏れを検出し修正。
初回の失敗ログと成功した再開ログは別々に保存しています。
対象別公式記録のrevisionは開始時HEADのc383440ですが、未commitの追加ファイルに対する検証です。
このrevisionの既存コードの性能測定とは扱いません。ローカル全体make verifyは反復1回、CIは既定3回で区別します。
全体検証と公開処理の最終結果はPRのrequired checks、mainのActions、公開ページでも確認します。
