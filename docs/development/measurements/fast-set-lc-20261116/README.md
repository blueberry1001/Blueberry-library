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

## 初回実行

[GCC初回](fastset-linux-gcc-initial.json)はGCC 15.2.0、Intel Xeon 6975P-C上で13ケースすべての出力がoracleに一致しました。N=1,000,000、Q=500,000、warmup後3回の大きい6分布の中央値は、候補15.10〜18.36ms、提出6.40〜9.15msでした。これは実行プログラム全体の値で、I/Oやデータ構造の寄与は個別に判明していません。

[Clang初回](fastset-linux-clang-initial.json)はClang 21.1.8で候補はコンパイルできましたが、提出のコンパイルがconstexpr評価のstep上限で失敗し、測定は未実行です。初回実行のrunnerを[run-initial.py.txt](run-initial.py.txt)としてバイト単位で保存しています。現runnerはClangに限り、候補・提出・段階計測・入力生成のすべてに `-fconstexpr-steps=100000000` を追加します。初回のGCCとはコンパイラとflagsが異なるため、異なる測定として記録します。Clang再実行結果は別途必要です。
