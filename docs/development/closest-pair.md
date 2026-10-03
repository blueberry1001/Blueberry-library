# Closest Pair: 実装・検証・比較

統合チェックポイント `5318b6b41cafc022f1fafe0a71c63f85392bc143` から、
専用ブランチ `feat/closest-pair` に最近点対を追加した。
[公開API](../geometry/closest-pair.md) は元の添字を返し、同距離なら添字対の辞書順最小を選ぶ。
座標型・座標上限・空入力・入力保持の契約は既存の幾何APIに合わせた。
ACLには同等機能がなく、凸包・最遠点対・矩形集約用KDTreeでは代替できない。

## 採用した実装

- 符号付き64bit以下、座標の絶対値は `2^62-1` 以下。減算前に `__int128` へ変換し、平方根を使わない。
- `(x,y,元の添字)` で一度整列し、重複があれば距離0の添字対を全グループから選ぶ。
- 重複がなければ分割統治し、y順のマージと帯状領域の走査に共通バッファを使う。
- 大きい部分問題の再ソートや再帰内の動的確保はない。作業用vectorは通常2個、重複で早期終了する場合は1個。
- 各マージ開始時の距離上界を固定した充填の議論により、帯の各点の比較数は定数。
  途中で上界が縮んでも成立し、境界の等号を含めることで添字のタイを落とさない。
  最悪時間 O(N log(N+1))、作業用メモリ O(N)、再帰スタック O(log(N+1))。

実装・公式検証・比較測定に使ったヘッダのSHA-256は
`d949453afd1f3695a4f63586e15785cac518188c20f3fe5d6b49d9d06de64214`。
比較後もこの実装を維持している。

## 検証

- [全体チェック](../../benchmark/results/closest-pair/whole-check.json):
  `make check CXX=g++ CXX_STANDARD=gnu++20 COMPILE_JOBS=2` は終了コード0。
  99単体テスト、108ライブラリの113実行例、109ヘッダ、146verifyプログラム、56ランダムプログラム×20 seedが通過。
- [重点検証](../../benchmark/results/closest-pair/focused.json):
  GCC14/Clang19、GNU++20/23、assert有効/無効の8構成で40 seed実行。
  単独・重複・all.hpp経由のinclude、符号付き各整数型、境界値も確認。
  358項目が想定結果で完了し、そのうち80項目は非対応型のコンパイル拒否または範囲外入力のSIGABRTを確認する検査。
  ASan/UBSan/leak検査も2実行通過。
- 全探索oracleは独立した `boost::multiprecision::int256_t` を使い、距離と元添字のタイを照合する。
  小格子の順序付き列・多重集合、重複グループ、格子、ほぼ共線、分割境界、座標上限、平行移動・反射、入力保持を検査。
- [公式データ](../../benchmark/results/closest-pair/datasets.json)はLibrary Checkerの
  `1814c4e5205517e368bb57a8d1127eb961cfeaae` に固定。29入力と29出力の全58ハッシュが公式定義と一致。
  **29ケース×3回、87/87 AC**。公式checkerは任意の最近点対を許容するため、辞書順・空入力・広い座標範囲は独立テストで補う。

検証時のHEADには未コミットの最近点対バッチが載っていたため、成果物はコミット名だけでなく入力ソースのハッシュに結び付けた。
全体チェック開始前の重点検証記録と文書の固定ハッシュを併用し、新規ファイルを含む467入力の最終一致を確認した。
初回の全体チェック補助スクリプトは未コミットの登録情報を拒否し、makeを起動する前に停止した。
[その記録](../../benchmark/results/closest-pair/whole-check-initial-guard.json)も保存し、実行済みの成功記録と区別している。

別コミット `9fb07ca` でRetroactive Priority Queueの同順位要素を区別するテストも追加した。
比較専用の値型と時刻順の独立再生で、過去への編集・拒否後の状態・全生存要素の識別を確認。
[GCC/Clangのassert有効/無効16実行](../../benchmark/results/priority-queue-ties.json)が通過し、実装の不具合は再現していない。
ライブラリ本体は変更していない。

## 性能の比較と判断

公開ヘッダ、再帰ごとに添字vectorを作る分割統治、順序付き集合を使う走査線を、
座標上限・正確な距離・辞書順のタイ・assert検査を揃えて比較した。
GCC14/Clang19、`-O2`、assert有効/無効、6入力条件、warmup1回と測定3回。
288アルゴリズム測定と64 I/O測定には合計88 warmupを含む。
[全条件・中央値・範囲](../../benchmark/results/closest-pair/performance/initial/report.md)と
[生の測定値](../../benchmark/results/closest-pair/performance/initial/raw.jsonl)を保存した。

下表はGCC14 release、N=100,000の呼び出し全体の中央値（ms）。入力生成は時間外。

| 入力 | 採用実装 | 再帰vector | 走査線 |
| --- | ---: | ---: | ---: |
| ランダム | 26.669 | 31.293 | 14.747 |
| 重複多数 | 12.971 | 12.390 | 13.085 |
| 格子 | 32.662 | 30.142 | 25.362 |
| ほぼ共線 | 18.995 | 18.109 | 14.462 |
| 広い座標範囲 | 29.834 | 31.826 | 16.538 |

走査線はランダム100,000点で40.3～44.7%、広い座標範囲で31.2～44.6%短い中央値だった。
格子はGCCでは走査線が速い一方、Clangでは2.2～7.9%遅く、入力とコンパイラで優劣が変わる。
採用実装には走査線より遅い条件が残る。

点レコードの移動量を減らす仮説を評価するため、固定した点配列と共有添字バッファを使う候補を1つ追加した。
[追加比較](../../benchmark/results/closest-pair/performance/index-layout/report.md)は同じ24条件で、候補と実ヘッダを再測定した。
候補の中央値は9条件で短く、15条件で長い。標本範囲が分離した改善はGCC release格子の23.9%短縮のみで、
GCC releaseランダム100,000点は22.2%、GCC assert広座標は16.1%長かった。
他の範囲は重なり、大きく変動したClangの値も保持している。安定した全体的改善を確認できず、候補は採用しなかった。

N=100,000で重複のないrandom/grid入力の動的確保診断では、採用実装は2回・要求4.8MB、共有添字候補は3回・3.2MB。
別プロセスのRSSはそれぞれ7,808KiBと6,272KiBだったが、入力・ランタイム・allocatorを含む。
重複多数の入力では両者とも1回・2.4MB、RSSは5,504KiBだった。
再帰vectorや走査線との比較も、確保回数だけでなく配置・比較式・探索順序が違うため、時間差の単一原因を断定しない。
共有ホスト・CPU非固定・各3標本の結果であり、min/maxは信頼区間ではない。
I/Oのparse/solve/format、別実行の制御候補prepare/search、確保回数・RSSは個別に記録し、異なる実行の区間時間は足し合わせない。

採用実装は整数のみの分割統治と連続バッファで、タイを含む最悪計算量を保つ。
測定からコピー量だけが速度差の支配因子とは確認できなかった。走査線との差をさらに調べる場合は、
マージ・帯の候補比較数と各処理時間の分離が次の課題になる。

## 再現と公開状態

コンパイラ、Boost、ACL、Python依存は既存環境を使う。この保存環境では最初に
`source /tmp/blueberry-setup/env.sh` を実行する。公式データはネットワークを使わず、固定LC checkoutから生成する。

```sh
python3 benchmark/closest-pair-datasets.py --output .verification/closest-pair-replay/datasets
python3 benchmark/closest-pair-validate.py --datasets .verification/closest-pair-replay/datasets/report.json --output .verification/closest-pair-replay/focused
make check CXX=g++ CXX_STANDARD=gnu++20 COMPILE_JOBS=2
```

出力先が既存なら上書きせず停止する。実行済みデータ生成helperも原本のまま保存し、
移植用entry pointはパス・引数処理のみ変更して構文とhelpを確認した。移植版によるデータ再生成は未実施。
性能は `benchmark/closest-pair.py` と `benchmark/closest-pair-index.py` の
`prepare --experiment <新しい名前> --production-sha256 <対象ヘッダSHA256>`、
`measure`、`diagnostics`、`summarize` を順に使う。

文書ビルドの終了コードとログは [文書検証記録](../../benchmark/results/closest-pair/docs.json) に保存する。
既存のAOJ取得403、全体公式検証記録の失敗、公開メトリクスゲートの制約は未解消。
本バッチの公式ACで全体公式検証の成功を主張せず、既存の `.verification/current.json` も上書きしていない。
AOJ/Fastestのブロックされた接続先への再試行、push、mainへのmerge、deploy、追加アーカイブは行っていない。

調査した一次資料は [LCの参照解答](https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/closest_pair/sol/correct.cpp)、
[KACTL ClosestPair](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/ClosestPair.h)、
[maspypy closest_pair](https://github.com/maspypy/library/blob/main/geo/closest_pair.hpp)。
公開提出の表示時間を同条件の比較にせず、コードはコピーせずにAPI契約に合わせて実装した。
