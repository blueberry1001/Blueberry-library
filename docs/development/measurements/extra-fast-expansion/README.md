---
title: 高速版追加と区間和改善の比較測定
---

# 高速版追加と区間和改善の比較測定

2026年10月19〜20日に実施した、整数和・静的RMQ・bitsetの用途別実装の比較。
[設計・調査記録](../../extra-fast-expansion.md)と各APIの条件を併せて読む。
既存の汎用ライブラリを置き換える測定ではない。

## 環境と測定範囲

Intel Core i7-14650HX、WSL2 Ubuntu、GCC 13.3 / Clang 18.1を使用し、CPU 0に固定した。
候補順を回転させ、最終比較は新しいプロセスで各5回実行する。pilotには3回の確認もあり、各記録の反復数を参照する。
OS全体の負荷制御はしていない。
GNU C++20で、portable O2 release、O2 assert有効、native O3 releaseをコンパイラごとに区別する。
外部提出との全体比較はGNU C++23・O2・NDEBUG・nativeで、元のpragmaを残す。
したがってCLIのflagsが同じでも、外部提出の有効な最適化設定まで同じとは限らない。

phase測定は同じ入力テキストのparse、確保・構築、操作、formatを分ける。
操作には回答配列への書き込みも含め、生成・破棄・OSのファイル入出力は含めない。
全体比較はLinux上の通常ファイルをstdin/stdoutに接続し、起動から終了までを測る。
こちらはOS入出力・破棄も含む。file cacheは消さず、fsyncしない。
両者のend-to-endを同じ範囲の値として比較しない。

メモリは初期値と構築済み構造だけを持つ別プロセスで、`/proc/self/status`のVmHWMを記録する。
allocator、コード、標準ライブラリも含むRSSであり、配列の要素数そのものではない。
時間測定時のRSSには問い合わせ・回答・テキストbufferも含まれるため、独立メモリ測定と区別する。

`scalar`は明示的SIMD経路を止めた通常のC++実装で、自動ベクトル化を禁じた意味ではない。
手書きasmの有無だけを高速化の根拠にせず、同じ条件のintrinsicsとも比較する。

## 測定用アダプタとinline

最初の測定では、共通APIへ揃える薄いアダプタの呼び出しが、コンパイラの展開上限によって残る場合があった。
RMQの公開prodをinline化しても、次にアダプタ自身が非inlineになり、効果が隠れた。
初期のwrapper版は消さず保存するが、実APIだけの固定費として説明しない。

最終版では、区間和とbitsetの測定アダプタを全候補で常時inlineに揃え、RMQの操作ループは実APIを直接呼ぶ。
各結果の`environment.json`にadapter policyを記録し、runner・harness・headerのsnapshotとSHA-256を保存する。
古いwrapper版と新しい直接呼び出し版の差を、そのままライブラリの改善率にはしない。
測定時のHEADは`eaff599`を基準とする作業中の版であり、未commitの実装もある。
再現するソースはrevisionだけで判断せず、各測定ディレクトリに保存したbyte snapshotとhashで特定する。

## 候補の採否

- 区間和では、寄与の分岐をmaskへ変更し、固定4vectorの更新を展開する177行の版を採用。
  合流時break、2段内の専用経路、全段展開も試したが、入力・コンパイラ間の差とコード量を考慮して採用しなかった。
- RMQは16要素のprefix/suffix、ブロック上のSparse Table、SIMD構築、prodのinline化を採用。
  単調スタックmask版は短区間を改善しても広区間と構築を遅くするため棄却した。
- bitsetは一時配列のないshift-ORと256-bitのOR/XOR/ANDを採用。
  64bit以下の特別経路は小さい改善に留まり、他の条件の回帰もあったため棄却した。
  動的loopのunroll候補はGCCの`ignoring loop annotation`警告をエラー扱いにしてコンパイルに失敗し、公開しない。

初期・途中の結果は以下に残す。未完了の実行は全件成功とは扱わない。

| 記録 | 用途 |
| --- | --- |
| [sum-pilot](sum-pilot/summary.json) | mask・更新展開・全段展開の初回比較、wrapper版 |
| [sum-final](sum-final/summary.json) | 初回6構成、wrapper版 |
| [sum-break-comparison](sum-break-comparison/summary.json) | 合流時breakの対照 |
| [sum-two-level-pilot](sum-two-level-pilot/summary.json) | 2段内の専用経路 |
| [new-pilot](new-pilot/summary.json) | RMQ/bitsetの初回とSIMD prefix構築対照 |
| [rmq-mask-pilot](rmq-mask-pilot/summary.json) | 単調スタックmask候補 |
| [bitset-small-pilot](bitset-small-pilot/summary.json) | 64bit以下の特別経路 |
| [sum-direct-pilot](sum-direct-pilot/INTERRUPTED.txt) | 中断した実行、元の部分ログ保持 |
| [sum-direct-resume-pilot](sum-direct-resume-pilot/summary.json) | アダプタ展開後の区間和比較 |
| [rmq-direct-resume-pilot](rmq-direct-resume-pilot/summary.json) | 実APIでのRMQ候補比較 |
| [rmq-direct-confirm](rmq-direct-confirm/summary.json) | prefix構築のGCC/Clang再確認 |
| [bitset-direct-resume-pilot](bitset-direct-resume-pilot/summary.json) | アダプタ展開後のbitset比較 |

## 最終版の区間和

[生ログ](sum-direct-final/samples.jsonl)、[集計](sum-direct-final/summary.json)、
[環境・ソースhash](sum-direct-final/environment.json)、[失敗記録](sum-direct-final/failures.json)を保存した。
uint64の和をmodulo 2^64で扱い、add/set/queryを同じ順序で実行する。
point-addだけは更新をaddに限定して問題の操作に揃える。
境界Nと全64bitの値を含む独立したvector oracleも測定外で確認する。

以下はN=65536、Q=200000、point-add、portable O2 releaseの操作時間中央値。単位はms。

| 候補 | GCC | Clang |
| --- | ---: | ---: |
| 旧高速版 | 2.325 | 2.301 |
| 改善版 intrinsics | 2.087 | 2.254 |
| 改善版 手書きasm | 1.996 | 2.329 |
| 改善版 通常ループ | 5.702 | 5.334 |
| ACL Fenwick | 3.968 | 4.089 |
| ACL segtree | 9.664 | 11.085 |

Clangのparse時間には大きなばらつきが記録されたため、操作時間の短縮をそのままプログラム全体の短縮とは説明しない。
旧版とのend-to-end改善を主張する資料ではない。生のparse/build/operations/formatは上記ログで確認できる。

## 最終版のRMQとbitset

[生ログ](phase-direct-final/samples.jsonl)、[集計](phase-direct-final/summary.json)、
[環境・hash](phase-direct-final/environment.json)、[失敗記録](phase-direct-final/failures.json)を保存した。
各候補を小さい独立oracleと照合し、測定入力と最終出力のhashも一致させる。

RMQは32bit非負整数のvalue-only min、空でない半開区間、任意順のオンラインAPIに揃える。
比較用LinearRMQのargmin等の追加保証を、新実装が備えるという意味ではない。
以下はGCC O2 release、N=500000、Q=200000、広区間の中央値。単位はms。

| 候補 | 構築 | 操作 | parseからformatまで |
| --- | ---: | ---: | ---: |
| 採用RMQ intrinsics | 2.218 | 1.084 | 11.588 |
| 採用RMQ 手書きasm | 2.217 | 1.196 | 11.738 |
| 初版prefix通常ループ＋prod inline | 2.443 | 1.097 | 12.089 |
| Sparse Table | 7.884 | 1.203 | 19.176 |
| Disjoint Sparse Table | 15.306 | 1.686 | 25.270 |
| LinearRMQ | 4.922 | 2.838 | 16.183 |
| Sqrt Tree | 10.167 | 2.734 | 20.767 |

prefix/suffix構築のSIMD化は、同じ入力・同じprod inlineの対照に対して構築を短縮した。
追加コードは約60行で、採用後のheader全体は254行。mask版は同じブロックの照会が軽くなる一方、
広区間と構築の回帰が大きかった。入力・問い合わせ密度によって既存構造の方が向く。
特にN=1024の短区間200000件では、採用版の操作1.688msに対してSparse Tableは0.393msだった。
この条件で新RMQが速いとは主張せず、問い合わせ数が多い小配列では既存Sparse Tableを選ぶ。

bitsetはN=262144、Q=4000。DPは正のshiftを伴う`B |= B << k`、logicはOR/XOR/ANDを交互に行う。
std::bitsetは同じbit数をコンパイル時に持ち、新実装は長さを実行時に持つため、APIの条件を区別する。
以下はGCC O2 releaseの操作時間中央値。単位はms。

| 候補 | DP shift-OR | OR/XOR/AND |
| --- | ---: | ---: |
| 採用Bitset intrinsics | 3.257 | 2.859 |
| 採用Bitset 手書きasm | 3.331 | 2.867 |
| 採用Bitset 通常ループ | 8.630 | 4.275 |
| std::bitset | 15.175 | 2.065 |

一時配列を作らない融合shift-ORはこのDPで有効だった。
単純な論理演算や小さいbit数ではstd::bitsetが速い。手書きasmもintrinsicsへ常に勝つわけではない。
長さがコンパイル時に分かり、shift融合が不要ならstd::bitsetを使う選択肢を維持する。
Clang測定中は全候補のparseにも大きな時間変動があり、GCCとの絶対値比較を行わない。
各コンパイラ・profile内の生ログと候補順を確認できる状態で残す。

## 外部提出を含むプログラム全体の比較

[生ログ](whole-final/samples.jsonl)、[集計](whole-final/summary.json)、
[環境・入力と出力のhash](whole-final/environment.json)、[ビルド結果](whole-final/compile.json)、
[失敗33件](whole-final/failures.json)を保存した。
同じLC制約内の入力を通常ファイルから読み、独立したPython oracleの出力SHA-256と照合する。
285実行のうち255件が正解で、自前の新実装は3問題・2コンパイラの全90件が正解だった。
外部GCC JITの実行失敗30件とコンパイル失敗3件は成功や0msとして集計しない。

以下は各5回のwall時間中央値、単位はms。起動・parse・構築・操作・format・終了をすべて含む。
区間和はpoint-addとrange-sumを混ぜ、N=65536ではQ=200000、N=500000ではQ=500000とする。

| compiler | 入力 | 旧高速版 | 改善版 | 403014タグ補助版 |
| --- | --- | ---: | ---: | ---: |
| GCC | N=65536 広区間 | 10.135 | 9.700 | 7.277 |
| GCC | N=500000 広区間 | 34.972 | 33.530 | 21.263 |
| GCC | N=65536 短区間 | 9.919 | 9.831 | 7.983 |
| Clang | N=65536 広区間 | 12.973 | 12.908 | 7.170 |
| Clang | N=500000 広区間 | 35.322 | 35.102 | 17.951 |
| Clang | N=65536 短区間 | 11.694 | 11.163 | 7.710 |

403014の元ソースはGCC/Clangとも、使用中のlibstdc++に`std::from_range`がなくコンパイルできなかった。
[タグだけの補助header](compat-from-range.hpp.snapshot)を`-include`する候補を別名で測定した。
元ソースは変更せず、runtime処理も補わないが、無補完の提出そのものが動いたという結果ではない。
無補完の失敗は[元のGCCログ](whole-final/compile-g++-sum-lc403014-original.log)と
[Clangログ](whole-final/compile-clang++-sum-lc403014-original.log)に残す。

RMQのN/Qと区間分布は区間和の表と同じ。新実装と上位提出402567を比較した。

| compiler | 入力 | 新RMQ | 402567 |
| --- | --- | ---: | ---: |
| GCC | N=65536 広区間 | 9.485 | 5.044 |
| GCC | N=500000 広区間 | 30.798 | 14.785 |
| GCC | N=65536 短区間 | 9.700 | 6.035 |
| Clang | N=65536 広区間 | 11.142 | 5.551 |
| Clang | N=500000 広区間 | 51.300 | 18.357 |
| Clang | N=65536 短区間 | 13.350 | 7.278 |

362370もGCCでは全15件正解だったが、Clangではconstexpr配列の初期化で
[コンパイルに失敗](whole-final/compile-clang++-rmq-lc362370.log)した。
区間和・RMQとも上位提出との差は残る。新旧の自前driverは同じFastIOを使い、外部提出は元の専用I/O・
配置・処理順を維持している。全体の差をデータ構造だけ、またはI/Oだけの差とは断定しない。
phase測定の操作時間をそのまま差し引いてI/O時間を推定することもできない。
次に全体時間を縮める際は、同じdriverでI/Oと構造を独立に替えて支配項を確認する必要がある。

bitsetはGF(2)行列積のdriverで、選択したBの行を`xor_with`する。
GCCでは外部JITが実行失敗したため、正解したClangの比較を示す。

| 入力 | 新Bitsetのdriver | JIT 246187 | JIT 406212 |
| --- | ---: | ---: | ---: |
| 密な256×256×256 | 2.989 | 25.685 | 25.615 |
| 密な4096×4096×4096 | 264.410 | 36.243 | 34.374 |
| 疎な127×513×193 | 3.072 | 24.622 | 24.406 |

JIT版は入力寸法によらず固定4096全体を12行×256列のブロックで処理し、入力Aに応じて命令列を書き替える。
専用SIMD入出力と、複数行の更新を融合した処理も持つ。
最大の密行列ではその設計が有利であり、新Bitsetが行列積全体でも同等速度に達したという結果ではない。
一方、小行列・疎な長方形では固定4096を処理する費用が重い。
任意の動的長に使う短いbitsetへ、固定寸法や実行可能メモリを持ち込むことはしなかった。

GCCで両JIT提出が全30件SIGSEGVになった原因は、固定の機械語長・operand offsetと、
コンパイラが関数先頭へ加えた`endbr64`4byteとの不整合だった。
[別の機能診断](jit-diagnostic/diagnostic.json)で先頭命令を確認し、元ソースを変更せず
`-fcf-protection=none -fno-stack-protector`を追加すると両提出の1×1入力が正解になった。
さらに246187では、[CETだけを無効化した診断](jit-cet-only-diagnostic/diagnostic.json)でも正解になった。
これらは追加flagsを持つ別buildの機能確認であり、時間は比較していない。元の失敗記録も置き換えない。

inlineの補足として、[小さい自作の直接呼び出し](inline-diagnostic/direct.cpp.snapshot)を
GNU C++20・GCC O2 releaseでcompileした[コマンド・hash・結果](inline-diagnostic/diagnostic.json)も保存した。
`prod`のinline指定だけが異なるheaderを比べると、[指定前のassembly](inline-diagnostic/baseline.s)には
`prod`へのcallが残り、[指定後](inline-diagnostic/candidate.s)では消える。
AVX2専用の短区間処理へのcallは残る。これは直接呼び出しのcompile診断であり、
初期の測定アダプタを含む全体の差をこの1要因だけで説明する資料ではない。

## 再実行

結果は空の出力先へ保存する。過去の結果へ上書きしない。
runnerはPython、harnessはC++だが、依存解析の対象にしないため拡張子を`.snapshot`としている。
各出力ディレクトリ内の同名snapshotは当該実行の記録であり、トップのrunnerを更新しても保存したbytesは変えない。

{% raw %}
```sh
taskset -c 0 python3 docs/development/measurements/extra-fast-expansion/sum-runner.py.snapshot \
  --output .benchmark/extra-fast-expansion/local-sum --matrix \
  --header old=docs/development/measurements/extra-fast-expansion/sum-before.hpp.snapshot \
  --header new=blueberry/extra-fast/segment-tree.hpp

taskset -c 0 python3 docs/development/measurements/extra-fast-expansion/phase-runner.py.snapshot \
  --output .benchmark/extra-fast-expansion/local-phase \
  --rmq-control docs/development/measurements/extra-fast-expansion/phase-direct-final/control-header.hpp.snapshot

taskset -c 0 python3 docs/development/measurements/extra-fast-expansion/whole-runner.py.snapshot \
  --output .benchmark/extra-fast-expansion/local-whole --repeats 5
```
{% endraw %}

外部提出との全体比較には、source URLとhashを確認して元ファイルをgit管理外の`.benchmark/extra-fast/`へ保存する。
第三者ソースは配布していない。診断用from_rangeタグ補完はC++標準ライブラリの修正や公開APIではなく、
手元のGCC13環境で403014を測るためだけに用いる。元の無補完ビルド失敗も保存する。
