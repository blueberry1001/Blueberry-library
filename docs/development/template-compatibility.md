---
title: Contest Templateの呼び出し互換性と依存整理
---

## 変更範囲

2026-10-20、基準revision `f9ccb0f704ee47974b372062684b4295bdc0ebea`。
保存原本`assets/code/blueberrytemplate.h`の有効な呼び出し方と、2種類の互換ヘッダを比較した。

- `using namespace std/atcoder`、vector/deque/pairのstream演算子を提供する。入れ子、独自allocator、`vector<bool>`も確認した。
- 可変長・vector版の`min/max`、旧ヘルパー、組合せ・篩の公開テーブル、`UnionFind`、`Rande::mt`、変更可能な`dx/dy`、旧マクロを復元した。
- `deb`は`BLUEBERRY`定義時に`O`へ展開し、未定義時には引数も評価しない。
- 原本の3引数`max/min`には標準の比較関数版との競合があったため修正した。数値へ変換できる比較関数も標準版へ渡す。
- 基本ヘルパーを`internal/contest-helpers-base.hpp`へ分離した。両テンプレートは`factorize.hpp`、`combinations.hpp`、`prime-sieve.hpp`、`topological-sort.hpp`に依存しない。単体`fast-io.hpp`の自作依存は自身だけである。
- 旧ヘルパーの`soinsu_bunkai`は原本と同じ試し割りの計算量を保つ。高速な64bit素因数分解が必要なら、独立した`factorize.hpp`を明示的に使用する。
- I/O・互換テンプレート関連の説明コメント、不要なassert・空白・空行を削除した。継続する開発方針は公開使用ガイドとは別に、`AGENTS.md`と`IMPLEMENTATION_POLICY.md`へ記録した。
- `all.hpp`を後からincludeする場合の`bit`マクロ衝突は、Static Range Inversions内部の変数名だけを変更して解消した。

`main`・`_main`の定義と`quick_exit`による自動終了はヘッダへ持ち込まず、利用者が`main`を書く。
原本の保存データは変更していない。64bitの`popc`、128bitの算術級数、strictな`parse128`など、既存の安全な拡張も維持した。
`parsetoint128`は原本どおり数字以外を無視する別関数である。
高速版の`scan/print`と標準streamは同じstdin/stdout上で混在させない。

## コード量と生成コード

自作ヘッダのincludeを再帰的に辿り、重複を除いたファイル全文をLFに正規化して集計した。
標準ライブラリ・ACLの本文と、`oj-bundle`の生成テキスト全体のサイズは含まない。

| 入口 | 変更前 bytes / 行 | 変更後 bytes / 行 |
| --- | ---: | ---: |
| Fast I/O | 13,265 / 351 | 10,936 / 322 |
| Contest Template | 19,243 / 524 | 11,924 / 264 |
| Contest Fast I/O | 34,218 / 914 | 24,250 / 616 |

GCC 13.3 / Clang 18.1、`gnu++20 -O2 -DNDEBUG -S`、同一のMany A+B用プログラムで、単体Fast I/Oの変更前後を比較した。
64bit・128bitの全4構成で生成assemblyがバイト単位で一致した。今回の簡潔化を実行速度の改善率として扱っていない。
比較コード・コマンド・ハッシュ・依存内訳は[記録](measurements/template-compatibility/README.md)に残した。

## 検証

専用の互換性テストはGCC / Clang、GNU C++20 / C++23、両テンプレート、`BLUEBERRY`定義有無の16構成を用いる。
標準stream、`scan/print`、副作用を持つ`deb`、異種型・比較関数付き`max/min`、旧ヘルパー・公開変数を確認する。
固定seed `71549`で、Union-Find、閉路を含むtopological sort、篩、素因数分解・約数列挙を愚直解と比較する。
さらに両テンプレートと`all.hpp`のinclude順8構成、I/Oの依存閉包、UBSanを確認した。

初回`make check`では152 unit testが成功した後、依存の変更によるLibrary Checker対応表の古さを検出した。
生成データを更新し、失敗ログを保持して全体検証を再実行した。

| 確認 | 結果 |
| --- | --- |
| `make check`、GCC GNU20 | 152 unit test、123文書の124 C++例、123ヘッダ、167 verifyコード、66 random test×20 seeds成功 |
| `CXXFLAGS=-DNDEBUG make include-test` | 123ヘッダ成功、ACL由来の警告はisystemで分離 |
| `make verify`、I/Oの6ドライバ、3回反復 | 64bit各9ケース・128bit各10ケース、すべてAC |
| `make docs` | 成功 |
| production Jekyll、`check_site.py --without-metrics` | 5カテゴリ・122 APIページ、footer/navigation確認成功 |

ローカル検証は基準revisionへ変更ファイルを重ねた専用cloneで実行した。
生のverifyレポートのrevisionはcloneのHEADを示し、未変更の基準ソースを検証したという意味ではない。
実装commit `caa2911364baddf684b33076f85d6b486068b718`と検証コピーの変更C++/テスト12ファイルの一致は、`validation-sources.json`へhashで記録した。
公式verifyのログ・反復ごとのJSONをそのまま残し、別revisionの比較測定や全167ドライバの実行結果としては扱わない。
