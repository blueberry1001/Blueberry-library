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
| 4096 | 0% | static | 0.136 / 0.132 | 0.354 / 0.327 | 4724 |
| 4096 | 0% | dynamic | 0.150 / 0.130 | 0.688 / 0.661 | 4736 |
| 4096 | 0% | acl | 0.022 / 0.021 | 2.778 / 2.768 | 4396 |
| 4096 | 1% | dynamic | 0.131 / 0.123 | 0.723 / 0.700 | 4724 |
| 4096 | 1% | acl | 0.023 / 0.021 | 2.794 / 2.620 | 4400 |
| 4096 | 50% | dynamic | 0.141 / 0.117 | 2.673 / 2.498 | 4704 |
| 4096 | 50% | acl | 0.025 / 0.017 | 1.697 / 1.645 | 4432 |
| 4097 | 0% | static | 0.292 / 0.244 | 0.366 / 0.336 | 5220 |
| 4097 | 0% | dynamic | 0.292 / 0.226 | 0.701 / 0.635 | 5220 |
| 4097 | 0% | acl | 0.050 / 0.028 | 2.900 / 2.552 | 4452 |
| 4097 | 1% | dynamic | 0.220 / 0.185 | 0.700 / 0.685 | 5268 |
| 4097 | 1% | acl | 0.037 / 0.035 | 2.561 / 2.505 | 4380 |
| 4097 | 50% | dynamic | 0.227 / 0.223 | 3.294 / 2.940 | 5224 |
| 4097 | 50% | acl | 0.037 / 0.035 | 1.620 / 1.557 | 4476 |
| 65536 | 0% | static | 2.823 / 2.261 | 0.759 / 0.584 | 12108 |
| 65536 | 0% | dynamic | 2.312 / 1.733 | 0.929 / 0.563 | 10980 |
| 65536 | 0% | acl | 0.343 / 0.324 | 3.860 / 3.588 | 5752 |
| 65536 | 1% | dynamic | 2.291 / 2.143 | 0.991 / 0.939 | 11036 |
| 65536 | 1% | acl | 0.352 / 0.336 | 3.872 / 3.593 | 5796 |
| 65536 | 50% | dynamic | 1.915 / 1.734 | 7.400 / 6.917 | 11068 |
| 65536 | 50% | acl | 0.349 / 0.287 | 2.309 / 2.154 | 5796 |
| 65537 | 0% | static | 4.904 / 4.797 | 0.551 / 0.456 | 21788 |
| 65537 | 0% | dynamic | 4.640 / 4.581 | 0.789 / 0.679 | 20228 |
| 65537 | 0% | acl | 0.607 / 0.568 | 3.937 / 3.557 | 6760 |
| 65537 | 1% | dynamic | 4.873 / 4.815 | 1.031 / 0.944 | 20200 |
| 65537 | 1% | acl | 0.614 / 0.608 | 3.819 / 3.658 | 6812 |
| 65537 | 50% | dynamic | 5.280 / 4.815 | 11.105 / 10.496 | 20232 |
| 65537 | 50% | acl | 0.685 / 0.619 | 2.349 / 2.282 | 6844 |

## 判断と限界

この加算・ランダム区間の入力では、DynamicSqrtTree のクエリ処理は ACL segtree より速く、
更新率 1% でも操作合計は短い。一方、更新率 50% では ACL segtree が速い。
更新なしでは、今回の4サイズで静的 SqrtTree が速かった。
過去の別 revision の測定では近い中央値が逆転したケースもあり、単一測定の微差から一般的な優劣は判断しない。
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
へ分離した。この段階の結果も次の更新最適化前の記録として分離した。

さらに一点更新では影響する prefix[p..end) と suffix[start..p] だけを再計算するように最適化した。
その変更前の140サンプルは
[before-partial-block-update](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/dynamic-sqrt-tree/before-partial-block-update)
へ保存した。上の比較表と通常のログリンクは、この最適化後に再測定した最終140サンプルのみを参照する。
過去の測定結果と合算していない。
