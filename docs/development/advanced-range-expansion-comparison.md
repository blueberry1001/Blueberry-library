# 静的区間 LIS・区間ソート・部分 Retroactive PQ の比較

特殊な操作を高速化する3構造について、同じ入力を直接的な実装でも処理して比較する。
比較対象は小規模入力で正しさを確認できる基準実装であり、他ライブラリの最適化された実装に対する優劣を示す測定ではない。

| 操作 | 追加構造 | 比較対象 | 計算量の違い |
| --- | --- | --- | --- |
| 静的区間の狭義 LIS 長 | StaticRangeLIS | 区間ごとに patience sorting | 前処理 O(N log²N)、各 O(log N) / 各 O(K log K) |
| 区間ソート・一点置換・区間積 | RangeSortRangeProduct | vector + std::sort + 逐次積 | 初期費用を含む合計 O((N+Q)(log N+W)) / ソート各 O(K log K)、積各 O(K) |
| 過去の insert/delete-min 操作の追加・削除 | PartiallyRetroactivePriorityQueue | 編集ごとに multiset で全履歴再生 | 各 O(log M) / 各 O(M log M) |

K は問い合わせ区間長、W は unsigned のビット数、M は事前に用意した時刻数。
RangeSortRangeProduct の sort は償却評価であり、単一操作が O(log N+W) とは限らない。
PartiallyRetroactivePriorityQueue の問い合わせは現在の状態だけを対象とする。

## 再現条件

- 同一実行ファイル・GCC・`-std=gnu++20 -O2 -DNDEBUG`。依存 ACL の revision、CPU、コンパイラ版、全直接依存ソースの SHA256 を環境ログに保存する。
- N または時刻数 M は 1024 / 4096。各 5000 操作、seed=20260919。
- LIS は値域約 N/4 の重複する乱数列。区間両端は一様乱数で、空区間も含む。
- 区間ソートは昇順35%・降順35%・一点置換15%・積15%を独立に抽選する。キーは全更新を通じて一意。積には64 bit unsigned の非可換な列 hash モノイドを使い、最終全体積も checksum に加える。
- Retroactive PQ は挿入・delete-min追加・操作削除を各1/3で抽選し、時刻も一様乱数。重複優先度、埋まった時刻への追加、空 queue からの削除を起こす無効履歴も含む。基準実装も同じ編集を拒否する。
- 5回の別プロセス、実装の順序は交互、ウォームアップ・CPU固定なし。入力生成・JSON出力を除き、構築と操作を別々に計測する。
- 同じ入力 hash・結果 checksum が全実装・全反復で一致することを確認する。測定中のソース変更は失敗扱い。

[比較ソース](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmarks/advanced-range-expansion.cpp) と
[再現スクリプト](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmarks/run-advanced-range-expansion.py) を用いる。
リポジトリ直下で `python3 benchmarks/run-advanced-range-expansion.py --output .build/advanced-range-remeasurement` を実行する。
既存サンプルは上書きせず拒否するため、再測定には新しい出力先を指定する。

## 測定結果

2026-09-19、Intel Core i7-14650HX / WSL2 Linux / GCC 13.3。
ほかの担当のコンパイル・公式検証・測定を止めた順次測定で、60サンプルすべての入力 hash と結果 checksum が一致した。
各欄は **中央値 / 最小値、単位 ms**。Q=5000。構築時間と操作時間は別々の区間で計測している。

| 対象 | N または M | 実装 | 構築 ms | 操作 ms |
| --- | ---: | --- | ---: | ---: |
| LIS | 1024 | StaticRangeLIS | 3.705 / 3.596 | 0.505 / 0.498 |
| LIS | 1024 | 区間ごと patience sorting | 0.003 / 0.002 | 28.748 / 28.282 |
| LIS | 4096 | StaticRangeLIS | 18.921 / 17.896 | 0.662 / 0.637 |
| LIS | 4096 | 区間ごと patience sorting | 0.006 / 0.006 | 151.652 / 146.346 |
| 区間ソート | 1024 | RangeSortRangeProduct | 0.075 / 0.068 | 3.175 / 2.967 |
| 区間ソート | 1024 | vector + sort + 逐次積 | 0.006 / 0.005 | 12.435 / 11.742 |
| 区間ソート | 4096 | RangeSortRangeProduct | 0.310 / 0.281 | 4.203 / 4.071 |
| 区間ソート | 4096 | vector + sort + 逐次積 | 0.023 / 0.018 | 54.605 / 52.148 |
| Retroactive PQ | 1024 | PartiallyRetroactivePriorityQueue | 0.039 / 0.037 | 1.357 / 1.266 |
| Retroactive PQ | 1024 | 全履歴再生 | 0.002 / 0.002 | 22.945 / 22.112 |
| Retroactive PQ | 4096 | PartiallyRetroactivePriorityQueue | 0.147 / 0.147 | 1.814 / 1.715 |
| Retroactive PQ | 4096 | 全履歴再生 | 0.008 / 0.008 | 71.904 / 70.655 |

この入力では3構造とも操作部分の時間を削減できた。StaticRangeLIS は前処理の負担が大きいため、問い合わせが少ない場合にも得になるとは限らない。
小さい構築時間の差を定数倍の優劣の根拠にはしない。メモリ使用量は実測していない。

[全60サンプル](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/advanced-range-expansion/samples.jsonl)、
[集計 JSON](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/advanced-range-expansion/summary.json)、
[環境・ソース SHA256](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/advanced-range-expansion/environment.json)、
[コンパイルログ](https://raw.githubusercontent.com/blueberry1001/Blueberry-library/main/docs/development/measurements/advanced-range-expansion/compile.log) を保存した。

## 検証範囲

StaticRangeLIS は同値・昇順・降順を含む全区間を patience sorting と比較する。
RangeSortRangeProduct は文字列連結による非可換積、unsigned の上位 bit、空区間、コピーも検証する。
PartiallyRetroactivePriorityQueue は任意位置の編集を全履歴再生と比較し、拒否された編集後の状態も確認する。
後者に過去編集を網羅する公式 judge の対応はないため、通常の priority queue 問題への AC を代用していない。

理論・APIの出典は各ライブラリページを参照。実行時間はこのワークロード・環境に限定した結果として解釈する。
