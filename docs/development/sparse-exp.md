---
title: 疎なFPS指数関数の追加・性能比較
---

# 疎なFPS指数関数の追加・性能比較

2026-10-19。Library Checkerの未対応問題から
[Exp of Formal Power Series (Sparse)](https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse)
に対応する `sparse_exp(terms,n)` を追加しました。
ACLのmodintを利用し、指数関数の係数は `g' = f'g` から独立に実装しています。
既存APIの変更はありません。疎な逆元・対数は既存の疎除算などを組み合わせられるため、
今回は追加していません。

入力は次数と係数の組で、順序・重複・相殺を許します。定数項の合計は0、
法pは素数、要求項数は `0 <= n < p` です。NTT対応法である必要はありません。
Kを入力数、Sを正規化後の有効な非零項数として、
最悪時間O(K log(K+1)+n(S+1))、追加メモリO(n+K)です。
所有する長さちょうどnのFPSを返します。詳細は[APIページ]({{ '/blueberry/math/sparse-exp.hpp.html' | relative_url }})を参照してください。

## 比較条件と再現資料

WSL2 / Intel Core i7-14650HX、GCC 13.3 / Clang 18.1で実行しました。
性能測定はCPUコア2に固定し、各入力・実装を直列に1回warmup、3回測定しました。
初期版は `dd06a63`、採用した最適化は `cee0f9e` です。

測定用コピーは当初main `e5778d8` をHEADとしていたため、初期・最適化のmetadataには
そのrevisionが残っています。実際にコンパイルしたヘッダは各組のSHA-256と
`source.hpp.txt` に保存しました。HEADだけで測定ソースを識別しません。
初期・最適化の測定器原本も `runner.py.txt` に保存しています。
公式検証の最終記録はコミット `cee0f9e` のソースに対するものです。
原本比較とphase診断の間では、依存ヘッダのCRLFをLFへそろえています。
改行を正規化した内容は同じですが、元のhashと各組の結果は分けて保持しています。

- [測定器と再実行方法](https://github.com/blueberry1001/Blueberry-library/tree/main/benchmarks/sparse-exp)
- [初期版の全サンプル・環境](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/sparse-exp/initial)
- [最適化後の全サンプル・環境](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/sparse-exp/optimized)
- [公式入力での原本比較](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/sparse-exp/official-comparison)
- [実際の入出力を使う内訳診断](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/sparse-exp/phases)
- [公式checkerによる検証](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/sparse-exp/official-validation/report.md)
- [検査ログと初回失敗記録](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/sparse-exp/validation)

各組を別の測定として保存し、異なる入力・flags・ソースの結果を合算しません。
成功した実行の中央値・最小値、全反復、入力・出力・ソースのhash、compiler、RSSを保存します。
RSSはプロセス全体で、測定器の入力文字列や出力文字列なども含みます。
失敗・欠測を0に置き換えていません。第三者のソースと測定バイナリは収録していません。

## 同じAPI契約での改善

この表は `-std=gnu++20 -O2 -DNDEBUG`、メモリ上の同一入力を使った
**API呼び出し全体の中央値（ms）**です。正規化・結果配列・逆元前計算・漸化式を含みます。
入出力・プロセス起動は含みません。通常の行はn=500,000です。

| 入力 | GCC 初期→最適化 | Clang 初期→最適化 |
| --- | ---: | ---: |
| 1項、低次 | 5.566 → 5.752 | 5.700 → 5.587 |
| 10項、低次 | 9.432 → 6.680 | 8.040 → 6.658 |
| 10項、広く分散 | 7.171 → 6.369 | 6.682 → 6.147 |
| 100項、低次 | 53.929 → 40.161 | 45.018 → 28.419 |
| 100項、広く分散 | 32.284 → 20.714 | 26.797 → 16.694 |
| 10項、末尾付近 | 2.389 → 0.401 | 2.262 → 0.413 |
| n=500、499項 | 0.126 → 0.103 | 0.098 → 0.066 |

採用した変更は次の2点です。

- 正規化後の項が4個以上なら、uint64に収まる個数の積をまとめて剰余を取ります。
  個数上限は `(p-1)^2` から計算します。p=2,147,483,647では4積までであり、
  その上限と大きな係数を愚直比較・sanitizerで確認しました。
- 最小次数dが `d >= n-d` を満たすときは、f²が要求範囲で0なのでexp(f)=1+fです。
  逆元表と漸化式を省きます。判定で2dを直接計算しないため、次数の加算overflowも避けます。

初期の一律batch候補は1項や高次だけの入力で遅くなりました。
少数項のMintによる経路を残し、高次だけの場合には上記の省略を適用しました。
1項のGCCでは約3%の増加が残っています。一律にすべての入力で速くなるとは主張しません。

逆元を係数ごとに累乗で求める比較候補も測定しました。
50万項・低次10項のGCCでは、独立した逆元表版の前計算1.696ms、漸化式7.606msに対し、
係数ごとの除算版の漸化式は41.156msでした。
最終APIの6.680msは、この前計算を省略して報告した値ではありません。
各phaseの中央値の和は、全体の中央値と一致するとは限りません。

## Fastest原本との差

公開REST APIから取得した上位3提出の記録とソースhashを
[保存](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/sparse-exp/fastest/report.md)しました。
実行比較と詳細調査には、取得時1位の
[409367](https://judge.yosupo.jp/submission/409367)を使いました。
原本は固定法998244353・最大10項・AVX2の漸化式、mmapとhuge page、
25,600係数ごとの処理、履歴に応じたring buffer、SIMDによる整数整形、
C++ runtime初期化前の実行を使っています。
任意の素数法、重複項の正規化、全係数を所有して返す今回のAPIとは契約が異なります。
原本のコードはライブラリへ転記していません。

以下は**同じ公式入力・compiler flags・CPUコアでのプロセス経過時間の中央値（ms）**です。
flagsは両者とも `-std=gnu++20 -O2 -DNDEBUG -mavx2 -mbmi -mbmi2`。
この比較にだけISA指定を使い、公開ヘッダの利用条件には加えていません。
全出力を公式の期待値と整数として照合しました。

| 公式入力 | GCC Blueberry / 409367 | Clang Blueberry / 409367 |
| --- | ---: | ---: |
| max_random_00 | 13.652 / 3.585 | 13.449 / 4.041 |
| min_K_00 | 3.158 / 1.923 | 3.260 / 1.980 |
| small_dense_00 | 22.467 / 6.957 | 23.988 / 7.388 |

max_random_00は100万項・非零10項、small_dense_00は100万項・低次4項です。
前節の人工入力とは別の測定です。Yosupo側の最大ケース時間とも直接比較しません。

同じFastInput/FastOutputを使う別の診断では、GCCのmax_random_00の計算が8.459ms、
整形・flushが3.268msでした。small_dense_00では計算11.105ms、整形・flush9.333msです。
Clangでは順に7.646/3.481ms、12.523/10.854msでした。入力のparseは各約0.006〜0.008msです。
phaseは個別に計時しており、別実行の中央値同士を引き算した推定ではありません。

残る差には、係数計算と整数整形の両方が関わります。
汎用計算では逆元前計算と係数へのアクセス、入出力では整数整形が次の調査対象です。
固定法のSIMDやring bufferによる原本の速度を、そのまま全係数所有APIへ適用できるとは扱いません。
同等契約で評価した今回の候補は上記の表に示し、残る原本との差も保持します。

## 検証

- `make check`（GCC / gnu++20）成功。147 unit tests、112文書・113実行例、
  全header・verifyのcompile、全random test各20 seedを実行しました。
- 最適化後の新規random testはGCC / Clang × gnu++20 / gnu++23で各20 seed成功。
  独立した冪和展開による愚直解、対応法では密FPS、重複・相殺・空・切り詰め、
  法2/17/998244353/2147483647を含みます。ASan/UBSanでもseed42を確認しました。
- 最終実装のreleaseインスタンスと重複includeも4構成で検査しました。
- `make test`を最終実装で再実行しました。
- `make verify`と共通の `scripts/verify_with_metrics.py` で対象を絞り、
  新規ドライバ25ケースと一括入口9ケースを各3反復して公式checkerに成功しました。
  残る147本を再計測したという意味ではありません。
- `make docs`、production設定のJekyll build、`scripts/check_site.py --without-metrics`を実行しました。
  ローカルでは今回のAC記録を既存のソース有効な記録へ加え、測定表の全体再計測とは区別しています。

初回のincludeパス未設定、テストのClang警告、NTT非対応法で比較用の密FPSを
実体化したことによるcompile失敗を別ログに残しました。テストを修正し、上記の構成で再実行しています。
Jekyllの最初の実行ではproduction指定がなくGA検査に失敗したため、元のログを残して
production設定で再ビルド・再検査しました。いずれも初回の失敗を成功として扱いません。
未コミットの新規verifyはimmutableなソース記録に存在せず、初回のAC記録保存が拒否されました。
このログも保持し、実装コミットと作業コピーを一致させてから公式検証・記録保存を再実行しています。
