---
title: 汎用性を維持してFastestとの差を縮める方針
---

# 汎用性を維持してFastestとの差を縮める方針

**使いやすいAPIと汎用性を維持したまま、不要な仕事・メモリアクセス・入出力の費用を減らす。**
計算量が良い、公式verifyがAC、というだけでは性能の確認を終えない。
一方、Fastestの特殊な前提を既存APIへ黙って持ち込まない。
2026-09-19の調査ではStatic RMQの4提出を実行比較し、ほか4問題・12提出もソースを確認した。
ライブラリ本体や既存verifyの入出力は、この調査では変更していない。

## Static RMQ: 入出力の影響は大きいが、それだけではない

Library Checkerの表示は最大ケース時間であり、verifyの全ケース合計とは別の指標。
ユーザーから示された約120ms対20msという差を、そのままデータ構造部分の6倍差とは解釈しない。
高速提出は[362370](https://judge.yosupo.jp/submission/362370)、
[362362](https://judge.yosupo.jp/submission/362362)、
[400989](https://judge.yosupo.jp/submission/400989)と、調査時の最速
[402567](https://judge.yosupo.jp/submission/402567)を確認した。
提示3本はほぼ同じ16要素ブロック方式と高速I/Oを使う。
[ソースの具体的な分析](fastest-rmq-source-study.md)に、配列配置・SWAR・AVX2等の違いを記録した。

同じ公式入力、同じCPUコア、逐次実行、各条件1回のwarmupと5回の測定を行った。
全出力を公式期待出力と照合し、870サンプルと174回のwarmupを保存した。
既存4構造と独自試作2構造は、計測外でも公式27入力すべてで照合した（162組）。

環境はWSL2 / Core i7-14650HX / GCC 13.3、既存構造の以下の表は
`-std=gnu++20 -O2 -DNDEBUG`。入力はN=Q=500,000。
表は**ローカルのプロセス経過時間の中央値**で、Library Checkerの再提出結果ではない。
プロセス起動・終了・実際のファイル入出力も含む。

| Sqrt Treeの入出力のみを変更 | 最大ランダム入力 | 短区間入力 | 小さい値の入力 |
| --- | ---: | ---: | ---: |
| 同期を切ったcin/cout | 83.65 ms | 116.02 ms | 71.23 ms |
| portableなfread/fwriteのバッファ | 38.99 ms | 72.28 ms | 33.94 ms |

構造も問い合わせ順も変えず、最大ランダム入力では約2.15倍の差が出た。
従って、共通の高速I/Oは優先度が高い。ただしこの試作はRMQの整数範囲だけを扱う測定器で、
全整数型やEOFの契約を備えた公開FastIOライブラリとして完成しているわけではない。
採用する場合は符号付き最小値、64bit、文字列、pipe、末尾改行なし、buffer境界を含めて設計する。
通常のstdinを使えなくするmmap専用化や、桁数の制限は採用条件にしない。

元の高速提出を同じ最大ランダム入力で実行すると、順に16.51 / 15.80 / 16.15 / 14.15 msだった。
これらは元のGCC O3 pragmaやAVX2、RMQ専用処理、特殊I/Oを維持している。
同じマシンでも**コンパイラ設定とAPIの前提は一致していない**ので、残る約2〜3倍を
「汎用Sqrt Treeの実装だけの改善余地」と断定しない。
初回は402567のAVX2未指定でコンパイルに失敗し、そのログも保存した。
再実行ではこの提出だけ`-mavx2`を明示した。

### I/Oを除いた費用

診断用に入力を先に読み、構築→全クエリ→出力を分離した。
先読みは測定のためであり、公開APIへbatch必須の制約を加える提案ではない。
最大ランダム入力、同じGCC設定での中央値は次のとおり。

| 構造 | 構築 | 50万クエリ | プロセスpeak RSS |
| --- | ---: | ---: | ---: |
| Sparse Table | 7.56 ms | 3.89 ms | 46,180 KiB |
| Disjoint Sparse Table | 14.13 ms | 4.95 ms | 50,380 KiB |
| Sqrt Tree | 12.36 ms | 5.03 ms | 46,616 KiB |
| LinearRMQ | 5.79 ms | 14.55 ms | 18,500 KiB |
| 16要素ブロックの独自RMQ試作 | 3.01 ms | 3.17 ms | 18,972 KiB |
| 32要素maskの独自RMQ試作 | 5.41 ms | 5.79 ms | 16,172 KiB |

RSSには入力・クエリ・答えの配列、標準ライブラリ等も含み、構造単体のメモリではない。
構築とクエリの中央値を足した値は、全体時間の中央値と一致するとは限らない。
短区間や小さい値の入力、Clang 18、assert有効の結果も生ログに残した。
streamingと先読みではループ構成・cache状態も異なるため、単純な引き算で厳密なI/O費用を求めない。
例えば短区間のSqrt Treeはstreaming bufferedで72.28ms、先読みbufferedで42.70msだった。
中央値だけで数%の優劣を断定せず、全サンプルの幅も確認する。

ここで分かったのは、Sqrt Treeでは**構築の費用もクエリより大きい**こと、
LinearRMQでは今回の入力で**クエリの定数倍が大きい**こと。
O(1)やO(N)という記号だけでは実際の優先順位は決められない。
固定ブロック試作は差の原因を調べる対照群であり、任意monoidのSqrt Treeや
厳密な線形空間・左端argminを持つLinearRMQの置き換えとして採用したものではない。

## 今後の実装順序

| 優先 | 対象 | 最初に試すこと | 維持する保証／測定する軸 |
| --- | --- | --- | --- |
| 1 | 共通I/Oと性能測定 | portableなbuffered入出力、構造×I/Oの交差比較、構築と操作の分離 | 全数値範囲・EOF・pipe・標準C++経路。verifyを一括変更する前に代表問題で検証 |
| 2 | FastSet | 空→非空のときだけ上位へinsertを伝播、不要なcontains/書き込みの統合 | 任意の対応universe、全境界操作、密度・更新率・重複insert |
| 2 | Sqrt Tree / DST | 配列の不要なコピー・初期化を減らす、layer情報を事前計算、配置を比較 | 任意の結合演算、非可換、空区間の既存契約、汎用S型。sum/min/affine/matrixを分けて測定 |
| 2 | LinearRMQ | Θ(logN)内で幅を2冪に丸めて除算をshiftへ、同じ意味の比較を減らす | 厳密O(N)前処理・空間、カスタムCompare、左端tie。固定32化で保証を失わない |
| 3 | Wavelet Matrix | 値と元位置をsortしてrankを書き戻し、N回lower_boundを除く。rank周辺の配置を比較 | 重複・符号・任意順の全既存query。pair sortの追加メモリと型の要件も確認 |
| 3 | Static Range LIS | 再帰scratchの再利用、部分配列copy削減、既知rankの内部再圧縮回避 | 重複ありstrict LIS、空区間、任意順query、入力の型・所有権 |
| 4 | Link-Cut Tree | op/pull/access回数を数え、同じ不変条件を保つ重複更新を削減 | 非可換の両方向集約、不正link/cutの扱い、反転・連結性。assert有効/無効を別測定 |

これらは**調査から得た仮説と着手順であり、未測定の高速化を保証するものではない**。
特にvector初期化の削減で、これまで不要だったdefault constructorやtrivial型の制約を加えない。
scratch共有で再入性・thread safety・参照の寿命を変えない。型やopのテンプレートは通常inline化できるので、
測定せず「汎用テンプレートが遅い」として型や演算を固定しない。

[他4問題の詳しい調査](fastest-other-structures-study.md)では、最速のRange KthとLISに
全クエリ先読みが含まれること、LCTに加算専用化があることを確認した。
それらは差を説明する条件として記録し、今回の汎用API改善の採用項目にはしない。
SIMD必須・固定N・可換専用・unsigned専用・例外的な入力桁数も同様。

## 改善を採用する基準

1. **契約を固定する。** public API・対応型・代数条件・問い合わせ順・計算量・tie・境界・失敗時動作を列挙し、前後で一致させる。
2. **費用を分離する。** end-to-end、parse/format、構築、操作、メモリを記録する。Fastestは同等契約と条件の違う提出を分ける。
3. **一度に一つ変える。** コピー削減、配置、分岐、bufferサイズを別々に測る。allocation数、op回数、生成された除算・loadも必要に応じて確認する。
4. **同じ入力で複数回測る。** 公式最大ケースだけでなく短区間、単調、全同値、境界、N/Q比を変える。compiler/flags/CPU/input/source hashと全ログを保存する。
5. **全操作を検証する。** 公式checker・固定seed愚直比較・非可換・カスタム型・sanitizer・GCC/Clangを対象にし、速度のためにcorrectnessを緩めない。
6. **回帰も公開する。** 一つのmin問題だけ速く、別の主要な操作が大幅に遅くなる変更を一律改善として採用しない。メモリ増加・構築増加を含む採否理由を書く。

同等契約で数倍の差が残る場合は、ACだけで完了扱いにせず、支配項と次の測定課題を残す。
ただし同条件でも数%の差はCPU周波数やOS負荷に埋もれうる。微小差を追う際も、複雑化の対価を
再測定で確かめる。Fastestの更新を監視する仕組みや全ライブラリへの一律の速度保証はまだ追加していない。

## 再現用資料

- [RMQ生ログ・集計・環境](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/fastest-study/rmq-avx-retry)
- [初回AVX2未指定の失敗記録](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/fastest-study/rmq)
- [測定用コードと再現手順](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/fastest-study/harness)
- [4問題のFastest取得記録](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/fastest-study/other)

この資料は性能調査であり、試作を公開ライブラリへ昇格させる変更ではない。
その後の実装は上記の契約と測定を満たす小さい変更ごとに進める。
