# 多点評価の低次数入力

係数数 N が小さく点数 M が多いと、従来は積木を全点分構築してから Horner 評価に切り替えていた。係数が1個なら結果を一括生成し、2〜32個または点が1個なら入口で Horner 評価する。空入力と `check_size` の扱い、modint の型・NTT長の前提は維持する。33個以上かつ2点以上の場合は従来の積木を使う。

## 参照した実装

- [Nyaan](https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html) は積木の構築後、剰余の係数数が64以下のとき直接評価する。NTT専用の積木構築もある。評価点全体に対する低次数の入口短絡はない。
- [Luzhiled](https://ei1333.github.io/library/math/fps/multipoint-evaluation.hpp.html) は積木と各葉への剰余伝播を使う。ACLに多点評価そのものはなく、Blueberry はACLの modint と畳み込みを利用する。
- Library Checker API の 2026-11-18 09:10 JST 時点、AC/C++ を時間順・ユーザー重複除外で調べると、[最速 #409516](https://judge.yosupo.jp/submission/409516) は 15 ms、次は 39 ms だった。結果は [fastest-submissions.json](fastest-submissions.json) に保存した。取得した最速のソース SHA-256 は `bcba5d9515f79dda9970e8d324aa38cb496466fc86f6be9a81c7b2427cd3c165`。出典の再利用条件が不明なのでソース自体は保存せず、公開提出へリンクする。
- #409516 は `N*M <= 2^22` で積木を作らず Horner を実行し、32点を AVX2 で並行評価する。それ以外は法 998244353、x86-64/AVX2 向けの転置積木を使い、NTT のカーネルにインラインアセンブリ、8点並列のデータ配置、専用 arena、固定幅の出力と起動処理の省略を組み合わせている。汎用 static_modint と通常の C++ 環境をサポートする現行 API とは前提が異なるため、そのまま置き換えない。特に閾値は SIMD の有無や環境によって変わりうる。提出時間と以下のローカル測定は、入力・コンパイラ・環境が異なるため直接比較しない。
- 比較対象として [Library Checker の公開コード #85826](https://judge.yosupo.jp/submission/85826) には別問題の提出に多項式ライブラリが埋め込まれ、その作者コメントは多点評価提出 [#75126](https://judge.yosupo.jp/submission/75126) を参照する。

参照元のコードをコピーせず、Horner 法を既存 API に適用した。一般の積木部分について最速提出との同条件測定は未実施である。

## 同条件の比較

記録ファイルの Windows 更新時刻は 2026-11-18 09:03〜09:04 JST。WSL2 Linux、GCC 13.3.0、`-std=gnu++20 -O2 -DNDEBUG`、同じ ACL と FPS、CPU 0 に固定。測定時の WSL 時刻と CPU 機種は記録していない。warmup 1回後、順序を交互にして7回測定した。表示は関数呼び出しの中央値と MAD（ms）で、入力生成・入力検証・起動は含まず、返り値生成と allocation は含む。

| 係数 N | 点数 M | 入力 | 変更前 median ± MAD | 候補 median ± MAD |
| ---: | ---: | --- | ---: | ---: |
| 1 | 100000 | 一様乱数 | 59.495 ± 0.439 | 0.095 ± 0.003 |
| 8 | 100000 | 一様乱数 | 64.787 ± 1.375 | 1.171 ± 0.019 |
| 31 | 100000 | 一様乱数 | 66.907 ± 0.828 | 7.581 ± 0.160 |
| 32 | 100000 | 一様乱数 | 70.239 ± 1.060 | 8.031 ± 0.158 |
| 33 | 100000 | 一様乱数 | 71.767 ± 1.050 | 72.874 ± 0.302 |
| 64 | 100000 | 一様乱数 | 78.749 ± 0.388 | 79.056 ± 0.726 |
| 2048 | 2048 | 一様乱数 | 2.355 ± 0.106 | 2.389 ± 0.029 |
| 32 | 100000 | 末尾半分が零 | 63.882 ± 1.032 | 8.020 ± 0.075 |

この比較では33個以上の経路は同じだが、命令配置や環境のばらつきで中央値が少し異なり、回帰がないとは断定できない。短い入力の時間は特に粒度の影響を受ける。RSS はプロセス全体のピークで入力や測定後の照合を含むため、関数の追加メモリだけを示すものではない。全 320 実行、最小・最大・標準偏差、RSS、compiler、seed、ハッシュは [results.json](results.json) に残した。

基準は main の `97aa84673d34421262140d3fb8311b09219aa82b`。実際にコンパイルした [基準ヘッダ](baseline.hpp)、[低次数候補ヘッダ](candidate.hpp)、共通の [FPS](fps.hpp)、[測定ドライバ](benchmark.cpp)、[oracle](oracle.cpp)、[runner](run.py) を保存した。低次数候補ヘッダ SHA-256 は `d7e7def08fd562322a624d86102c85f16df3a201ac4085ea519ca77bb2fc0b1b`。最終候補はさらに下記の1点短絡を含む。FPS は git commit から取得し、既存の未コミット変更を混ぜていない。ACL は両版で同一の snapshot を使用し、tree hash は JSON に記録している。

ランナーは、指定 commit の基準ヘッダと FPS、指定 ACL を隔離コピーし、差分を挿入して両版をコンパイルする。次のコマンドは Linux / WSL での再現例であり、パスと利用可能な CPU 番号は環境に合わせる。

```sh
python3 docs/development/measurements/multipoint-low-degree-20261118/run.py \
  --repo /path/to/Blueberry-library \
  --revision 97aa84673d34421262140d3fb8311b09219aa82b \
  --acl /path/to/ac-library --cpu 0
```

3 種の素数 modint と seed 1729、1730、1826 で、空列・重複点・末尾零・31/32/33を含む全結果を Horner と照合した。oracle は `-UNDEBUG` で assert を有効にし、各測定でも結果全体を計時外で照合している。

## 1点の追加比較

低次数候補を基準に、点が1個であれば係数数にかかわらず Horner で評価する候補を追加した。記録ファイルの Windows 更新時刻は 2026-11-18 09:13〜09:14 JST。上記と同じ revision、FPS、ACL、コンパイラ、flags、CPU 0、warmup 1回と7回の交互測定。こちらは独立した実験であり、前表と同じ試行として混ぜない。

| 係数 N | 点数 M | 基準 median ± MAD | 候補 median ± MAD |
| ---: | ---: | ---: | ---: |
| 33 | 1 | 0.006648 ± 0.000466 ms | 0.000347 ± 0.000078 ms |
| 1024 | 1 | 0.121877 ± 0.005358 ms | 0.003662 ± 0.000043 ms |
| 8192 | 1 | 1.845037 ± 0.130194 ms | 0.050212 ± 0.013442 ms |
| 100000 | 1 | 19.845411 ± 0.289049 ms | 0.470814 ± 0.137117 ms |
| 8192 | 2 | 1.140637 ± 0.126723 ms | 1.055786 ± 0.077451 ms |
| 2048 | 2048 | 2.297857 ± 0.106006 ms | 2.403007 ± 0.089568 ms |

変動が大きい試行もあり、特に影響を受けない入力での差を原因に結びつけない。全192実行と3 seed・3 modint の oracle、正確な [基準](one-point/baseline.hpp)・[候補](one-point/candidate.hpp)・[FPS](one-point/fps.hpp)・[driver](one-point/benchmark.cpp)・[runner](one-point/run.py)・[結果](one-point/results.json) を保存した。最終の実装はこの候補と等価で、名前空間末尾コメント・改行コードのみが異なる。

```sh
python3 docs/development/measurements/multipoint-low-degree-20261118/one-point/run.py \
  --repo /path/to/Blueberry-library \
  --revision 97aa84673d34421262140d3fb8311b09219aa82b \
  --acl /path/to/ac-library --cpu 0
```

## 変更後の検証

固定 main の独立 checkout に対象ヘッダ・ドキュメント・テストを重ね、共通 ACL を使用した。GCC 13.3 / Clang 18.1 の gnu++20 / 23 で、独立 random test の seed 1、1729、1730、1826 と `-DNDEBUG` の単独・重複 include がすべて成功した。[compiler-validation.log](validation/compiler-validation.log)

対象の `make verify VERIFY_REPEATS=1 VERIFY_PATHS=verify/math/multipoint-evaluation.test.cpp` は Library Checker の 11 / 11 ケースが成功した。[再開後ログ](validation/official-verify-resume.log)。再開前には、archive には git 情報がなかったため失敗し、独立 clone に変更した後も非ログイン shell の PATH に `oj` がなく失敗した。[git 情報不足](validation/official-verify-initial-gitless.log)、[PATH 不足](validation/official-verify-initial-no-oj.log)。

`library_checker_coverage.py` の生成と `--check` は成功した。一方、全体の `make test` は WSL 再起動で途中中断し、未完了である。[中断時ログ](validation/make-test.log)。最初の隔離準備でも shell 引数の引用と WSL 一時ディレクトリの消失による失敗があり、それぞれ [記録](validation/initial-validation-failures.log) した。`make docs` と全 target の `make verify` は、この隔離検証では実行していない。
