# Library Checker coverage expansion

ユーザーの追加依頼は、Library Checkerに収録された問題を中心に収録数を増やし、Fastest提出も参考にすること。既存の競技向けAPI・性能・説明・互換性方針と、commit/push/mergeの許可を引き継ぐ。

## 方針

今回は4分野12ヘッダを追加する。ACL標準機能の再実装ではなく、オンライン順序集合、永続性、ポテンシャル、グラフ構造、整数論、漸化式、文字列処理を補う。既存APIと旧snippetは変更しない。

大規模な動的木・永続lazy区間コピーを一度に増やす案より、公式問題で直接検証できる独立した部品を優先する。単に問題ごとの解答を収録する案より、境界と型の契約を持つ再利用可能なAPIを採る。

## 分野と契約

- Data Structure: Potential Union Find（群のポテンシャル制約と矛盾判定）、Ordered Set（オンライン挿入・削除・順位・k番目）、Persistent Segment Tree（任意の過去版から一点更新・半開区間積）。全域の型を狭めるFastestの前提は汎用APIへ持ち込まない。
- Graph: Hopcroft–Karp（最大二部matchingと最小vertex cover）、Biconnected Components（頂点blockとblock-cut forest）、Eulerian Trail（有向・無向、辺idの復元）。深いgraphでも再帰stackに依存しない。
- Math: Factorize（uint64 primalityと昇順素因数分解）、Mod Sqrt（素数mod、根がなければoptional空）、Linear Recurrence（BMの最短漸化式、ACL畳み込みによるBostan–Moriのk番目）。群・体・NTT・整数範囲を文書に明記。
- String: Aho–Corasick（固定文字種、add→build→next/count）、Eertree（オンライン文字追加、異なる回文と出現数）、Lyndon Factorization（Duval、半開区間境界）。Ahoは空文字rootを含め、Eertreeの特別頂点は-1/0、一般頂点は1から。Lyndonの空入力は境界列{0}。

## 実装・検証

C++20、header-only、blueberry名前空間。各APIに全操作のdetails、assert付き最小main、計算量・空入力・所有権・overflow等の契約を付ける。catalog/all.hpp/用途別案内は統合担当だけが変更する。

先に固定seedの小入力・愚直解比較・境界テストと公式verifyを作り、未実装で失敗することを確認してから実装する。GCC/Clang × C++20/23、release二重include、sanitizers、make test/verify/docs、Jekyll/site検査を実行する。担当外の独立レビューでAPI契約とコード品質を確認する。

Fastest上位の公開ソースはローカルの調査専用とし、コピーして配布しない。提出番号・日時・ソースhash・観察・採否を残す。候補の入力条件/I/O/メモリの違いを分析し、自作候補は同一入力・compiler・flags・環境で複数回計測する。異なる環境の順位をBlueberryの高速化の証拠にしない。

## 完了条件

12ヘッダをカタログから探して使用例をそのままコンパイルできる。対応する公式問題と独立random testを通過し、測定・レビュー結果が記録され、PR CIを通してmainへ統合しPagesへ反映する。問題の全網羅や全入力での最速は主張しない。
