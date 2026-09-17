# データ構造拡充: 実装・検証・性能調査

## 採用したAPIと設計

- SegmentTreeBeats: long long公開値、chmin/chmax/add/prod/min/max/get/set。混合clampの償却log²境界、空min/maxはoptional。歴史的なlazy addの蓄積と、push時の一時的大値を扱うため内部128bit（標準環境112byte/node、4N確保）。任意のS/action抽象化は追加せずBeatsに必要な値構造だけを保持。
- BinaryTrie<UInt,Bits>: 重複、多重度、erase-one、rank/kth、最小XOR値。型全幅bitshiftを安全に扱い、不要な経路はpoolへ回収・再利用。countによるset化は公式driver側。
- AggregateQueue/Deque<S,op,e>: 2方向の集約を持つvectorスタック。非可換monoidに対応し、default constructorや逆元は不要。Queueは一方向転送、Dequeは片側が空の削除時にほぼ半分へ再配置。端点更新償却O(1)、最悪O(N)、全体集約O(1)。

全4型に独立copy、安全なmove、空で再利用できる移動元、自己代入、所有値の返り値を採用。共有catalog/all.hpp/READMEは親担当。公開ソースの転記は行っていない。

## TDD / 境界

`.build/operation-structures/tdd-red.log`: ヘッダ未作成の期待されたコンパイル失敗。
`tdd-green.log`: 実装後の最初の独立oracle PASS。
`tests.log`: GCC13/Clang18 GNU++20/23各10seed（Wall/Wextra/Wshadow/Werror）、release単独/double include、ASan/UBSan3seed PASS。
`examples.log`: 全4日本語API文書の最小例をGCC/Clangでcompile/run PASS。

random testはvectorの区間clamp/add/get/set oracle、重複込みsorted vectorのTrie全操作、std::dequeによる非可換affine合成を使用。64bit最上位、1bit Trie、pool再利用、空/単一、累積lazy加算>int64、内部区間和>int64だが返却値範囲内、自己copy/move、移動元再利用、Deque両端交互削除、集約演算回数による償却挙動を確認。

独立レビュー担当からの追加5seed ASan/UBSanもPASS。Queueの片端削除に関する文書表現と汎用テンプレ文を指摘に沿って修正済み。アルゴリズムの指摘はなし。

## Fastest一次資料（取得2026-09-17）

取得コマンド: scripts/fetch_lc_submissions.py の4 problem指定、--limit2 --analyze-top1。metadata/source hashはbenchmark/results/operation-structures/fastest-metadata.json。外部ソースはignored .build/operation-structures/fastest-sources/にだけ保存。

- range_chmin_chmax_add_range_sum [204482](https://judge.yosupo.jp/submission/204482): OYのカスタムNode型を使うBeats木、int64値・和・lazy、極値と第2極値・個数を保持。buffered I/O。アルゴリズムは同じ系統で、こちらは公開値範囲を明示し大きな歴史的lazyを128bitで保護。
- set_xor_min [403013](https://judge.yosupo.jp/submission/403013): bounded_xor_set<30>、bitmap葉・directory/occupancy階層とAVX2 I/O。集合・最小XORに特化。こちらの重複・順位APIと全UInt幅対応の分だけ機能と記憶量が異なる。
- queue_operate_all_composite [403012](https://judge.yosupo.jp/submission/403012): 事前容量指定のlinear aggregate queue、連続値配列とsuffix/front、単一back fold、AVX2 I/O。転送時の集約更新を連続領域で行う。
- deque_operate_all_composite [403007](https://judge.yosupo.jp/submission/403007): centered aggregate deque、事前容量指定の中央配置とcut/topの両側集約、再構築。こちらはstd::vectorの2スタックで一般的なcopy可能monoid値を所有。

[NyaanのBeats解説と実装](https://nyaannyaan.github.io/library/segment-tree/segment-tree-beats.hpp.html)をAPI/アルゴリズム比較にも用いた。公開ソースは参考設計として確認し、本体へコピーしない。

## 測定条件

benchmark/operation-structures-prepare.py がseed20260917、各Q100000、Beats N100000の入力を生成。現行driverと取得上位sourceを同じGCC GNU++23 -O2 -DNDEBUG -march=nativeでcompile。

benchmark/operation-structures.py はnative Linux一時領域へ同じ入力とbinaryを複製し、通常ファイルstdin、起動/I/Oを含むend-to-end、warmup1回＋候補を交互に5回測定、全integer tokenのSHA256を照合。失敗や不一致は停止・ログ保存し、0へ置き換えない。他担当の重い作業を止めたquiet枠のみで実行。公開judgeの時間とは比較しない。

初回測定は親の統合compileと開始連絡が競合して並走したため不採用。initial-overlapped-*.jsonへ保存。Beats初回入力に公式制約外の空clamp区間が2個あったため、再測定用generatorはl<rを強制し、add幅を10^6以下にして全処理中の絶対値<=10^8+Q/4*10^6<10^12も構成的に保証する。ライブラリ自身の空区間契約は従来どおり。


## 公式検証

公式4drivers、`--repeats 1 --timeout 1800 --output .build/operation-structures/official-retry`で80/80ケースPASS（Deque16、Queue19、Trie12、Beats33）。初回はchecker生成のgit pullがGitHubのDNS解決に失敗し中断したため、失敗出力をofficial/へ保持して別outputへ再試行した。コードの不正答ではない。

親の統合compileと並走したため公式elapsedは比較性能の結論に使用しない。全compiler/case/input/hash等はofficial-correctness.jsonに保存。helperのcutoffは60秒で、各LC問題の制限を設定した投稿実行ではない。

数学担当の独立レビューも実施し、指摘なし。GF2全小行列とrhs5054systemsおよびN<=8全畳み込み入力対をASan/UBSanで検証。詳しくは.build/operation-structures/review-math.md。

## 採用する静かな枠の測定結果

親のdocs Pythonと残っていたcompiler子2個がすべて停止状態Tであることを確認して再実行。終了後、子だけをresumeし、親の再開を依頼した。quiet-processes.jsonに停止確認を保存。8variantすべてcompile成功、各warmup1+測定5回で全integer token一致。source/header hashもprepare時と一致したものだけ測定する。

単位ms。Q=100000、BeatsのみN=100000。他は空から開始する。入力とbinaryはnative Linux一時領域、Intel Core i7-14650HX / WSL2 / GCC13.3 GNU++23 -O2 -DNDEBUG -march=native。

| 問題 | 実装 | 中央値 | 最小 |
| --- | --- | ---: | ---: |
| Range Chmin Chmax Add Range Sum | Blueberry | 179.57 | 175.88 |
| 同上 | 公開204482 | 92.32 | 89.94 |
| Set Xor Min | Blueberry | 26.84 | 26.18 |
| 同上 | 公開403013 | 5.96 | 5.88 |
| Queue Operate All Composite | Blueberry | 8.82 | 8.65 |
| 同上 | 公開403012 | 2.53 | 2.43 |
| Deque Operate All Composite | Blueberry | 8.39 | 8.29 |
| 同上 | 公開403007 | 2.47 | 2.27 |

この比較ではBlueberryは公開上位提出より遅い。高速化したという主張はしない。Beatsは同じ極値アルゴリズムだが、広い数値契約を守る128bit内部情報と4N配置を選択した。Trieは汎用UInt幅・重複・順位を維持し、公開版は30bit集合のbitmap階層を使う。Queue/Dequeは同系統の集約だが、こちらは動的vectorと値所有、公開版は総操作数を容量予算にした連続配置を使う。公開版の専用剰余演算/AVX2 I/Oと、こちらのACL modint/iostreamも異なる。

測定はプロセス起動とI/O込みなので、これらの設計差が何割の時間差を生むかは分離していない。単純なkernel定数差やjudge順位を再現した結果とは扱わない。全4実装の公開APIは公式80ケース・独立oracle・両compiler・sanitizer・独立レビューを通過している。専門特化版を無理に取り込むより、今回の機能範囲・数値条件・所有権を保ち、差を公開する判断とした。

再実行: repo rootでFastest取得後 `python3 benchmark/operation-structures-prepare.py`、他のcompile/verify処理を止めた枠で `python3 benchmark/operation-structures.py`。raw.jsonはすべてのサンプル、results.jsonは環境・flags・source/header/input hashと中央値/最小。initial-overlapped-*.jsonは不採用の初回として残す。

## 担当完了

4ヘッダ、4日本語API文書、4公式driver、tests/random/operation-structures.cpp、再現可能な比較script・全測定ログを追加。既存API/共有catalog/all.hpp/READMEは担当側では変更していない。commit/pushは行わず親へ引き継ぐ。全体make check/docs/サイト/PR作業は親の統合担当。
