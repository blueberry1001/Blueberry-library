# Dynamic Sqrt Tree の同条件比較

固定配列向け `SqrtTree`、一点代入対応 `DynamicSqrtTree`、ACL `segtree` を比較した。
更新可能な構造も静的クエリへ利用できる一方、常に静的専用構造より速いとは限らない。

## 条件

- 2026-09-19、Intel Core i7-14650HX、WSL2 Linux、GCC 13.3。
- `-std=gnu++20 -O2 -DNDEBUG -I. -I.deps/ac-library`。各実装は同じ実行ファイルに含め、同じコンパイラ・オプションを使う。
- 要素型 `long long`、加算モノイド、値は 0..1000。N = 4096 / 4097 / 65536 / 65537。
- 各入力に 50,000 操作。更新率 0% / 1% / 50%、seed = 20260919。
- 更新位置・代入値・クエリの両端は乱数、100 操作単位で指定個数を更新にする。添字区間は半開で、空区間もあり得る。
- 5 回、各サンプルは別プロセス。実装の実行順は反復ごとに回転する。ウォームアップなし。
- 更新なしでは全3実装、更新ありでは DynamicSqrtTree と ACL のみ。静的構造に再構築を強制した比較は行わない。
- 構築時間は初期配列からの構築だけ。操作時間には共通のループ・結果 checksum 計算も含む。乱数入力生成とJSON出力は含まない。
- 全実装・全反復で入力 hash と結果 checksum の一致を検査した。最終全体積も checksum に含めた。

[測定ソース](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmarks/dynamic-sqrt-tree.cpp)、
[再現スクリプト](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/dynamic-sqrt-tree/run.py)、
[全140サンプル](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-sqrt-tree/samples.jsonl)、
[集計JSON](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-sqrt-tree/summary.json)、
[環境・全依存ソースSHA256](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-sqrt-tree/environment.json)、
[コンパイルログ](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/dynamic-sqrt-tree/compile.log) を保存した。
ソースが計測途中で変わっていないこともスクリプトで確認する。

## 結果

時間は ms、各欄は中央値 / 最小値。RSS はプロセス全体の peak KiB の中央値で、同一入力・標準ライブラリ・allocator も含む。
Linux `/proc/self/status` の `VmHWM` を用いたもので、データ構造単独の使用量ではない。
小さい RSS 差は allocator や環境差も含むため改善根拠にしない。

| N | 更新率 | 実装 | 構築 ms median / min | 操作 ms median / min | peak RSS KiB median |
| --- | --- | --- | --- | --- | --- |
| 4096 | 0% | static | 0.125 / 0.121 | 0.344 / 0.339 | 4688 |
| 4096 | 0% | dynamic | 0.108 / 0.106 | 0.686 / 0.677 | 4740 |
| 4096 | 0% | acl | 0.022 / 0.020 | 2.699 / 2.661 | 4396 |
| 4096 | 1% | dynamic | 0.117 / 0.101 | 0.737 / 0.735 | 4732 |
| 4096 | 1% | acl | 0.022 / 0.017 | 2.730 / 2.661 | 4412 |
| 4096 | 50% | dynamic | 0.115 / 0.101 | 2.841 / 2.796 | 4732 |
| 4096 | 50% | acl | 0.019 / 0.019 | 1.589 / 1.554 | 4428 |
| 4097 | 0% | static | 0.255 / 0.250 | 0.350 / 0.345 | 5260 |
| 4097 | 0% | dynamic | 0.225 / 0.211 | 0.713 / 0.705 | 5264 |
| 4097 | 0% | acl | 0.037 / 0.036 | 2.673 / 2.614 | 4432 |
| 4097 | 1% | dynamic | 0.231 / 0.217 | 0.794 / 0.783 | 5264 |
| 4097 | 1% | acl | 0.039 / 0.031 | 2.693 / 2.674 | 4396 |
| 4097 | 50% | dynamic | 0.241 / 0.229 | 3.843 / 3.759 | 5220 |
| 4097 | 50% | acl | 0.038 / 0.035 | 1.634 / 1.608 | 4376 |
| 65536 | 0% | static | 2.132 / 2.072 | 0.902 / 0.637 | 12120 |
| 65536 | 0% | dynamic | 1.770 / 1.694 | 0.866 / 0.847 | 11028 |
| 65536 | 0% | acl | 0.320 / 0.282 | 3.765 / 3.726 | 5796 |
| 65536 | 1% | dynamic | 1.828 / 1.738 | 1.039 / 0.926 | 11076 |
| 65536 | 1% | acl | 0.301 / 0.279 | 3.713 / 3.701 | 5760 |
| 65536 | 50% | dynamic | 1.800 / 1.642 | 8.723 / 8.569 | 11072 |
| 65536 | 50% | acl | 0.336 / 0.262 | 2.328 / 2.189 | 5792 |
| 65537 | 0% | static | 5.225 / 4.732 | 0.811 / 0.698 | 21844 |
| 65537 | 0% | dynamic | 4.703 / 4.373 | 1.119 / 0.935 | 20236 |
| 65537 | 0% | acl | 0.593 / 0.558 | 3.833 / 3.706 | 6816 |
| 65537 | 1% | dynamic | 5.171 / 4.800 | 1.317 / 1.270 | 20200 |
| 65537 | 1% | acl | 0.674 / 0.555 | 4.189 / 3.938 | 6848 |
| 65537 | 50% | dynamic | 4.618 / 4.554 | 14.087 / 13.356 | 20220 |
| 65537 | 50% | acl | 0.590 / 0.571 | 2.279 / 2.234 | 6764 |

## 判断と限界

この加算・ランダム区間の入力では、DynamicSqrtTree のクエリ処理は ACL segtree より速く、
更新率 1% でも操作合計は短い。一方、更新率 50% では ACL segtree が速い。
更新なしでは、小さい2サイズと N=65537 で静的 SqrtTree が速かった。
N=65536 の両者の中央値は近く逆転しており、単一測定の微差から一般的な優劣は判断しない。
静的な用途をすべて DynamicSqrtTree に置き換える根拠にはならない。
ACL は構築が安く、クエリ数の少ない入力では構築込みの優劣も検討する必要がある。
N が 2 冪を超えるとパディングにより構築・更新コストとメモリが増えることも観測できる。

更新可能という機能だけで候補を隠さず、クエリ主体なら DynamicSqrtTree も静的検索の候補に含め、
更新が多い場合は ACL segtree を優先する使い分けが妥当と考える。
これは有限の加算ワークロードによる測定であり、非可換演算・大型要素・別の区間分布・別CPUでの優位は主張しない。
非常に短い操作時間には共通の checksum 処理の比率も含まれる。
正式な正当性確認は対応する verify と非可換演算のランダムテストで別途行う。

初回の `getrusage(RUSAGE_SELF).ru_maxrss` を使ったログは
[pilot-rusage](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/dynamic-sqrt-tree/pilot-rusage) 以下へ保存した。
この値に起動元 Python の exec 前 high-water mark が混入して一律の下限が見えたため、
メモリ比較には採用しなかった。初回と最終測定はソース hash が異なるので合算していない。
欠損・失敗値を 0 として扱っていない。

最終版では root/index のクエリを `Table::prod<bool Indexed>` で区別し、
GCC の誤検出警告を避けるため再帰の有無をコンパイル時に確定した。
ソース hash とコード生成が変わるため、修正前140サンプルは
[before-indexed-template](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/dynamic-sqrt-tree/before-indexed-template)
へ分離した。上の比較表と通常のログリンクは修正後に再測定した140サンプルのみを参照する。
