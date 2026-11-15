# FastSet と Predecessor Problem 上位提出の比較手順

このディレクトリには比較手順と測定結果を保存します。Library Checkerの
[Predecessor Problem](https://judge.yosupo.jp/problem/predecessor_problem) の公開提出
[403023](https://judge.yosupo.jp/submission/403023) または [278227](https://judge.yosupo.jp/submission/278227)
と、現在の `verify/data-structure/fast-set.test.cpp` を同じ入力、コンパイラ、最適化設定、実行環境で比較します。
ACLに同等の固定宇宙predecessor集合はありません。

対象提出のライセンスが確認できないため、ソースをリポジトリには保存しません。実行時に公開APIから一時ディレクトリへ取得し、記録済みSHA-256を確認してからコンパイルします。`--external` で手元の取得済み原本を使うこともできます。CRLFのファイルはLFに変換し、変換前後のhashを保存します。提出コードは外部から取得した実行可能コードなので、機密情報を渡さない使い捨て環境で実行してください。

提出403023はC++23とAVX2を必要とし、regular fileの標準入力と単一区切りの数値レコードを前提とする専用I/Oを使います。生成データはこの条件を満たします。両者に `-std=gnu++23 -O2 -DNDEBUG -mavx2 -mpopcnt` を使い、Clangでは両者に `-fconstexpr-steps=100000000` も追加するため、比較結果はこのCPU・コンパイラ設定に限定されます。AVX2またはPOPCNTがない場合やコンパイルが失敗した場合、結果JSONを失敗として残し、未実行を0にはしません。C++23の `std::format` などを提供する標準ライブラリも必要です。

## 実行

Linux上の `/usr/bin/time`、Python 3、C++23対応コンパイラが必要です。API接続が可能な隔離環境で、リポジトリ直下から実行します。

```sh
python3 docs/development/measurements/fast-set-lc-20261116/run.py \
  --submission 403023 --compiler g++ --runs 3 \
  --output .verification/fast-set-lc/results.json
```

既存の出力は上書きしません。提出原本が既にある場合は `--external /path/to/submission.cpp` を付けます。既定はN=1,000,000、Q=500,000で、`--n 10000000 --q 1000000` まで公式制約内で変更できます。全1ケースの独立oracleは `std::set` を使うため、最大Nは多くのメモリと時間を必要とします。`--timeout` はコンパイル、入力生成と各実行のそれぞれに適用されます。

## 記録と解釈

- 固定seedで空、全1、半分、疎、256文字単位の塊、更新中心の6分布と、1・63・64・65・4095・4096・4097の境界を生成します。各queryは `0 <= x < N` です。
- 生成時の独立した `std::set` による期待出力と、候補・提出・段階計測の出力をすべてバイト単位で比較します。生成とoracleの所要時間はまとめて記録し、実行時間へ含めません。
- 候補と提出は同じ入力をregular fileから読み、通常のまま標準出力へ書きます。各プロセスのwall time、peak RSS、exit status、stderr、出力hashを記録し、warmup後の中央値と最小値を示します。実行順は回ごとに入れ替えます。実行可能CPUの先頭へのaffinity固定を試み、元の候補、選択CPU、成否を記録します。固定できなかった場合もその旨を記録します。RSSが取得不能または0の場合は欠測として記録し、測定全体を失敗にします。
- `phases.cpp` は候補の追加計測用です。全queryを先読みし、parse、構築、操作、出力を分けます。このプログラムは本来のストリーミング処理とメモリ配置が異なるため、各phaseの合計を本来のプロセス時間や提出側の分解値として扱いません。I/O実装自体も提出と候補で異なるため、end-to-end差はデータ構造だけの差を意味しません。
- 結果にはhash、flags、コンパイラ、CPU、ケース、すべての試行、生成時間、失敗を保存します。提出ソースは一時ディレクトリの削除とともに消えます。実行環境や入力が違う既存測定と数値を混ぜません。

この測定は合成入力の比較です。公式ケースでの検証や、提出のI/O・データ構造を独立に差し替えた比較は別途必要です。

## 実行結果

[GCC初回](fastset-linux-gcc-initial.json)はGCC 15.2.0、Intel Xeon 6975P-C、[Clang再実行](fastset-linux-clang-retry.json)はClang 21.1.8、Intel Xeon Platinum 8573Cです。どちらも選択した1つのCPUへのaffinity固定に成功しました。各環境内で候補・提出・段階計測は同じ入力とコンパイラ設定を使い、6分布と7境界の全13ケースにおいて、warmupを含む全出力が独立した `std::set` oracleと一致しました。計測は各ケースでwarmup後3回です。表はN=1,000,000、Q=500,000のプロセス全体のwall time中央値で、括弧は候補÷提出の比です。

| 分布 | GCC 候補 / 提出 (ms) | Clang 候補 / 提出 (ms) |
| --- | ---: | ---: |
| 空 | 17.82 / 9.15 (1.95倍) | 26.07 / 11.70 (2.23倍) |
| 全1 | 16.78 / 7.85 (2.14倍) | 24.77 / 9.43 (2.63倍) |
| 半分 | 17.92 / 7.94 (2.26倍) | 26.03 / 9.60 (2.71倍) |
| 疎 | 17.89 / 9.12 (1.96倍) | 26.20 / 11.63 (2.25倍) |
| 塊 | 18.36 / 8.55 (2.15倍) | 26.83 / 10.72 (2.50倍) |
| 更新中心 | 15.10 / 6.40 (2.36倍) | 22.43 / 8.26 (2.72倍) |

同じ6分布のpeak RSS中央値は、GCCで候補5,508〜5,560KiB、提出8,744〜8,856KiB、Clangで候補5,380〜5,416KiB、提出8,756〜8,852KiBでした。RSSはプロセス全体の最大値です。コンパイラとCPUが異なるためGCCとClangの数字を直接比較しません。

段階計測版では、6分布のparse中央値がGCCで9.58〜9.93ms、Clangで14.12〜14.32msでした。この時間には初期文字列・全queryの読み込みとquery配列の確保を含みます。通常版と処理順序・メモリ配置が異なり、提出側のphaseを計測していないため、これだけで上位提出との差をI/Oやデータ構造に割り当てることはできません。通常版では両者のI/O実装も異なります。どの部分が差を生むか、同じ環境でI/Oとデータ構造を個別に替える追加調査が必要です。

[Clang初回](fastset-linux-clang-initial.json)は候補をコンパイルできましたが、提出のコンパイルがconstexpr評価のstep上限で失敗し、測定は未実行です。初回実行のrunnerを[run-initial.py.txt](run-initial.py.txt)としてバイト単位で保存しています。現runnerはClangに限り、候補・提出・段階計測・入力生成のすべてに `-fconstexpr-steps=100000000` を追加します。Clang再実行ではこの設定を用い、初回の失敗は別のJSONに保持しています。

GitHub Actionsの[初回ログとartifact](https://github.com/blueberry1001/Blueberry-library/actions/runs/43728245358)、[Clang再実行ログとartifact](https://github.com/blueberry1001/Blueberry-library/actions/runs/43728565656)も保存しました。いずれも測定専用の一時PRであり、required CIとは別です。
