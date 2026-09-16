# Library Checker coverage expansion — 2026-09-17

## 追加内容

基準はmain `42781ad`。カタログ17→29、公開・互換入口19→31、公式verifyドライバ29→46。
ACLに重複する標準APIの再実装を避け、既存のheader/短いAPI/Pages構成へ12ヘッダを追加した。
既存APIと旧snippetは変更していない。

| ヘッダ | 主な用途・API | 対応するLibrary Checker問題 |
| --- | --- | --- |
| Potential Union Find | 群の制約、`merge/diff`、矛盾判定 | [加算](https://judge.yosupo.jp/problem/unionfind_with_potential)、[非可換群](https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group) |
| Ordered Set | オンライン集合、`insert/erase/rank/kth` | [Ordered Set](https://judge.yosupo.jp/problem/ordered_set) |
| Persistent Segment Tree | 任意の過去版から`set/apply`、各版の`prod` | [Rectangle Sum](https://judge.yosupo.jp/problem/rectangle_sum)、[Point Set Range Composite](https://judge.yosupo.jp/problem/point_set_range_composite) |
| Hopcroft–Karp | 最大matching、最小vertex cover | [Bipartite Matching](https://judge.yosupo.jp/problem/bipartitematching) |
| Biconnected Components | 頂点block、block-cut forest | [Biconnected Components](https://judge.yosupo.jp/problem/biconnected_components) |
| Eulerian Trail | 有向・無向、辺IDを含む一筆書き経路 | [有向](https://judge.yosupo.jp/problem/eulerian_trail_directed)、[無向](https://judge.yosupo.jp/problem/eulerian_trail_undirected) |
| Factorize | uint64全域の`is_prime/factorize` | [Primality Test](https://judge.yosupo.jp/problem/primality_test)、[Factorize](https://judge.yosupo.jp/problem/factorize) |
| Modular Square Root | 素数modでの`mod_sqrt`、根なし判定 | [Sqrt Mod](https://judge.yosupo.jp/problem/sqrt_mod) |
| Linear Recurrence | Berlekamp–Massey、Bostan–Moriのk番目 | [Find Linear Recurrence](https://judge.yosupo.jp/problem/find_linear_recurrence)、[Kth Term](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence) |
| Aho–Corasick | 辞書登録・検索遷移・複数パターン出現数 | [Aho Corasick](https://judge.yosupo.jp/problem/aho_corasick) |
| Eertree | オンラインの回文木・回文出現数 | [Eertree](https://judge.yosupo.jp/problem/eertree) |
| Lyndon Factorization | Duval法による辞書順分解 | [Lyndon Factorization](https://judge.yosupo.jp/problem/lyndon_factorization) |

Rectangle Sumでは各x-prefixを永続版にして、過去の2版を実際に問い合わせる。
任意の版からの分岐は、コピー配列を使うrandom oracleでも確認。
最小vertex cover、Ahoの全文出現数、回文木の全文出現数など、公式出力だけでは検証できないAPIも独立oracleで検証する。

## Fastestからの採否

公開REST APIから問題別のC++ AC上位3件と上位2件のソースを読み取り、取得日時・提出番号・
source SHA-256・観察結果を保存した。提出ソース自体はcommitせず、アルゴリズムを独立実装している。
上位の実行時間とローカル計測は環境・集計方法が違うため、倍率比較しない。

- 素数判定の専用ハッシュ表・値域前提は採らず、uint64全域で使える7基底Miller–Rabin。
  素因数分解はPollard–Brentの積バッチgcdと試行budget付き再開を採用。
- kth項はBostan–Mori＋ACL convolution。NTT内部のSIMD/変換再利用まで複雑化しない。
  BMの最速上位はhalf-GCDで計算量が異なるが、今回のN=1万の公式範囲ではO(N²)BMも十分通過した。
- Ordered Setの最速上位は30bit値域や全クエリ先読みの前提を持つ。任意の比較型でオンラインに使うAPIとは区別する。
- グラフは平坦な隣接配列、再利用するstack/queue、反復DFSを採用。全辺を一度ずつ辿る操作は、
  次辺カーソルを管理して無駄な探索を避ける。再帰深度に依存させない。
- 文字列は入力形状でdense/sparseの優劣が逆転した。固定小文字種で直接遷移するAPIを採用し、
  σ倍のメモリ条件を明記。疎候補が有利なケースも記録している。

各分野の提出URL・測定環境・生ログ:

- [データ構造](../../benchmark/results/lc-expansion-ds/README.md)
- [グラフ](../../benchmark/results/lc-expansion-graph/README.md)
- [数学](../../benchmark/results/lc-expansion-math/README.md)
- [文字列](../../benchmark/results/lc-expansion-string/README.md)

同一WSLホストで公式検証が並走した時間帯がある。交互順の複数回計測を行い、僅差は結論にしない。
例として、同条件の自作候補比較では永続木のindex poolはpointer版より時間・RSSとも小さく、
疎な回文遷移は同一文字列に有利、dense回文遷移はランダム26文字種に有利だった。

## 検証とレビュー

- 変更前 `make check`: 24 Pythonテスト、17文書/18実行例、19ヘッダ、29verify、9random×20seeds成功。
- 追加4分野それぞれ、missing-header失敗を観測してから実装。固定seedの独立oracleと境界テストを追加。
- 新規17ドライバの個別公式検証: 数学95、DS103、文字列119ケースを各3回、グラフ120ケースを1回成功。
- 統合 `make test`: 24 Pythonテスト、29API文書・30実行例成功。
- GCC/Clang × C++20/23で31ヘッダ・46verify・13random各20seedとrelease includeを通過。
  追加したbyte境界ケースも同4構成で再実行し、文字列のASan/UBSan 3seedも成功。
- `make docs`、Jekyll、`check_site.py --without-metrics`で4カテゴリ・29APIページの生成を確認。
  ブラウザの通常幅736pxと狭幅390pxで日本語検索→新APIページ、詳細展開を確認した。
  関連するconsole errorなし。長いシグネチャの表は文字途中で強制改行せず横スクロールさせ、
  計算量・「開く」が一文字の縦列になる問題と、複数行リンクの行間クリックを改善した。
- リリースゲートは公式46ドライバ各3回と、計測反映後のサイト検査。
  全体の完了状態と実測はPRのCIチェック・`verification-metrics` artifact、および公開サイトの計測ページで確認する。
- 担当を入れ替えて仕様・コードをレビュー。グラフはさらに4×4二部グラフ全65536通り等の独立全列挙、
  文字列は256byte字母・NUL/high-byteケースを追加確認。統合カタログ・全公開API文書にもブロッカーなし。

初回のHTML生成ではbenchmark用Pythonの依存解析がtimeoutし、測定時生成ヘッダを使うC++候補も
依存解析に失敗した。ライブラリページではない`benchmark/`を既存の除外設定へ追加し、
`scripts/generate_docs.py`が依存解析前に除外する経路を利用する。Jekyllの除外は前方一致なので
末尾の`/`を付け、公開用`benchmarks.html`まで除外される問題を修正した。
初回の依存解析失敗と、その次のページ欠落のログは`.build/lc-expansion-docs-preflight*.log`に保持し、成功扱いにしない。

個別・全体ログは`.build/lc-expansion-*.log`と各分野の`.build/lc-expansion-*`。
公開用計測は`.verification/current.json`、CIでは`verification-metrics` artifactを保存する。

## 残る範囲

全Library Checker問題の網羅ではない。永続lazy区間コピー、dynamic tree、implicit sequence treap、
幾何、畳み込みの高度な派生などは未収録。旧6スニペットは変更せず、旧ordered treapの集合用途には新Ordered Setを案内する。
Ordered Setは期待計算量のtreapで敵対入力の最悪保証を持たず、Biconnected Componentsは自己ループを受け取らない。
Mod Sqrtの素数modはINT_MAX以下、kth項はNTT変換長、Aho/Eertreeは固定byte文字種の前提を守る必要がある。
