---
title: 2026年10月3日のライブラリ追加・改善の統合状況
---

# 2026年10月3日のライブラリ追加・改善の統合状況

支配木・一般グラフ最大マッチング・整数凸包の追加、StaticTopTreeの説明と構築処理の改善、
DynamicFenwickTree／PersistentSegmentTreeの点取得高速化、最遠点対を、
ローカルブランチ `integration/library-2026-10-03` に統合した。
実装の統合コミットは `a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c`。
親は点取得改善の `2958f619020f6421870bba5173b6d7a3614e8be4` と、
最遠点対の `e7c4e0821f8419999f7624f521d78bca1acaa5ea` である。
元の履歴と各バッチの検証・測定記録を保持し、共有する生成済みcoverage一覧2ファイルを更新した。

統合版の全体 `make check` と追加の対象別検証は成功した。全件公式検証と公開は未完了である。
性能値と元バッチのテスト結果は各時点の記録として保持し、統合後の検証結果と分けて記載する。

## 追加・改善と元コミット

| 対象 | 採用した内容 | 元コミット |
| --- | --- | --- |
| 支配木・一般グラフ最大マッチング | ACLにないグラフ機能、文書、公式verify、乱択テストを追加 | [4568d4d](https://github.com/blueberry1001/Blueberry-library/commit/4568d4d02d36a2d235ede82e869d3774f009ea8d) |
| 整数凸包 | 符号付き整数座標、重複・共線点に対応する凸包を追加 | [0c07b13](https://github.com/blueberry1001/Blueberry-library/commit/0c07b13aba8d06f937720f21e09aec723923efec) |
| StaticTopTree | 総和から一次関数DPへ進む説明、結合順・型・コールバック・固定根の解説、完全な使用例2本。構築時の要素1個の不要処理を削減 | 文書 [5b9804a](https://github.com/blueberry1001/Blueberry-library/commit/5b9804abe294ab2dbeb9c04fcb797fdc18386e7e)、実装 [b733868](https://github.com/blueberry1001/Blueberry-library/commit/b733868d69fd7061e65d5c14e4b31167c90e4ae7)、記録 [ca8a4df](https://github.com/blueberry1001/Blueberry-library/commit/ca8a4df35269a6980cf0b9612878e420744fc860) |
| DynamicFenwickTree | `get` を局所セルからの差分にし、共通prefixのハッシュ表参照を削減 | `5182af3cdb4f05a24ae8978b0599d67c38f1eb70` |
| PersistentSegmentTree | `get` を指定版の根から葉への下降に変更。結合演算や内部ノード生成を行わない | `2958f619020f6421870bba5173b6d7a3614e8be4` |
| 最遠点対 | 凸包と面積による回転キャリパーで元の添字を返す。2点以下・全点同一では凸包構築を省略 | 初期保存 `da912761b09e4c4cf6d67d2c154c03672ca8658b`、完成バッチ `e7c4e0821f8419999f7624f521d78bca1acaa5ea` |

既存APIの型・代数条件・所有権・計算量を維持した。PersistentSegmentTreeの返り値のコピーは、
値型によってメモリを確保し得る。最遠点対は同距離の任意の組を返し、辞書順最小は保証しない。
比較したSuffixAutomatonの2案は安定した短縮を確認できず、不採用のままである。

## 元バッチで確認した性能

GCC 14／Clang 19、GNU C++20、`-O2` の同条件比較を使った。
assert有効・無効や異なる入力の結果は分けて記録し、元データと追加測定を混ぜていない。
各行は別々の実験であり、統合版を改めて測定した数値ではない。
StaticTopTreeは対象ヘッダが同一でも、過去の記録にACL・ツールチェーンとの完全な対応付けがないため参考値とする。
点取得の2実験は、変更しない他の構造をca8a4dfに揃えた対象単独の比較である。
最遠点対は対象ヘッダ・凸包・FastIO・測定プログラムの保存内容が統合版と一致するが、測定日時は統合前のままである。

| 対象 | 観測した結果 | 適用範囲・注意点 |
| --- | --- | --- |
| StaticTopTree | releaseの構築時間中央値が20条件で2.1〜28.4%短縮 | 10万頂点・木5種類・値型2種類。更新処理は変更していない。assert有効の悪化例と追加測定も保存 |
| DynamicFenwickTree | `get` 中央値が24条件で56.3〜91.5%、混合処理が29.9〜52.0%短縮 | 両方とも24条件すべてで標本範囲が分離。混合処理は更新4万回とget16万回 |
| PersistentSegmentTree | `get` 中央値が24条件で18.8〜59.3%短縮 | 標本範囲の分離は22/24条件。混合処理の微小差や未変更の区間取得の遅延例は明確な改善として扱わない |
| 最遠点対 | 最終実ヘッダ比較で全点同一が91.37〜96.85%、tinyが11.82〜27.87%短縮 | 全点同一は8/8条件、tinyは2/4条件で範囲分離。通常入力を含む33/76条件で中央値が増加し、すべて範囲が重なった |

共有ホスト・CPU非固定の測定であり、標本範囲の分離は統計的な信頼区間ではない。
最遠点対の全同一判定には最悪O(N)の追加走査があり、一般入力の一律な高速化は主張しない。
他の改善も、構築・更新・問い合わせ・I/O全体が同じ割合で速くなるという結果ではない。

## 元バッチの検証と統合後の検証

元バッチでは、支配木13・一般マッチング12・凸包25の公式ケースを各3回通過した。
StaticTopTreeは26ケースを各3回、点取得改善はDynamicFenwickTreeの21ケースと
PersistentSegmentTreeの14＋16ケースを各3回通過した。
点取得の公式ドライバは既存の更新・区間取得の回帰検査であり、`get` の直接検査は独立したローカルoracleで行った。
最遠点対の完成バッチは46ケースと依存する凸包25ケースを各3回、計213回通過した。
これらは各時点の対象別結果であり、統合版の全公式テスト成功として足し合わせない。

統合コミット `a2ed9204f4bcbaef8cb66423b7edd4fe5111c12c` に対する
[全体のmake check](../../benchmark/results/consolidated-library-2026-10-03/whole-check/report.json) は成功した。
単体テスト99件、ライブラリ文書107件の実行例112個、ヘッダ108個、公式verifyドライバ145本のコンパイル、
乱択テスト54本×20シードを確認した。所要時間は440.33秒。
検査対象462ファイルのハッシュと、既存の失敗した全件公式レポートが変わっていないことも確認した。

[追加の対象別検証](../../benchmark/results/consolidated-library-2026-10-03/focused/report.json) は185ステップ成功した。
include検査16件、複数APIを組み合わせた検査のコンパイル8件・実行8件、
Clang／GNU C++23のコンパイル7件・乱択実行140件、sanitizerのコンパイル3件・実行3件である。
これらはローカルの統合検査であり、公式全ケースを改めて実行したものではない。

文書生成・Jekyll・サイト事前検査の実行結果の保存先は
[docs/report.json](../../benchmark/results/consolidated-library-2026-10-03/docs/report.json) とする。
[履歴資料の再利用範囲](../../benchmark/results/consolidated-library-2026-10-03/evidence-reuse-audit.json)、
[ソース確認](../../benchmark/results/consolidated-library-2026-10-03/source-review.json)、
[証拠ファイルの対応表](../../benchmark/results/consolidated-library-2026-10-03/manifest.json) も同じディレクトリに保存する。

既存の全件公式検証は、AOJ `ALDS1_14_B` の公式データ取得がproxy CONNECT403で失敗したままである。
当時の142/144成功とLCM再試行成功による143/144の記録を保持し、元の全件コマンドを成功へ書き換えていない。
この143/144は当時の対象数であり、今回の統合版の成功件数ではない。
従来の `make docs` も、不完全な測定結果の公開用メトリクス出力を拒否していた。
Jekyll生成や `check_site.py --without-metrics` の成功だけで、公開ゲート通過とは扱わない。

## GitHubと公開状況

2026年10月3日09:26:36 UTCの読み取り専用確認では、
`improve/static-top-tree-docs-performance` は
[ca8a4df35269a6980cf0b9612878e420744fc860](https://github.com/blueberry1001/Blueberry-library/commit/ca8a4df35269a6980cf0b9612878e420744fc860)
としてGitHubへpush済みだった。支配木・一般マッチング・凸包もこの履歴に含まれる。
`feat/dominator-general-matching` の4568d4dもGitHub上に存在する。

代表構造の比較調査 `da421b2fdb3efdd051ad1da57d5478c31ce0f762` も未pushである。
未pushで保持する現時点の6コミットは、初期最遠点対da912761、比較調査da421b2、
DynamicFenwickTreeの5182af3、PersistentSegmentTreeの2958f61、最遠点対完成のe7c4e08、統合のa2ed920。
本報告のコミットはこの後に追加する。
点取得改善、最遠点対、今回の統合ブランチはローカルのみで、今回push・PR作成・mainへのmerge・deployは行っていない。
読み取り時点のopen PRは0件、mainは
[22784da0bad55f734e98c853c92bd9350f3ab4bd](https://github.com/blueberry1001/Blueberry-library/commit/22784da0bad55f734e98c853c92bd9350f3ab4bd)
のままで、今回の追加・改善はmainへ未統合である。
以前のGitHub Actions手動起動はtransport Forbiddenで拒否され、CIの開始を確認できていない。
GitHub上の作業ブランチの存在を、CI成功やGitHub Pages更新の証拠とはしない。

## 詳細記録

push済みの説明には変更されないコミット指定のリンクを使う。
未pushのバッチは上表の完全なSHAと、統合ツリー内の保存記録を対応付ける。

- [グラフ・凸包の統合検証記録（ca8a4df内）](https://github.com/blueberry1001/Blueberry-library/blob/ca8a4df35269a6980cf0b9612878e420744fc860/benchmark/results/repertoire-integration/validation.json)
- [StaticTopTreeのAPI説明・使用例（ca8a4df内）](https://github.com/blueberry1001/Blueberry-library/blob/ca8a4df35269a6980cf0b9612878e420744fc860/docs/graph/static-top-tree.md)
- [StaticTopTreeの説明・性能・不採用案（ca8a4df内）](https://github.com/blueberry1001/Blueberry-library/blob/ca8a4df35269a6980cf0b9612878e420744fc860/docs/development/static-top-tree-and-suffix-automaton.md)
- [DynamicFenwickTreeの最終コード比較](../../benchmark/results/point-access-performance/dynamic-fenwick-final-code-report.md)／[検証manifest](../../benchmark/results/point-access-validation/dynamic-fenwick/validation.json)
- [PersistentSegmentTreeの最終コード比較](../../benchmark/results/point-access-performance/persistent-segment-final-code-report.md)／[検証manifest](../../benchmark/results/point-access-validation/persistent-segment/validation.json)
- [最遠点対の比較・採否・検証](furthest-pair.md)／[検証manifest](../../benchmark/results/furthest-pair-resume/validation/manifest.json)
