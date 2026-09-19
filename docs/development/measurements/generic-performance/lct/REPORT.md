# Link-Cut Treeの汎用性を保った集約処理の削減

`e0d8e71`の実装と、公開APIを維持した4条件を同じ環境で比較した。
**空の子に対する演算省略・rotation中の重複pull削減・auxiliary rootとaccessの不要処理省略を組み合わせた候補を採用する。**
加算への専用化、反転の省略、同じ連結成分かの確認の省略は行わない。

## 条件と範囲

WSL2、Core i7-14650HX、論理CPU 0に固定し逐次実行。
GCC 13.3／Clang 18.1.3、gnu++20、O2。
releaseはNDEBUG、GCC assert条件はassert有効と`_GLIBCXX_ASSERTIONS`を使用した。
N=8192、Q=60000、固定seed20260919。各条件warmup1回と測定5回。
4候補×3設定×3演算×4形状で、720測定・144warmupを保存した。
op回数は別の計数用binaryで測り、その時間を性能サンプルに混ぜていない。

入力生成は計測外。以下の時間はコンストラクタ・初期link・全操作を含むC++内部時間の中央値。
全操作にはprod、set/get、evert/leader、same、失敗link/cutとchecksum計算を含む。
dynamicはleafの切断・再接続であり、任意の内部辺を変える操作分布とは異なる。
全体のプロセス時間・構築・操作・RSS・全試行は生ログに残した。

## GCC releaseの結果

| 演算 | 木・操作分布 | 変更前 ms | 採用候補 ms | 候補／変更前 |
| --- | --- | ---: | ---: | ---: |
| sum | path | 40.068 | 40.431 | 1.009 |
| sum | star | 4.483 | 3.160 | 0.705 |
| sum | random | 37.044 | 36.107 | 0.975 |
| sum | dynamic | 42.331 | 39.572 | 0.935 |
| affine | path | 52.114 | 44.421 | 0.852 |
| affine | star | 6.402 | 5.624 | 0.879 |
| affine | random | 45.620 | 38.172 | 0.837 |
| affine | dynamic | 52.071 | 42.503 | 0.816 |
| matrix | path | 93.624 | 67.153 | 0.717 |
| matrix | star | 14.782 | 8.813 | 0.596 |
| matrix | random | 83.243 | 60.961 | 0.732 |
| matrix | dynamic | 80.752 | 67.289 | 0.833 |

affineとmatrixは非可換演算。正逆両方の集約を従来通り保持した上での結果である。
加算ではpathが0.9%遅く、randomの差も小さい。全条件で速くなるとは主張しない。

各設定における4形状の総時間比の範囲は次の通り。

| 設定 | sum | affine | matrix |
| --- | ---: | ---: | ---: |
| GCC release | 0.705–1.009 | 0.816–0.879 | 0.596–0.833 |
| GCC assert有効 | 0.767–1.054 | 0.593–0.846 | 0.634–0.816 |
| Clang release | 0.783–0.970 | 0.708–0.880 | 0.600–0.723 |

GCC assert有効のsum/randomは中央値が46.388→48.878 msで5.4%遅かった。
変更前の範囲は45.843–50.976 ms、候補は47.062–70.331 msで重なり、外れ値もある。
ノイズと断定して無視せず、軽い演算でのこの差は次の性能確認項目として残す。
一方、全compiler・形状の非可換演算では中央値が改善し、呼び出し回数の削減も確認できたため、組合せ候補を採用する。

## 何を減らしたか

操作フェーズのop呼び出し回数は、releaseのsumで以下の通り。演算回数を数えるためのcounterは本番実装に含まれない。

| 形状 | 変更前 | 採用候補 | 削減率 |
| --- | ---: | ---: | ---: |
| path | 13,922,668 | 7,028,536 | 約49.5% |
| star | 2,619,876 | 1,218,918 | 約53.5% |
| random | 11,882,016 | 5,609,264 | 約52.8% |
| dynamic | 12,444,472 | 5,789,324 | 約53.5% |

単位元との演算省略だけ、rotationのpull削減だけの候補も測った。
例えばGCC releaseのaffine/pathでは単位元省略だけだと約1.14倍、matrix/starではrotation変更だけだと約1.19倍となった。
個々の変更に常に利益があるとは限らないため、回数だけで採用を決めず、最終的な組合せを測定している。

rotationでは、下に移った親の子集合は確定するためpullする。
上に移った頂点の集約は、その後に下へ移動した時点か、splayの最後の根で再計算する。
途中のrotationはその頂点の古い集約に依存しない。
祖先の反転は従来通りstackを使い上からpushする。stackを省けるのは既にauxiliary rootである場合だけ。
accessでは右子が実際に変わった時だけ再集約する。

単位元との結合を省く際も、両子がある場合の結合順、forward/backward、Sのコピー・代入の要件は変えていない。
publicメソッドの処理、戻り値、失敗時の根の変化も維持した。

## 正しさの確認

- GCC／Clang×gnu++20／23で、強化した文字列連結+BFSのrandom testを6seedずつ、合計24回実行して合格。
- Clang ASan／UBSanで同テストを1回実行して合格。
- 2種類の公式動的木問題の全38ケースを、公開verify driverで実行し、全出力を公式期待値と照合して合格。
- benchmarkの全条件でchecksum一致。失敗すべきlink/cut、成功すべきlink/cut、set/get、evert/leaderも実行中に確認。
- 非可換の逆向きパス、失敗操作後を含む根、copy/move、default constructorを削除した所有型も回帰対象に追加。

これはLibrary Checkerへ新規提出した結果ではない。第三者最速提出との時間比も示していない。
別環境・異なるN/Q・異なるopの費用・内部辺の変更分布に対する同じ倍率を保証しない。

## 再現資料

- [測定器と実行手順](README.md)
- [全測定・環境・op回数](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/lct/results/full)
- [先行した小規模比較](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/lct/results/quick)
- [random／sanitizer／公式照合結果](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/lct/results/validation)

hashで固定した実装と入力生成条件を使い、比較不能な測定を一つの倍率に混ぜない。
