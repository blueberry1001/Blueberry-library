---
title: LC数列3種の追加と性能調査
---

## 対象と契約

LC対応表で未対応だった`bernoulli_number`、`bell_number`、`partition_function`を追加しました。3関数とも`function<Mint>(n)`で添字0からnまでを返します。既存のMontmort数列と同じ、最後の添字を含むAPIです。Bernoulli数はB₁=-1/2とします。

Mintには素数法pのACL `static_modint`を使います。共通の前提は`0 <= n < p-1`と、n+1以上の最小の2冪がp-1を割ることです。FPSの`inv/exp`が要求する`n+1 < p`も含めた契約にしています。3種類とも最悪時間O((n+1) log(n+2))、メモリO(n+1)です。n=0でも1項の`{1}`を返します。

## 公式仕様と公開実装の調査

[Bernoulli数](https://judge.yosupo.jp/problem/bernoulli_number)、[Bell数](https://judge.yosupo.jp/problem/bell_number)、[分割数](https://judge.yosupo.jp/problem/partition_function)はいずれも0..Nの全項を出力し、法は998244353、最大Nは500000です。公式問題定義とBellのパラメータを`benchmark/lc-expansion-fps-sequences/research/`へ保存しました。

2026年10月17日に公式REST APIのFastestから上位提出のソースを取得しました。取得結果は`benchmark/lc-expansion-fps-sequences/fastest-*/`に、ソースはgit管理外の`.build/fps-sequences/research-sources/`に保存しました。追跡対象にはリンク・SHA-256と測定結果を残します。順位の時間はYosupoの最大ケース時間であり、ローカル測定とは混ぜません。

| 数列 | 調べた提出 | 数列生成の方法 | 性能に関係する実装条件 |
| --- | --- | --- | --- |
| Bernoulli | [405758](https://judge.yosupo.jp/submission/405758)、[383856](https://judge.yosupo.jp/submission/383856)、[17797](https://judge.yosupo.jp/submission/17797)、[65329](https://judge.yosupo.jp/submission/65329) | 1/(i+1)!の級数を反転しi!倍 | Rust/C++の専用modintとNTT、65329は固定上限の階乗表 |
| Bell | [364958](https://judge.yosupo.jp/submission/364958)、[406022](https://judge.yosupo.jp/submission/406022) | 逆階乗からexp(x)-1を作りexpを1回 | 専用modint/NTT、406022はCPU target指定 |
| partition | [207699](https://judge.yosupo.jp/submission/207699)、[406026](https://judge.yosupo.jp/submission/406026) | 五角数定理の疎な母関数を反転 | AVX2必須の8要素ベクトル演算とMontgomery表現、専用I/O |

[Nyaanの有名な数列](https://nyaannyaan.github.io/library/fps/fps-famous-series.hpp)も確認しました。BernoulliのEGFと五角数定理による分割数を公開しており、同じ母関数の選択です。公開コードは調査用に保存し、追加ヘッダは式から独立に実装しました。ACLが提供しない数列生成だけを追加し、NTTを再実装せず既存の`FormalPowerSeries`へ委譲しています。

## 比較候補と判断

Bernoulliでは各逆階乗を個別の逆元演算で作る候補と、階乗1回の逆元から逆順に作る候補を比較しました。Bellではexpを2回呼ぶ直接の式と、内側のexpを既知の逆階乗で直接生成する方法を比べました。分割数では五角数漸化式のO(n√n) DPと、約数和からlog母関数を作ってexpするO(n log n)の候補も測りました。採用した処理はそれぞれ階乗の逆元1回、exp1回、五角数母関数のinv1回です。

`benchmark/lc-expansion-fps-sequences/compare.cpp`は前計算を含む数列全項の生成、返り値の走査、出力の文字列化、文字列の解析を分けます。返り値は全候補で同じ長さ・係数型・順序です。N=8192、131072、500000を各3回測り、重み付き全項checksumを比較します。`run.py`が生ログ・中央値・最小値・process時間・`/usr/bin/time`の最大RSSを保存します。文字列化と解析はメモリ上の測定で、ファイルや端末の転送時間は含みません。

公開C++提出は`fastest-local.py`で別にコンパイルし、同じGCC・言語標準・最適化フラグ、同じ入力を使います。ライブラリのverifyプログラムも同時にビルドし、全項出力を同じ方法で回収して比較します。公開ソースのtarget pragmaや固定法・上限は保持し、汎用APIと同じ保証として扱いません。コンパイルできない候補はエラーログを残し、測定値を0にはしません。

### 生成時間の結果

Intel Core i7-14650HX、WSL2 Ubuntu、GCC 13.3、`-std=gnu++20 -O2 -DNDEBUG`で測定しました。全81サンプルは`benchmark/lc-expansion-fps-sequences/raw.json`、中央値と最小値は`summary.csv`、環境は`environment.json`に保存しています。N=500000での生成時間は次の通りです。queryは単なる全項走査で、個別の更新操作はありません。

| 生成法 | 3回の中央値ms | 最小ms | 最大RSS KiB |
| --- | ---: | ---: | ---: |
| Bernoulli・階乗逆元1回 | 50.398 | 50.376 | 11244 |
| Bernoulli・逆元を各項で計算 | 81.681 | 79.818 | 11268 |
| Bell・逆階乗直接生成とexp1回 | 87.969 | 87.763 | 21660 |
| Bell・exp2回 | 167.287 | 165.788 | 22952 |
| partition・五角数母関数inv | 47.804 | 47.632 | 11252 |
| partition・五角数DP | 789.362 | 788.181 | 10464 |
| partition・約数和とexp | 94.484 | 93.227 | 23552 |

Bernoulliの係数生成で繰り返していた逆元、Bellの既知のexp(x)の計算を省きました。分割数は五角数のO(√n)個の非零係数だけを作ってinvへ渡します。約数和の走査やexpを使う候補よりも時間とメモリが小さい結果でした。型・返す項数・呼び出し順の制約は全候補で同じです。

### Fastestとの比較と制約

BernoulliのC++提出17797と65329は3サイズ各3回で全項checksumが一致しました。N=500000のend-to-end中央値は17797が57.899ms、65329が67.489ms、初期iostream版が78.613msです。17797は同じ3回でも最小46.071ms・最大156.494msと揺れがあり、微小差から優劣を決めません。専用modint・NTTとI/Oの組合せであるため、数列生成式だけの差にもできません。

Bellの406022は`std::views::zip/enumerate`を使い、このGCC 13/C++20条件ではコンパイルできませんでした。`compile-406022.log`に診断を保存し、時間は未測定としています。

partitionの207699は、初回のpipe入力で全9回がsignal 11になりました。ソースの入力処理は`fstat(stdin)`のファイル長で`mmap`する方式で、サイズ0のpipeに対応していません。失敗ログは`fastest-local.json`へ残し、通常ファイルをstdinに渡す比較を別に行いました。ソース自体は修正していません。

I/Oの予備測定では、N=500000項のメモリ上の文字列化がstringstreamで14.956ms、to_charsで6.189msでした。新しい専用I/O処理は追加せず、verifyの3本で既存の`FastInput/FastOutput`を使いました。最初の別ラウンド測定`own-fastio.json`ではBellのend-to-endが大きく揺れたため、変更前後の速度比には使いません。`io-driver.cpp`で同じ数列生成を行い、既存I/Oとiostreamを交互に測りました。

`final-file-io.py`は全driverへ同じ通常ファイルをstdinとして渡し、3サイズ・各3反復を交互に実行しました。公開partition提出も含めた63回は全係数が完全一致しています。N=500000の結果は次の通りです。`final-file-io.json`に全サンプルを保存しました。

| driver | end-to-end中央値ms | 最小ms | 最大RSS KiB |
| --- | ---: | ---: | ---: |
| Bernoulli・iostream | 97.427 | 92.045 | 11248 |
| Bernoulli・既存FastInput/FastOutput | 95.952 | 65.187 | 11504 |
| Bell・iostream | 156.243 | 143.902 | 20316 |
| Bell・既存FastInput/FastOutput | 126.774 | 110.525 | 21784 |
| partition・iostream | 92.161 | 81.172 | 11264 |
| partition・既存FastInput/FastOutput | 77.566 | 65.867 | 11448 |
| partition・公開提出207699 | 82.924 | 45.890 | 11356 |

全3種類で既存Fast I/O版の中央値は悪化せず、追加verifyも同じI/Oを再利用する形にしました。Bernoulliの1.5ms差はばらつきの範囲と扱います。公開partitionの最小値は速いものの、中央値ではこの環境で数倍の差は再現しませんでした。専用NTT・Montgomery表現・AVX2による差は、ACLを置き換える理由にはしていません。今後の比較でも通常ファイル入力という条件を明示する必要があります。

担当間で計測時間帯を分けましたが、同じ端末の別chatでC++コンパイルと検証が動作していることを確認しました。ログはこの共有環境の測定であり、占有CPUでの最速値を保証しません。

## 検証

### クリーン環境からの測定手順

リポジトリのルートで以下を実行します。`prepare.py`は比較に使う4提出をID指定で取得し、
保存済みSHA-256との一致を確認してから、数列生成の比較プログラムをビルドします。
外部ソースは`.build`内にだけ保存します。

{% raw %}
```bash
make setup-acl
python3 benchmark/lc-expansion-fps-sequences/prepare.py
python3 benchmark/lc-expansion-fps-sequences/run.py
python3 benchmark/lc-expansion-fps-sequences/fastest-local.py
python3 benchmark/lc-expansion-fps-sequences/final-file-io.py
```
{% endraw %}

`fastest-local.py`が外部提出207699をコンパイルしたあとに`final-file-io.py`を実行します。
前者のpipe入力での失敗と、後者の通常ファイル入力での成功は別の結果です。
既存ログを残すには別checkoutで実行し、入力・コンパイラ・フラグ・revisionが異なる結果を
今回の測定と同じ条件として比較しないでください。

`tests/random/fps_sequences.cpp`ではBernoulliの二項係数漸化式、Bellの集合分割漸化式、分割数のunbounded knapsackと比較します。0・1、2冪の前後、FPSの小入力分岐の前後、異なる素数法257と法2の0項を含みます。引数seedを受け取り、不一致時はseed・法・n・数列・添字・期待値・実測値を出力します。

3つのverifyは公式checkerで各11ケース・3反復を完了しました。結果は`benchmark/lc-expansion-fps-sequences/official/current.json`に、詳細ログはgit管理外の`.build/fps-sequences/official.log`に保存しています。公式データの実行時間はプロセス起動・I/Oを含み、core測定とは区別します。公式検証の間は他担当の処理があり得るため、性能候補の採否には後の専用計測を使います。

コンパイラ行列の初回実行では、PowerShellからWSLへのシェル変数の引き渡しが失敗しました。`compiler-tests-initial-shell-failure.log`を残し、変数を展開しない専用の`compile-test.sh`へ修正しています。

releaseの単独include確認では、`-I.`経由でACLを一般ヘッダとして読んだため、ACL内部のassert専用変数への警告で`-Werror`が失敗しました。`compiler-tests-initial-acl-warning.log`を残しています。自作ヘッダを`-iquote .`、ACLを`-isystem .deps/ac-library`に分け、ACL由来の警告と追加コードの警告を分離しました。

`compile-test.sh`の再実行はGCC 13.3/Clang 18.1とgnu++20/gnu++23の全4構成で成功しました。各構成でseed 20261017と731の愚直比較、各ヘッダの単独・二重includeのreleaseビルド、3本のverifyの`-Wall -Wextra -Wshadow -Werror`ビルドを確認しています。結果は`compiler-tests.log`にあります。この後のI/O変更と小さい法の最大境界追加は、統合担当の全体検証で確認します。
