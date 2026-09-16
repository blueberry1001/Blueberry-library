# Contest readiness — 2026-09-16

## 変更と利用範囲

既存のC++20・ACL-first・verification-helper/Pages構成を維持し、
カタログを17ライブラリ、公開・互換入口を19ヘッダ、公式verifyを29本に整備しました。
今回の開始点は `b4695f9`。その時点のLi Chao Tree / Wavelet Matrix / Manacher追加も
mainへの反映対象に含まれます。

- FPS: ACL modintでコンパイルできなかった疎多項式除算、同じFPS内の係数を参照する
  スカラー乗算、要求次数以降を無視すべき平方根、定数多項式を法とする0乗を修正。
- Dijkstra: 標準整数型では加算前に改善可能な距離差を判定し、符号付き・符号なしの
  overflowを防止。距離上限以上は未到達扱い。独自Costには新たに減算を要求しません。
- Offline Fenwick Tree 2D: 更新座標を事前登録して点加算・半開矩形和を処理。
- LowLink: 無向多重グラフの橋・関節点・二辺連結成分。再帰を使いません。
- Rerooting DP: 頂点・辺の変換を指定する全方位木DP。入力辺順で結合し、非可換演算にも対応。
- HTML: 日本語/英語/全角の別名検索、カテゴリ絞り込み、例のコピー、深いAPIリンク、
  モバイル表示、ローカル依存リンク、展開済みヘッダ表示を整備。
- 検証: 二重includeと `-DNDEBUG -Werror` のCIゲートを追加。既存HLD/LCAの警告を注釈で修正。

旧スニペット6件は、API互換保持のため残し、新規利用対象外であることをREADMEに明示しました。
全旧ファイルの安全性や、すべての競技アルゴリズムの網羅を保証する変更ではありません。
残る移行・拡張候補は [ROADMAP](../../ROADMAP.md) の最新欄を参照してください。

## 検証

| 検査 | 結果 |
| --- | --- |
| `make check` | PASS: Python 24テスト、17 API文書/18実行例、19ヘッダ、29 verify、9 randomプログラム×20 seeds |
| GCC 13.3 / Clang 18.1.3 × GNU C++20/23 | 4構成ともcompile/random PASS |
| リリース・二重include | 19ヘッダ×4構成、76/76 PASS (`-DNDEBUG -Werror`, ACLはsystem include) |
| ASan + UBSan | 9 randomプログラム×3 seeds (101–103) PASS |
| 最短路の最終調整 | GCC+UBSan / Clangで20 seeds PASS |
| 追加の公式検証 | FPS除算35ケース、LowLink21ケース、Rerooting16ケース、それぞれ3回PASS |
| 公式verify全体 | 29/29本、624ケース×3回PASS。初回28本成功＋時間上限で未実行の1本を再開 |
| `make docs` + github-pages Jekyll | PASS |
| `check_site.py`（全計測反映後） | 4カテゴリ/17 APIページ/29計測済みverify、依存リンク・footer・ナビゲーション PASS |
| 実ブラウザ | 検索、カテゴリ、リセット、`/`、コピー、API展開、依存リンク、展開済みソース、390×844表示 PASS |

最初のリリースヘッダ検査は、assert除去でunusedとなる変数・引数とACL内の警告で失敗しました。
自前ヘッダは `[[maybe_unused]]` で修正し、外部ACLは `-isystem` 指定で分離した上で、
全76件を再実行しています。Graph個別公式検証の最初の起動は `oj` のPATH不足で失敗し、
既存venvの有効化後に再実行して通過しました。これらの失敗を成功や0秒の結果には数えていません。

全件の `make verify` は、開始時の旧1500秒上限により28本成功・全ヘッダverify1本未実行で
exit 1になりました。残る `verify/utility/all-headers.test.cpp` を `--timeout 3600` と
別出力先で再実行し、9ケース×3回PASS・exit 0を確認しています。
元の `.verification/readiness-initial.json` と再開結果を保持し、環境情報の一致、残件集合、
全29本のpassedと反復数を確認して、成功行を変更せずに `.verification/readiness-complete.json`
へ集計しました。初回コマンドがexit 0だったという意味ではありません。
今後の `make verify` は既定3600秒、CIジョブは75分とし、確定コミットをCIで全件再検証します。

ローカルの詳細ログは `.build/readiness-*.log`、公式計測は `.verification/current.json` と
`.verification/run-*/`。GitHub Actionsでも全ゲートを再実行し、公式計測artifactを保存します。

## 計測

再現コマンド:

```sh
python3 scripts/run_readiness_benchmark.py
bash benchmark/run_graph_additions.sh
```

CPU: Intel Core i7-14650HX、WSL2 Linux、GCC 13.3、GNU C++20、`-O2`。
比較ベンチマークは `-DNDEBUG`、1回warmup後に5回交互実行し、入力seedは1729です。
公式検証がバックグラウンドで動作しているため、I/OやCPU負荷の影響は残ります。
同一ラウンド内の同条件比較のみを示し、数%の差を確定的な性能向上と解釈しません。
グラフ基準測定はassert有効・3回で、比較ベンチマークとは条件が異なります。

| 同条件の比較 | 操作時間中央値 | 最小 | peak RSS中央値 |
| --- | --- | --- | --- |
| Dijkstra `b4695f9` | 33.002 ms | 31.005 ms | 21,896 KiB |
| Dijkstra 安全化後 | 33.417 ms | 31.056 ms | 21,944 KiB |
| 密な2D BIT | 12.312 ms | 11.445 ms | 12,564 KiB |
| 座標圧縮2D BIT | 87.303 ms | 86.456 ms | 6,172 KiB |

Dijkstraは10万頂点・50万辺。安全化後はこの入力で約1.3%の差ですが、測定ノイズと区別できる
ほどの差とは判断していません。初回の安全化案は同じラウンドの基準より約10%遅く、
更新不要な辺の早期除外に変更しました。以前より高速という主張はしません。

2D BITは1024×1024の範囲、3万登録点、6万回の更新と矩形クエリ。
密な格子を確保できる条件では密なBITの方が速い結果です。本追加の目的は、
巨大で疎な座標を $O(N\log N)$ の空間で扱うことです。時間とメモリの交換条件を隠しません。
登録点は重複を除去した後、入力の余剰容量を解放します。

20万頂点のグラフ基準測定（中央値）:

| 形状 | LowLink | Rerooting |
| --- | --- | --- |
| path | 15.079 ms | 13.204 ms |
| star | 10.711 ms | 11.166 ms |
| 固定seedのrandom tree | 53.655 ms | 30.786 ms |

外部実装との速度比較ではありません。入力形状と再帰深度に依存せず処理できることの基準測定です。

生ログ・ソースハッシュ:

- [最終比較](../../benchmark/results/contest-readiness-2026-09-16.json)
- [初回案](../../benchmark/results/contest-readiness-initial-2026-09-16.json)
- [早期除外案の初回計測](../../benchmark/results/contest-readiness-refinement-2026-09-16.json)
- [グラフ基準測定](../../benchmark/results/graph-additions-2026-09-16/)

最初の2つの試行ではDijkstraの構築時間欄は計測対象外を表す0でした。
その値を比較には使わず、最終記録では入力グラフ構築も実際に計測しています。

## レビュー

担当をFPS、グラフ、HTML、統合に分離し、実装担当以外によるレビューを実施。
発見した余剰capacity、リリース警告、説明の不一致、生成HTMLの重複ボタンと依存リンクを修正しました。
最後の全体レビューではブロッカーなし。既存 `AGENTS.md` のユーザー変更はコミット対象から除外しています。
