# 操作比較・ライブラリ拡充の検証記録

## 変更範囲

Issue #24〜#30に分けた実装を統合した。22ヘッダを追加し、操作検索を64候補へ拡充。
比較は9組にまとめ、候補からのリンクで必要な折り畳みだけを開く。
個別APIページでも同じ比較データを使用する。

Disjoint Sparse Table自体は今回変更していない。更新・クエリ回数を含む比較を追加した。
Static Top TreeとDynamic Top Tree、heap版と有限定義域のsplay版Slope Trickは別APIとして公開する。
永続化は任意型の値を持つPersistentArrayとPersistentUnionFindを追加し、既存永続構造と比較する。
Retroactive Priority Queueは過去操作を編集して現在の状態を得る部分retroactive版。

## アルゴリズムとAPI

新規25公式verifyドライバについて、担当ごとの実行で計585ケースが成功した。
heap版Slope Trickと部分Retroactive Priority Queueは対応する専用公式問題を未登録として明記し、
前者は整数点での凸関数比較、後者は履歴を毎回再生する愚直解との比較を実施した。
通常の優先度キューの問題を過去編集の検証の代わりにはしていない。

各追加構造にGCC/Clang、C++20/23の確認と固定seedのランダム比較があり、
ASan/UBSanでも検証した。独立レビューではDynamic Top Treeのall_prodが以前の根に依存する
境界を返す問題を見つけ、単一頂点境界へ正規化し、回帰テストで確認した。
Dynamic Top Treeの保証は保守的な償却O(log² N)で、狭義の辺クラスタモデルそのものとは区別する。

## 比較測定

- [DST・Sqrt Tree・ACL segtree](range-product-selection-comparison.md): 更新率と操作数を変えた160サンプル。
- [集合・双対セグ木・ヒープ・Slope Trick](specialized-structures-comparison.md): 90サンプル。
- [区間LIS・区間ソート・部分Retroactive PQ](advanced-range-expansion-comparison.md): 60サンプル。
- [固定木での静的・動的構造](dynamic-tree-comparison.md): 60サンプル。
- [線形RMQ/LCA・KD Tree](static-tree-expansion-comparison.md): 同一入力で各5回。

コンパイラ・入力・ソースhash・生ログを各レポートから参照できる。
双対セグ木、線形RMQ/LCA、KD Tree、固定木での動的構造が比較対象より遅い場合もそのまま記録した。
専用構造の速度を一律に優れているとは説明していない。

## 統合チェック

- `make check`: 101 API文書、105実行可能例、102単独/重複include、137 verify compileが成功。
- 90個のrandom programを各20seed（1〜20）で実行し成功。
- 表示崩れの回帰テスト追加後、最終Python unit 92件も成功。
- Clang/C++23/NDEBUGの全header確認はACLを`-isystem`で分離して成功。
- Jekyll buildと`check_site.py --without-metrics`が成功。公式verify全体の実行結果は各verifyページと測定一覧で確認できる。

初回のrelease header確認はACL内部のassert専用変数に対するunused警告で失敗したため、
AGENTS.mdの指定に従って外部ACLの警告を分離した。自作ヘッダの警告を無効化したわけではない。
ドキュメント生成は新benchmark runnerのPython依存解析でtimeoutし、既存runnerと同じ除外設定を追加して成功した。
Link-Cut Tree等4ページのHTML summary中にあるテンプレート括弧もescapeし、コード例のMarkdown崩れを解消した。
初回失敗ログは破棄せず、再実行結果と区別して保持した。

## ブラウザ確認

Browser pluginがこの環境で利用できないため、Playwrightから実際のEdgeを起動して確認した。
desktop 1440×1000とmobile 390×844を使用。スクリーンショットも目視し、ページ全体の横はみ出しがないことを確認した。

| 確認項目 | 結果 |
| --- | --- |
| 初期状態で比較を折り畳み、候補のリンクから該当比較と親を開く | 成功 |
| リンクにfocusしてEnterで開く | 成功 |
| mobileの比較表だけ横スクロール、案内文を表示 | 成功 |
| 配列選択後、祖先・辺接続・部分木操作を候補から除外 | 成功 |
| 固定木条件でもStatic/Dynamic Top Treeの両方を表示 | 成功 |
| 文字列では部分文字列操作を表示し、辺操作を表示しない | 成功 |
| 個別APIにも同じ比較を折り畳んで表示 | 成功 |
| verify不足一覧に未登録構造と関連ローカルテストを表示 | 成功 |
| JavaScript無効でも64候補とnative detailsを利用可能 | 成功 |

ページのJavaScript例外・必須リソースのエラーはなし。ローカルserverの任意faviconだけは対象外として記録した。
clipboardの成功・fallbackと入れ子anchorは既存`tests/docs_ui_test.cjs`でも確認した。
本確認は上記2サイズとEdgeが対象で、全ブラウザ・全assistive technologyを網羅したものではない。
