# Offline Dynamic Component Sum 追加記録

2026-10-03。`82f6cb427df64aa4a92e0849d7d4632cb9c51283` を基点とする
ローカルブランチ `feat/offline-dynamic-component-sum` の独立した追加です。
PCへのソース同期用に作業ブランチへのpushが許可されています。mainへのmerge・deployは行いません。

## 追加範囲と契約

[OfflineDynamicComponentSum](../graph/offline-dynamic-component-sum.md)は、
無向グラフの辺の追加・削除、頂点加算、連結成分和を記録してまとめて解くAPIです。
ACLのDSUは辺の削除を扱わず、既存のEuler Tour Treeは森の接続・切断を対象として
閉路を作る辺を拒否するため、一般グラフの本問題を直接置き換えられません。
既存の `RollbackUnionFind` をそのまま再利用し、成分和の履歴だけを重ねています。
既存データ構造の公開API・実装は変更しません。

- 辺は向きを区別しない多重集合。追加1回につき1本増え、削除は1本だけ消費する。
  存在しない辺の削除は `false` を返して状態を保つ。自己辺も同じ追加数・削除数の規則を使う。
- 頂点加算の効果は以後の問い合わせに残り、成分が分裂した後も対象頂点側に残る。
- `query(v)` は登録順のIDを返す。`solve() const` はID順の全回答を返す。
  再実行と後続操作の追加を許し、過去の問い合わせの意味を保つ。
- N=0・問い合わせ0件を許す。頂点を引数にする操作には常に有効な添字が必要。
- Tにはコピー構築・コピー代入と、結合的・可換な加算を要求する。
  単位元、デフォルト構築、比較、減算、`+=`、ムーブ操作は不要。
  加算の結果は暗黙にTへ変換できるものとする。
- ネイティブ符号付き整数では、内部で順序を変えて求める部分和もすべて型の範囲内であること。
  最終回答や時系列順の頂点値だけが収まる条件では不十分。
  Library Checkerの非負制約では `long long` に十分収まる。
- N≤INT_MAX、成功した登録操作の総数Q≤INT_MAX/4。
  多重辺の追加・自己辺の操作もQに含め、存在しない辺の削除は含めない。

## 正当性と実装上の判断

時刻をK個の問い合わせ位置に圧縮します。辺の多重度が0から正になる時点で区間を開き、
正から0になる時点で閉じます。各頂点加算は登録時点から最後の問い合わせまでの接尾区間です。
閉じた区間に問い合わせがなければ捨てますが、未閉鎖の辺と末尾の加算は記録に残します。
この区別により、現在の `solve()` では見えない更新も、後で追加した問い合わせには反映されます。

区間を時間方向のセグメント木へ分配し、各葉への経路で有効な辺・加算を1回ずつ適用します。
辺の併合は互いに素な成分をまとめ、加算は対象頂点の現在の代表へ適用します。
結合則と可換則により、途中の適用順が異なっても、その葉の成分和は一致します。
各節点から戻るときは、保存した代表の配列添字へ以前の和を代入してからUFを巻き戻します。
代表を改めて検索して復元せず、減算による逆演算も使いません。

時間木の処理列は節点ごとのvectorではなく、offsetとイベントIDの連続配列に格納します。
offsetと履歴長には `size_t` を用い、辺IDと加算IDの符号で処理を区別します。
回答は `reserve` と `push_back` で構築し、Tのゼロやデフォルト構築を要求しません。
重複併合をUFへ渡さないため、同時に保持するUF履歴は高々N−1です。

K>0の `solve()` はO(N+Q log(Q+1) log(N+1))時間、
O(N+Q log(Q+1))作業メモリ。K=0ならO(1)で空配列を返します。
辺追加は最悪O(log(Q+2))、成功した辺削除は償却O(log(Q+2))、
頂点加算と問い合わせ登録は償却O(1)です。

独立レビューでは、加算結果を直接代入すると、コピー代入が可能でもムーブ代入を
明示的に削除したTでコンパイルできないことが見つかりました。
加算結果を名前付きの一時値に保存してコピー代入する形へ修正し、専用回帰テストを追加しました。
閉じた辺区間のコピー後に再確保する無駄も、事前reserveとinsertで除きました。

## 調査と比較条件

[NyaanNyaanのOffline Dynamic Connectivity](https://github.com/NyaanNyaan/library/blob/master/graph/offline-dynamic-connectivity.hpp)と
[KACTLのUnion Find Rollback](https://github.com/kth-competitive-programming/kactl/blob/main/content/data-structures/UnionFindRollback.h)
で時間区間分解とrollbackの構成を確認しました。実装コードは転用していません。
[Library Checker公式解](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/graph/dynamic_graph_vertex_add_component_sum/sol/correct.cpp)
はオンラインの動的連結性を扱うため、本APIと同等契約の速度比較対象とはしません。
Fastest APIは既存作業で403となっており、指示どおり再試行していません。
順位や「最速」の主張はしません。

性能比較では公開ヘッダと、同じ記録・UF・成分和の履歴・処理順を保ち、
時間木の格納だけを `vector<vector<int>>` に替えた対照実装を使います。
両者とも同じイベントIDを格納し、N・Q・問い合わせ比率、コンパイラ、assert設定を揃えます。
入力生成を時間外へ置き、構築・登録・solve・破棄を含む全体時間を分けて記録します。
I/Oとallocation/RSSは別の実行で確認します。

### 測定結果と採否

GCC 14.2 / Clang 19.1、GNU++20、`-O2`、assert有効・無効の4構成です。
長寿命辺・短い辺寿命・頂点加算中心はN=Q=300,000、閉路・多重辺中心はN=4,096、Q=300,000。
問い合わせ数Kは順に100,000、120,000、30,000、59,808です。
各組合せでwarmup 1回と測定3回、kernelは128実行、releaseのみのI/O比較は16実行。
別途allocation 8実行とRSS 8実行を保存しました。
準備時のBFS照合、独自型、4大規模出力配列の完全一致、40件のコンパイラ間入力・出力照合も通過しています。

次表は構築・登録・solve・データ構造の破棄を含む全体時間の中央値の差です。
負値は連続配列が速いことを表し、前処理を除いた改善率ではありません。

| 入力 | GCC release | GCC assert | Clang release | Clang assert |
| --- | ---: | ---: | ---: | ---: |
| 長寿命辺 | +7.1% | +24.1% | +0.4% | −0.5% |
| 短い辺寿命 | −11.8% | −25.5% | −15.2% | −16.5% |
| 頂点加算中心 | −3.5% | −11.7% | −4.3% | −3.5% |
| 閉路・多重辺 | +3.3% | −14.2% | −10.0% | −7.7% |

solve単体の中央値では16条件中12条件で連続配列が速くなりました。
短い辺寿命の全体時間は4構成すべてで測定範囲が分離しています。
一方、長寿命辺のsolveは4.0〜25.8%遅く、GCC assertの全体時間も測定範囲が分離して遅くなりました。
連続配列は区間を数えてから格納する2回の走査を行い、対照は1回の走査で各vectorへ追加します。
この処理量とallocation・配置の違いを含む比較で、内部段階ごとの時間やCPU命令の原因分析までは行っていません。
GCC releaseの閉路ケースでは、共通実装の登録時間にも約21%の差が出ており、
全体時間の差を格納方式だけの安定した効果とは断定できません。
共有ホスト・CPU固定なしの測定で、外れ値と全サンプルを保存しています。

非計装実行の最大RSSは、連続配列/対照の順に
43,364/48,220 KiB、27,444/36,276 KiB、38,808/46,104 KiB、18,528/22,964 KiBでした。
計装実行のallocation回数も同順で100,063/300,100、60,062/300,074、44/180,056、28,532/204,371です。
allocationの計測区間は構築・登録・solveを含み、入力生成を含みません。
RSSは入力も含むプロセス全体の最大値です。
多くの条件での時間短縮と全条件でのメモリ削減を理由に連続配列を採用し、
長寿命辺で常に有利とはしません。環境依存の切替閾値や公開APIの分岐は追加しませんでした。

I/O比較では短い辺寿命の同一入力について、既存のFast I/Oが入力解析の中央値を
GCCで40.82→15.45 ms、Clangで41.04→18.52 msへ短縮しました。
解析・整形を含む全体の中央値は91.36→57.00 ms、85.59→57.29 msです。
このI/O測定用ハーネスは一旦操作列を保持するため、逐次登録する公式driverそのものの時間ではありません。
共通の入力処理改善として公式driverに既存 `FastInput` / `FastOutput` を採用し、下記の最終driver検証で公式ケースを再確認しました。

初回の測定準備では、複数の乱数呼び出しを関数の引数に置いたため、
GCCとClangで入力列が異なり、照合ゲートが停止しました。
測定開始前に乱数の評価順をローカル変数で固定し、失敗した準備ログを残した別実験で全照合をやり直しました。
この失敗を性能結果に混ぜていません。

## 検証結果

- 固定したLibrary Checker revision
  `1814c4e5205517e368bb57a8d1127eb961cfeaae` の公式19ケース。
  38個の入力・期待出力のハッシュが公式値と一致し、公式checkerで19×3=57 AC。
- 独立した辺多重度行列・時系列頂点値・BFSによる照合を含む8コンパイラ構成×4 seed。
  GCC/Clang、GNU++20/23、assert有効/無効の32実行で、91,976問い合わせ、
  80,904 solve、3,183,296個の有効なチェックが成功。
  問い合わせのうち192件はコピー専用型の手計算済み回帰ケースで、残り91,784件がBFS照合。
- 固定した最終ヘッダで250件の期待結果を確認。
  単独・重複・all.hppの24コンパイル/実行、8構成のUF状態復元、
  不正添字64件の期待したSIGABRT、ASan/UBSan/リーク検出の2 seedを含む。
  64件の期待した拒否を通常の成功終了と混同していません。
- `make check CXX=g++ CXX_STANDARD=gnu++20 COMPILE_JOBS=2` はexit 0、446.96秒。
  99 unit tests、109文書・114実行例、110ヘッダ、147 verifyドライバ、57乱択テスト×20 seedが成功。
  この実行開始時のmanifest後に新ヘッダと新乱択テストをレビュー修正したため、
  開始時manifestを最終版の固定実行の証拠とはしません。
  変更した最終ヘッダ・テストは上記の独立した固定実行で再確認し、API文書もGCC/Clangで追加確認しています。

最終のFast I/O版公式driverはGCC/Clang・GNU++20/23・assert有効/無効の8構成でコンパイルし、
各構成の公式例題がACとなりました。GCC GNU++20・assert有効では公式19ケースを3回実行し、57 ACを確認しました。
138ステップすべてが終了コード0です。最終ヘッダ・乱択テスト・既存のfocused記録のハッシュも維持しています。
[最終driver記録](../../benchmark/results/offline-dynamic-component-sum/final-driver.json)と
[ログ・保存対応表](../../benchmark/results/offline-dynamic-component-sum/final-driver-provenance.json)に、
従来のfocused検証とは分けて記録しました。

`make docs` は文書生成後に既存の公開メトリクス検査で終了コード2となりました。
拒否理由は `do not publish unsupported or failed measurements` で、成功した文書ビルドとしては扱いません。
独立したJekyllビルドは終了コード0、`scripts/check_site.py --without-metrics` も終了コード0で、
7カテゴリ・109 APIページを確認しました。公開メトリクスを含む全体検査の成功を意味するものではありません。
最終文面を反映した再描画と、各コマンドのログ・入力ハッシュは
[文書・サイト確認記録](../../benchmark/results/offline-dynamic-component-sum/docs.json)に保存します。

## 再現と証拠

最終ヘッダSHA256:
`7d54e1694d42da12503677a76f853197fc592ae351e60d6cf916cbb2694e6be9`。
独立乱択テストSHA256:
`75159cfbe547ea52fbbf594f469955d8a5ae4cc98e1756079275aac03501309a`。
コマンド・入力識別子・コンパイラ・生の測定値・拒否ログを
[benchmark/results/offline-dynamic-component-sum](../../benchmark/results/offline-dynamic-component-sum/)へ保存します。
大きな公式データ・生成バイナリはGitへ含めません。
実行環境内の原本は `.verification/offline-dynamic-connectivity/` に残します。

```sh
source /tmp/blueberry-setup/env.sh
make check CXX=g++ CXX_STANDARD=gnu++20 COMPILE_JOBS=2
python3 benchmark/offline-dynamic-component-sum.py prepare --experiment rerun \
  --production-sha256 7d54e1694d42da12503677a76f853197fc592ae351e60d6cf916cbb2694e6be9
python3 benchmark/offline-dynamic-component-sum.py measure --experiment rerun
python3 benchmark/offline-dynamic-component-sum.py diagnostics --experiment rerun
python3 benchmark/offline-dynamic-component-sum.py summarize --experiment rerun
```

`benchmark/offline-dynamic-component-sum-validate.py` は、このコミット前の実行を固定するため、
`.verification/offline-dynamic-connectivity/root-base.json` に記録したHEAD
`82f6cb427df64aa4a92e0849d7d4632cb9c51283` とソース・データの一致を検査します。
コミット後のHEADでは停止するため、任意のチェックアウトでそのまま再実行できるコマンドとはしません。
検証runnerは用意済みの公式データを使い、ネットワークへアクセスしません。
最終driverの8構成のコンパイル、19ケースの実行、公式checker呼び出しの引数は
[最終driver記録](../../benchmark/results/offline-dynamic-component-sum/final-driver.json)の `steps`、
実行したコードは [runnerの保存写し](../../benchmark/results/offline-dynamic-component-sum/final-driver-runner.py.txt)を参照してください。
これらは実行履歴です。再実行には記録と一致する公式入力・期待出力・checkerを用意し、
ローカルの入力パスと新しい出力先を指定する必要があります。保存済みの結果や上限・ハッシュ検査は書き換えません。
性能測定の `prepare` は比較ソース生成・コンパイル・正答照合、`measure` は他の重い処理がない状態で実行します。
既存の実験ディレクトリへ上書きしません。

## 全体の未解決事項

本バッチは独立レビューと限定検証を完了した専用ブランチです。既存の統合版82f6とは別に保存し、作業ブランチでソースを同期します。mainへのmerge・公開は行いません。
この追加の公式問題に対する結果と、リポジトリ全体の公式verifyの成否は別です。
既存のAOJ `ALDS1_14_B` はデータ取得時のCONNECT 403で未完了のままです。
再試行・別経路での回避は行いません。
元の `.verification/current.json` を限定実行の成功結果で上書きしません。
公開用の測定ゲートに失敗が残る間、`make docs` や公開全体を成功とは記載しません。
