# LCAの高速化調査（2026-10-17）

基準は`d6af3e9`です。公開API、任意の頂点番号・根、無向隣接リストと親→子リスト、オンラインで到着する問い合わせ、最悪計算量を維持します。HLDは`origin/perf/hld-api-2026-10-04`で同じ領域の改善が進んでいるため、この変更では編集しません。

## 一次ソースと候補

[KACTL LCA.h](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/LCA.h)はDFS順に並べた親のpreorder番号にRMQを行う方法を掲載しています（CC0、folklore）。この数学的な帰着を反復DFSで実装し直しました。異なる2頂点のpreorder番号を`a < b`とすると、preorderで`(a,b]`に入る頂点の親番号の最小値がLCAのpreorder番号です。同じ頂点への問い合わせはその頂点を返します。

従来のEuler Tourは`2N-1`個の`pair<int,int>`をSparse Tableへ渡していました。候補は`N`個の`int`を渡すため、表の要素数をおよそ半分、1要素を半分にします。頂点への逆引き配列は`N`個の`int`が増えます。DFSの復帰処理、辺の再開位置、Euler Tourからpairへの変換も不要になります。計算量は構築・メモリO(N log N)、LCA O(1)のままです。

[KACTL BinaryLifting.h](https://github.com/kth-competitive-programming/kactl/blob/main/content/graph/BinaryLifting.h)と[ei1333 Doubling LCA](https://ei1333.github.io/library/graph/tree/doubling-lowest-common-ancestor.hpp.html)は頂点数から表の高さを決め、各行を走査する実装です。Blueberryの候補では実際の最大深さから必要な行数を決めます。LCAの同時上昇は深さを揃えた頂点のbit幅までに限定します。浅い木で不要だった表の初期化・構築・問い合わせを削減します。長いパスの最悪計算量と祖先クエリの有効範囲は変わりません。

追加候補`bit-scan.hpp`は距離の1-bitだけを`std::countr_zero`で辿ります。0のときは走査せず、符号なし変換後に呼ぶため未定義動作はありません。採否はGCC/Clang、release/assert有効、浅い木と深い木の結果を見て決めます。

## Library Checker Fastest

公開REST APIから上位3件のAC提出を取得しました。取得結果は`fastest/fastest.json`、表示時間は`fastest/report.md`に保存しています。これらの時間とローカルの時間は比較しません。取得したソースのhashは`fastest/source-hashes.json`に保存し、著作権表示が不明なソース自体は追跡対象外の`.benchmark/research/frequent-2026-10-tree/`に保存しました。

[278239](https://judge.yosupo.jp/submission/278239)は固定上限500000、親が子より小さい頂点番号、番号順の部分木サイズ集計を使い、親番号のminを64要素ブロックのprefix/suffixとブロックSparse Tableで取得します。同じブロックでは線形走査します。[370969](https://judge.yosupo.jp/submission/370969)と[370928](https://judge.yosupo.jp/submission/370928)もDFNとブロックRMQを使います。専用I/O、`Ofast`、固定配列を含むため、表示時間だけでは構造の効果を分離できません。任意label/rootのBlueberryでは親の頂点番号のminを直接使えず、preorder番号への変換が必要です。

固定サイズのブロックRMQや既存のLinearRMQを使う案もありますが、今回はSparse Tableのままで表現の冗長性を除きます。LinearRMQを使う別のLCAは既存の`linear-lca.hpp`にあります。HLDへの置き換えは構築O(N)になる一方で祖先・パスクエリ向けに配列を増やし、RMQ版のO(1)問い合わせを維持しないため採用しません。汎用Sparse TableをLCA専用に変更することもありません。

## 再現手順と測定範囲

`run.py prepare --output full`でGCC release/assert有効とClang releaseをコンパイルします。他の測定・重いテストが終わってから`run.py run --output full`を実行します。基準と候補のヘッダは同梱し、Sparse Tableも基準revisionへ固定しています。生成した木・問い合わせは同じseedです。入力生成を時間外に置き、構築・LCA/祖先操作・合計を分けて記録します。この測定には入力parseや答えのformatを含みません。操作には結果を使うchecksumの計算が含まれます。プロセスRSSには共通の木・問い合わせ配列とランタイムが含まれます。

N=200000、Q=500000、path/star/balanced/broom/random/permuted、各構成はwarmup 1回の後5回です。各roundで構成順を固定seedでshuffleし、CPU affinityを固定して直列実行します。全checksum一致を必須とし、compiler・flags・CPU・ソースhash、全試行、中央値・最小値を別ファイルに保存します。小規模のN/Q比は別出力先へ保存し、同じ測定には混ぜません。

## 検証

`tests/random/lca-boundary.cpp`はseedを引数または`BLUEBERRY_RANDOM_SEED`から受け取ります。N=1、2冪付近、path/star/balanced/broom/random、任意label/root、隣接順のshuffle、親→子/無向の両方について、全頂点対のLCA・距離と全有効祖先距離を愚直解と比較します。失敗時はseed、根、問い合わせ、期待値・実際値、隣接リストを出力します。さらにN=200000のパスで再帰を使わない構築を確認します。

## 採用結果

最大深さによる親表の削減、LCAの必要な行だけの走査、距離の1-bitだけを辿る祖先ジャンプ、N要素の親preorder番号RMQを採用しました。公開APIと最悪計算量は変わりません。

全1620試行のchecksumが一致しました（warmupを含む）。`full/`はN=200000、Q=500000、3 compiler設定の864試行、`packed-retry/`は追加候補の324試行、`small/`はN=4096、Q=100000、2 compiler設定の432試行です。各設定の中央値・最小値と全試行をそれぞれ保存しています。代表値と採否の詳細は[REPORT.md](REPORT.md)にまとめました。

GCC releaseのrandom木ではLCA操作が50.74→18.72ms、祖先操作が5.40→2.91msでした。RMQ版の構築は23.48→7.15ms、構築とqueryの合計は28.42→12.04ms、プロセスRSSは80368→33508KiBでした。同条件のClangとassert有効でも構築・操作合計が改善しています。小規模追試も全構成の合計中央値が改善しました。

RMQのパス木では逆引きの追加loadによりquery単体が遅くなる試行があり、初回のClang releaseでは4.82→5.32msでした。構築は13.12→4.38ms、合計は18.49→9.65msです。queryだけの一律な高速化は主張しません。親preorder番号と親頂点番号を64bitへ詰めて逆引きを避ける候補も測りましたが、int版よりRSSが約13MiB増え、合計時間が概ね悪化したため棄却しました。

GCC/Clangとgnu++20/23による境界・乱択・既存tree random・単独重複include・verify driver compile、GCC ASan+UBSanが通過しました。106個のcompile/runコマンドと終了状態を`validation.log`/`validation.json`に保存しています。リポジトリ全体の検証と公式テスト実行は統合担当で行います。
