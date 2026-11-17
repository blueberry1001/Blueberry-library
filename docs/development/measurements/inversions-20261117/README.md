# Static Range Inversions のブロック幅調査

公開APIの返却順・対応型・転倒数の定義を変えずに、Mo順序の幅を調整した。
従来幅は `max(1, floor(sqrt(max(1,N))))`。採用候補は `Q<N` かつ従来幅より長い区間がある場合に、幅を少なくとも `floor(N/sqrt(max(1,Q)))` にする。追加走査は最大O(Q)、追加メモリなし。空queryでは除算せず、端点差と幅の変換は既存の `N<INT_MAX` の下でintに収まる。移動量は以前の最悪計算量の範囲内に収まる。

調査資料の [maspypy Mo](https://maspypy.github.io/library/ds/offline_query/mo.hpp) は問い合わせ数に応じた幅と局所的な順序調整を用いる。[Library Checker提出 213470](https://judge.yosupo.jp/submission/213470) は区間を先読みし、更新をまとめる別方式を使う。これらはアルゴリズムの参考であり、同条件の速度比較や現時点での最速の証明ではない。ACLのFenwickを維持し、提出コードのコピーはしていない。

## 計測条件

Windows、Intel Family 6 Model 183 Stepping 1、Android NDK Clang 18.0.3、Windows MSVC 14.43.34808 標準ライブラリ、`--target=x86_64-pc-windows-msvc -fuse-ld=link -std=gnu++20 -O2 -DNDEBUG`。seed 20261117、warmup 1回の後5回。順番は毎回反転し、全候補のchecksum一致を確認。事前生成した同一入力で公開関数全体を計時し、生成とchecksumは計時外。I/O・メモリ・圧縮・排序・操作それぞれの費用は分離していない。

次は [results.json](results.json) の中央値ms。queryは一様な両端点、shortは長さ63以下、mixedはshort群に `[0,N)` を1件加えたもの。duplicateは値の種類が8。計測後半に負荷または周波数の変化があり、同じbaselineが別の計測より大きく変動した。わずかな差の優劣には使わない。

| N | Q | 分布 | 従来 | 採用候補 |
| ---: | ---: | --- | ---: | ---: |
| 100000 | 1 | random | 8.853 | 8.940 |
| 100000 | 100 | random | 48.298 | 24.279 |
| 100000 | 1000 | random | 226.117 | 54.979 |
| 100000 | 10000 | random | 314.603 | 148.333 |
| 100000 | 50000 | random | 406.104 | 306.876 |
| 100000 | 1000 | short | 13.921 | 14.032 |
| 100000 | 1000 | mixed | 18.065 | 19.628 |
| 100000 | 1000 | duplicate | 98.961 | 20.174 |
| 100000 | 100000 | random | 521.385 | 520.461 |
| 20000 | 200000 | random | 152.071 | 172.014 |

mixedには約8.7%の回帰がある。Q≥Nでは幅を決める処理は従来と同じで、今回のばらつきだけから速度差を断定しない。単に `N/sqrt(Q)` とする候補は [guarded-results.json](guarded-results.json) の短区間で12.723→14.085msと回帰した。queryごとのブロック番号事前計算も [initial-results.json](initial-results.json) で安定した優位がなく採用していない。途中結果や不採用候補も削除していない。

## 再現と正しさ

このディレクトリのファイルを `.build/inversions-experiment/` にコピーし、保存用の `.cpp.txt` と `.hpp.txt` の末尾 `.txt` を取り除く。次に `python .build/inversions-experiment/prepare.py`、`.build/inversions-experiment/build.ps1`、`python .build/inversions-experiment/run.py` を順に実行する。Windows上のcompilerとSDKのパスは `build.ps1` に記載した環境固有の値なので適宜変更する。現在の `bench.cpp.txt` と `run.py` は最終測定用で、先行測定とは異なる。先行結果のソースハッシュは各JSONに保持する。`shortguard.hpp.txt` は測定後、namespace名を除いて現行ヘッダと一致することを確認した。

候補の事前検証では固定seedの小入力を全候補で愚直計算と比較した。現行ヘッダと新旧random testはWindows Clangで `gnu++20` / `gnu++23`、seed 1・2・17・20261117が成功した。初回の新testコンパイルは `<string>` 不足で失敗し、includeを追加して再実行した。GNU GCC、Library Checker、`make test` / `make verify` / `make docs` はこの個別調査では未実行。WSLはアクセス拒否だった。
