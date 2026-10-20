# 998244353専用FPSの測定

## 調査

2026-10-20のLibrary Checker公開REST APIを`+time`順に調べた。[取得時の情報](research.json)と各操作の表（[積](research-convolution.md)、[inv](research-inv.md)、[log](research-log.md)、[exp](research-exp.md)）を保存した。順位は動くため、提出時の最大ケース時間をローカルの合計時間と比較しない。

調査した[積409465](https://judge.yosupo.jp/submission/409465)、[inv409370](https://judge.yosupo.jp/submission/409370)、[log409452](https://judge.yosupo.jp/submission/409452)、[exp409662](https://judge.yosupo.jp/submission/409662)は約5,700〜6,900行。raw sourceはignoredの`.benchmark/extra-fast-more/fps`にのみ保存し、ライセンスを確認できないコードは転載しない。

上位提出は次の工夫を組み合わせている。

- NTTはradix-4/8の不完全変換と8係数の葉多項式積。定数twiddleの乗算にはShoup quotient、葉の積や添字による乗除にはMontgomery還元を使う。inline asmはレジスタ割り当てや計算順序を含む専用kernel全体を固定する。
- invは5変換のNewton更新で近似の変換を使い回し、積の必要な上半分だけを出力する。
- expは近似と逆元を維持し、最終Newton段を半分長の変換とblock計算で処理する。logは最大4blockでf'/fを直接求め、単純な全体の逆元と積を避ける。
- arena、入力領域の再利用、huge page、mmap/bulk SIMD入力、固定幅SIMD出力、preinit/_exitも含む。ソースの全体時間をNTT単体の速度と同一視できない。

今回の短い実装は、独立した完全radix-2 DIF/DIT NTTを8lane Montgomeryで処理する。inv/expは通常版でも利用している変換再利用のNewton反復を用い、logはdiff * invの積を積分する。全操作はcanonicalな`vector<uint32_t>`を受けて所有vectorを返し、CPU実行時検出とscalar fallbackを持つ。通常版の型・法の汎用性は変更しない。

## 比較条件

最終測定はGCC/Clang、gnu++20、`-O2 -DNDEBUG`、assert有効`-O2`、`-O3 -march=native -DNDEBUG`を分ける。同じcanonical uint32_t入力・所有vector出力にそろえ、通常FPSのmodintへの変換も、その逆変換も操作時間に含める。NTT表の構築、Montgomery変換、バッファ確保は各呼び出しの操作時間に含め、前計算を除いた速度比にはしない。

CPU0固定、seed20261020、各候補5反復で実行順を回転する。測定表のNは入力の係数数で、積はaがN項、bがN-1項、結果が2N-2項。級数演算は入力と結果がともにN項。N=64/1025/16384/131072/500000/131073のランダム係数に加え、N=131072で定数項以外0、または法-1の係数を測る。各操作の定数項は必要な条件に合わせる。入出力は全候補で同じFastInput/FastOutput、Linuxの`/tmp`に置く同一内容の通常ファイル。入力生成・出力hash計算は時間に含めない。

`operation_ms`は呼び出し全体、`parse_ms`と`format_ms`は入出力、`total_ms`はそれらの合計、`wall_ms`はプロセス起動・終了も含む外側の時間。raw phaseの`max_rss_kib`はfork親の常駐量を含むため無効とし、別memory phaseの値を採用する。intrinsicsとasmは同じ数学的処理で、scalarは明示的なSIMDを選ばない設定。native profileではコンパイラによる自動vector化までは禁止しない。

実行環境はIntel Core i7-14650HX、WSL2 Linux 5.15、GCC 13.3.0 / Clang 18.1.3。OS全体の負荷は統制していないため、他環境やcompiler間の絶対値を一般化しない。

初期[NTT pilot](pilot/README.md)は方向性を見るための測定で、別作業と重複した可能性がある。NTT表構築を分離しており、公開APIと同じ契約の速度比として採用しない。

## 候補の採否

[candidates](candidates/summary.json)はGCC/Clang、O2 release、同じcanonical vector契約で、NTT表・係数変換・作業領域も含めた比較。N=1025/131072/500000、4操作、4候補、各5回の480標本が全て一致した。

h=1,2,4の最終3段をscalarの短い蝶に分割せず、lane permutationで8係数単位にまとめる候補を採用した。h=1のtwiddleは1なので乗算も省く。大きい段のAVX2処理と共通のMontgomery乗算を使い、追加は約35行。前処理込みの操作時間は、代表的な大入力で約25〜35%短縮した。

逆rootを`w^-j = -w^(h-j)`から作る候補も評価したが、前処理を含む改善が安定せず、他の候補と組み合わせると大きい積で回帰したため棄却した。raw sample、4候補それぞれのsource、compiler commandは同フォルダに保存した。候補比較はfinalの通常版との速度比と区別する。

logの根表を作る変換長は、`diff(f)`の最大N-1項と逆元N項の積なので`bit_ceil(2N-2)`で足りる。初期版は2N-1を使い、N=2^k+1で根表を倍にしていた。[before-root-bound](before-root-bound/summary.json)の3360標本全一致を保存し、この1項分を直してN=131073も加えたfinalを実行した。数学的な結果と他の操作のアルゴリズムは不変。

最初のfull試行はACLを通常includeしたNDEBUG + Werrorでassert専用変数のunused警告がerrorになり、比較元4buildが失敗した。[interrupted-acl-warning](interrupted-acl-warning/INTERRUPTED.txt)に失敗と途中データを保存した。projectのWerrorを保ち、ACLのみ-isystemへ分離した全比較を再実行している。この途中runの欠測を0msにしない。

## 最終結果

[final](final/summary.json)は3840標本（32入力条件×2 compiler×3 profile×4 backend×5回）、全出力が一致し、compile失敗・実行失敗・時間field欠損は0。NTT表・変換・確保込みの操作で、4操作とも通常FPSから改善した。O2 release、N=500000のランダム係数、5回中央値、msを示す。通常版はcanonical uint32_tとのadapter変換も含む。

| compiler | 操作 | 通常FPS 操作 | intrinsics 操作 | asm 操作 | scalar 操作 | 通常FPS 合計 | intrinsics 合計 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| g++ | convolution | 37.770 | 17.998 | 18.323 | 45.456 | 57.203 | 37.795 |
| g++ | inv | 54.686 | 23.964 | 24.305 | 67.247 | 64.673 | 34.049 |
| g++ | log | 97.096 | 41.226 | 39.970 | 112.314 | 107.119 | 51.484 |
| g++ | exp | 94.769 | 39.588 | 40.125 | 110.264 | 105.071 | 50.489 |
| clang++ | convolution | 39.469 | 16.714 | 16.484 | 41.795 | 65.407 | 39.928 |
| clang++ | inv | 59.768 | 20.460 | 20.663 | 61.311 | 72.326 | 32.410 |
| clang++ | log | 98.434 | 35.863 | 35.959 | 99.963 | 110.283 | 47.178 |
| clang++ | exp | 101.780 | 35.212 | 35.464 | 102.404 | 114.151 | 46.683 |

この条件でintrinsicsの操作時間はGCC約2.10〜2.39倍、Clang約2.36〜2.92倍の速度。native/assert有効の結果、分布別・小入力・冪+1の結果はsummaryとraw sampleに全て残す。asmを挿すだけでintrinsicsから一律に速くなる結果ではなく、主な改善は8laneのMontgomery演算と小段のshuffle。

N=64のinvは通常版の直接計算が有利で、SIMD版にも前処理の負担がある。明示SIMDを使わないscalar版も大きい入力で通常版より遅いことがある。小さいinvやAVX2非対応環境を含め、通常版と使い分ける。

### メモリの別測定

finalの`max_rss_kib`はPythonのfork親の大きい常駐量を含み、全候補が約230MiBとなってしまった。このfieldはメモリ比較に使わない。時間fieldと出力照合は有効なまま保存し、[memory-final](memory-final/summary.json)で同じbinary bytesをLinux/tmpへコピーして再測定した。小さい`/usr/bin/time` helperをexecしてから対象をforkするため、Python親の常駐量を対象のpeakに持ち込まない。O2 release、2 compiler×4 backend×2入力長×4操作×5回の320標本が全て元の出力と一致。

GCC、N=500000、KiBの中央値。作業領域だけでなく入力・結果・ランタイムも含む。

| 操作 | 通常FPS peak RSS | intrinsics peak RSS |
| --- | ---: | ---: |
| convolution | 23120 | 25432 |
| inv | 13348 | 17464 |
| log | 25092 | 25560 |
| exp | 23712 | 26208 |

時間は短縮したが、全長の正逆rootを持つためRSSは増えている。特にinvは約4MiB増。型や結果の所有を変えてメモリ削減を装わない。

### 上位提出の全体比較

[whole](whole/summary.json)は第三者の原提出を変更せず、GCC/Clang、gnu++23、O3 native NDEBUG、Linux/tmpの同じ保存先のbinary・入力・出力で5回ずつ比較した。3入力長×4操作×2 compiler×2 candidate×5回の240標本が全て一致し、失敗0。正しさはphaseの通常版との全係数照合・直接計算テストに加えて、wholeでは原提出との整数token列も照合する。自作版はFastIO、原提出はそれぞれの専用I/Oとarenaを保つので、kernel単体の比較ではない。

N=500000、プロセス全体wallのms中央値。

| compiler | 操作 | 自作版 whole | 原提出 whole | 自作版 RSS KiB | 原提出 RSS KiB |
| --- | --- | ---: | ---: | ---: | ---: |
| g++ | convolution | 39.526 | 12.315 | 25488 | 18912 |
| g++ | inv | 32.336 | 12.917 | 17492 | 11620 |
| g++ | log | 48.378 | 14.726 | 25588 | 15680 |
| g++ | exp | 41.433 | 14.758 | 26280 | 15772 |
| clang++ | convolution | 42.095 | 12.657 | 25404 | 19756 |
| clang++ | inv | 32.957 | 12.712 | 17408 | 11580 |
| clang++ | log | 47.859 | 14.357 | 25572 | 15700 |
| clang++ | exp | 39.723 | 14.691 | 26124 | 15700 |

原提出にはまだ約2.5〜3.3倍の全体差がある。Shoup twiddle・葉の直接積・半分だけを出す変換・exp最終段の小さい変換・logのblock算法・arena・専用I/Oが残る差の候補。各々の寄与率をwholeの値から割り当てることはできない。今回の短い完全radix-2版では大きな専用kernel群を取り込まず、通常FPSと同じcanonical入出力契約での改善を確認して採用する。

phase後の最初のwhole実行中に、mainのPR54（Contest FastIO）が統合された。phaseのFastIO SHAは60366b...、最終wholeはb8b9c2...で、整数read/write経路は維持されるがbyte sourceは異なる。両者の時間を同一条件の前後比較としてまとめない。原本の[旧FastIO snapshot](fast-io-before-pr54.hpp.snapshot)と[新snapshot](fast-io-after-pr54.hpp.snapshot)を保存した。更新mtimeと最初のwhole GCC compileが5秒重なった240標本は[whole-io-transition](whole-io-transition/TRANSITION.txt)として保存し、最終wholeは新FastIOで全て再実行した。

### 検証と共通マクロ

[validation](validation/results.json)の26確認（GCC/Clang、gnu++20/23、intrinsics/旧asm/Intel、実行時scalarとSIMD、両compilerのASan/UBSan、単独・重複include、4公式verifyコードのcompile）は成功。seed20261020で、直接計算との境界比較と通常FPSとの大きい入力の全係数照合を行った。公式judgeの実行は統合側で別途記録する。

公開直前に共通`BLUEBERRY_EXTRA_FAST_USE_ASM`と`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`へ合わせた。旧測定用INLINE_ASMは同じasmの別名として維持し、アルゴリズム・命令列を変更していない。[common-macros](common-macros/results.json)には差分・旧新snapshotと、GCC/ClangのNDEBUGでdefault/旧asm/共通asmのプリプロセス出力がbyte一致する証拠を残す。最終測定source SHA17e342...に対する変更はこの2つの条件式だけ。新sourceで共通asm/Intel/force-scalar/force-scalar+asmをC++20/23で確認し、追加ASan/UBSanを含む17確認が成功した。


## 再実行

各フォルダの`*.snapshot`を元の拡張子で作業用ディレクトリへ復元する。`phase.cpp`とrunnerは自作で、外部提出の原本は含めない。`environment.json`のcompiler/flags/source SHA、各compile log、raw samples、summary、failuresを併せて確認する。異なるソースやadapterの結果を同一の前後比較としてまとめない。

- 候補比較: `.benchmark/extra-fast-more/fps/candidates.py`をworktree rootから実行。
- 公開API比較: `.benchmark/extra-fast-more/fps/run.py`を実行。
- RSSの独立測定: `.benchmark/extra-fast-more/fps/memory.py`を実行。前phaseの同じbinary bytesを使う。
- 共通マクロ/同値性検証: `.benchmark/extra-fast-more/fps/common-macros.py`を実行。
- 上位提出全体比較: `.benchmark/extra-fast-more/fps/whole.py`を実行。公開REST APIから元提出をignored作業領域へ別途取得する必要がある。

失敗・未実行は`failures.json`とlogに残す。成功時間を取得できなかった候補へ0msを補わない。
