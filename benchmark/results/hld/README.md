# HLD の構築・問い合わせ性能比較

採用した候補2は、構築と、平衡木・ランダム木での path callback を短縮した。
候補1にあった星形木の LCA の一貫した悪化も解消した。短い問い合わせでは小さな悪化や
測定の揺れが残るため、全操作・全入力で高速になったとは扱わない。

基準は `0dfa0e6c0f1de890feef2af80ae2a3e182ea8f55` の
`blueberry/graph/heavy-light-decomposition.hpp`。採用版の SHA256 は
`2cd350a1c36e66b5aef013b0cc730f1e9c15485bf9339756c49d43b483a57a05`。
測定時のヘッダは各結果ディレクトリの `candidate.hpp` に保存した。

## 条件

- Intel Core i7-14650HX、WSL2、Linux 5.15.167.4、glibc 2.39。CPU affinity は 2。
- GCC 13.3.0 / Clang 18.1.3、`-std=gnu++20 -O2 -Wall -Wextra -Werror`。
  assert 有効と `-DNDEBUG` を分けた。`-march=native` は使用していない。
- `N = Q = 200000`、根は 0、seed は `20261004`。
  木は鎖、星、`parent(v)=(v-1)/2` の平衡二分木、`parent(v)` を `[0,v)` から選ぶ
  ランダム木の4種。隣接リストを固定 seed でシャッフルした。
  このランダム木は、一様ランダムなラベル付き木ではない。
- 問い合わせの10%は同一頂点、10%は1〜32回親へ上がった祖先との組
  （両方向）、残り80%は一様な頂点対。部分木操作は頂点・辺モードを混在させた。
- 操作ごと・実装ごとに1回ウォームアップし、7回測定。基準と候補の実行順を交互にした。
  同じプロセス内の同じ入力を使い、各回のチェックサムと callback 回数を比較した。
- 構築はコンストラクタだけを測定し、木の生成、全配置の検査、デストラクタを除外した。
  問い合わせは構築済みの HLD を使う。入力生成・入出力は測定外。
  callback 内の小さな符号なし整数チェックサムの計算は測定に含む。

計測中は、この作業の他のコンパイル・テストを停止した。CPU 周波数、温度、OS、
PC 上の他の処理までは固定していない。これは API の microbenchmark であり、
Library Checker の end-to-end 時間や総メモリ使用量の比較ではない。

## 採用版の結果

以下は [candidate2-clean/summary.csv](candidate2-clean/summary.csv) の
**候補の中央値 / 基準の中央値**。1未満が短縮、1超が悪化。
`p2` は2引数 callback、`p3` は向き付き3引数 callback、`V/E` は頂点/辺モード。
`range/subtree` は変更していない部分木 API の対照測定。
絶対時間・最小値・最大値は CSV、7回すべての値は `raw.csv` に残した。

| compiler / mode | 木 | build | lca | p2 V | p2 E | p3 V | p3 E | range | subtree |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| GCC / assert | 鎖 | 0.749 | 0.390 | 0.814 | 0.868 | 1.072 | 1.025 | 1.002 | 0.983 |
| GCC / assert | 星 | 0.764 | 0.906 | 0.821 | 0.870 | 1.040 | 1.043 | 0.980 | 0.908 |
| GCC / assert | 平衡 | 0.721 | 0.967 | 0.660 | 0.642 | 0.726 | 0.712 | 1.006 | 0.824 |
| GCC / assert | ランダム | 0.815 | 0.933 | 0.582 | 0.586 | 0.641 | 0.651 | 0.992 | 0.855 |
| GCC / release | 鎖 | 0.784 | 0.353 | 0.839 | 0.819 | 1.051 | 1.079 | 1.066 | 0.995 |
| GCC / release | 星 | 0.756 | 1.019 | 0.856 | 0.768 | 0.965 | 0.772 | 0.941 | 0.933 |
| GCC / release | 平衡 | 0.761 | 0.943 | 0.662 | 0.618 | 0.693 | 0.705 | 0.993 | 0.935 |
| GCC / release | ランダム | 0.886 | 0.929 | 0.567 | 0.614 | 0.678 | 0.740 | 1.010 | 0.941 |
| Clang / assert | 鎖 | 0.775 | 0.943 | 1.011 | 1.013 | 0.947 | 0.974 | 0.991 | 1.007 |
| Clang / assert | 星 | 0.687 | 0.821 | 1.018 | 0.981 | 0.995 | 0.951 | 1.003 | 0.995 |
| Clang / assert | 平衡 | 0.755 | 0.868 | 0.751 | 0.752 | 0.721 | 0.714 | 1.036 | 1.029 |
| Clang / assert | ランダム | 0.863 | 0.879 | 0.700 | 0.714 | 0.679 | 0.701 | 0.972 | 0.946 |
| Clang / release | 鎖 | 0.752 | 0.990 | 0.961 | 1.049 | 0.994 | 1.001 | 0.977 | 0.991 |
| Clang / release | 星 | 0.624 | 0.863 | 0.904 | 0.997 | 0.830 | 0.810 | 1.023 | 1.040 |
| Clang / release | 平衡 | 0.730 | 0.841 | 0.701 | 0.694 | 0.731 | 0.730 | 1.012 | 1.056 |
| Clang / release | ランダム | 0.825 | 0.911 | 0.716 | 0.694 | 0.632 | 0.691 | 0.942 | 1.044 |

構築は全16条件で0.624〜0.886倍、平衡木・ランダム木の path callback は
全32条件で0.567〜0.752倍だった。同じ反復番号どうしの比の中央値で確認しても、
構築の短縮は全条件で残る（0.640〜0.888倍）。

鎖の3引数 callback は GCC で約2.5〜7.9%の悪化を含む。星や鎖の短い操作では
コンパイラ・モードによって改善と悪化が混在する。部分木の対照測定にも
0.824〜1.066倍の幅があるため、その短縮を実装変更の効果とは数えず、
数%の差だけを理由に分岐や特殊処理を追加しなかった。

## 候補の採否と揺らぎ

候補1は構築と path の反復化に加え、LCA の比較に Euler 順を使った。
平衡木・ランダム木の LCA は0.611〜0.845倍だった一方、星形木では全4条件で
1.142〜1.420倍になった。この LCA の変更は採用しなかった。

候補2は head を局所変数に保持し、head の深さを比較して片側ずつ上る。
候補1と比べて平衡木・ランダム木の LCA の改善幅は小さくなるが、基準との比は
0.841〜0.967倍で、星形木は0.821〜1.019倍となった。構築と path の改善を保ちつつ、
星形木の一貫した悪化を避ける候補2を採用した。候補どうしは別の測定であり、
候補1の時間を候補2で直接割った比は報告していない。

中央値の比だけでは小さな差を判断しにくいため、同じ反復番号の比も
`paired-ratios.csv` に保存した。例えば候補1の Clang/assert/星/p2 V は
中央値の比が1.195、反復ごとの比の中央値が0.972で、悪化という解釈が安定しない。
採用版でも Clang/release/星/p2 E は0.997と0.802に分かれる。
このような条件の改善率は強い結論に使わない。

候補2の最初の測定中に、別の正しさ検査が
2026-10-04 02:29:23.327542〜02:29:56.751230 UTC に実行された。
その測定は採否に使わず、全条件を新しい `candidate2-clean` でやり直した。
一部の速いサンプルだけを選び直してはいない。
[重複実行の記録](candidate2/measurement-status.json)を残し、採否に使わなかった元データと
重複する compiler 別ログはローカルの `.verification/hld-bench-extra/` に保存している。

## 証跡と再現

- [候補1](candidate1/summary.csv): [raw](candidate1/raw.csv)、
  [環境](candidate1/environment.json)、[ヘッダ](candidate1/candidate.hpp)、
  [反復ごとの比](candidate1/paired-ratios.csv)。
- [採用版の全条件再測定](candidate2-clean/summary.csv):
  [raw](candidate2-clean/raw.csv)、[環境](candidate2-clean/environment.json)、
  [ヘッダ](candidate2-clean/candidate.hpp)、[反復ごとの比](candidate2-clean/paired-ratios.csv)。
- 両測定とも各1,792行。全反復で配置・callback 順序を含むチェックサムと回数が一致した。
  この一致確認は、独立した愚直解との比較テストを代替するものではない。

リポジトリのルートから、基準 commit と GCC/Clang が利用できるローカル環境で実行する。
同名の `raw.csv` がある場合、runner は上書きを拒否する。

```sh
unshare -Urn -- python3 -B benchmark/hld.py \
  --name reproduced --candidate-header benchmark/results/hld/candidate2-clean/candidate.hpp \
  --n 200000 --q 200000 --repeats 7 --cpu 2
python3 -B benchmark/hld-stats.py reproduced
```

[hld.cpp](../../hld.cpp) が測定本体、[hld.py](../../hld.py) がスナップショット・
コンパイル・集計、[hld-stats.py](../../hld-stats.py) が保存済みデータの感度確認を行う。
生成バイナリは `.benchmark/hld/` に置き、リポジトリには含めない。
保存する CSV/JSON は LF 改行に統一した。測定値・解析結果が同じことを確認し、元の
CRLF ファイルは `.verification/hld-bench-extra/line-endings/` に保存している。
採用版の測定後に `hld.py` へ加えた変更は、この出力改行の指定のみで、
測定本体 `hld.cpp` は変更していない。

## 最終版の実装と検証

構築では逆 DFS 順で部分木サイズを親へ集約し、隣接リストの走査を1回減らした。
一時的な DFS 順には `vertex_at_` を使い、探索と chain 列挙で stack を再利用する。
heavy child は構築中だけ保持し、構築後に残す整数配列は8本から7本になった。
path は u 側を直ちに列挙し、v 側の区間を局所配列に積んで逆順に取り出す。
配列長は int のビット幅で決まり、各 root-to-vertex path の light edge 数を収容できる。

公開 API は変更していない。2引数・3引数 callback の順序、同サイズ部分木の選択順、
`in/out/head` の配置も維持する。`distance` は中間値の int overflow を避ける形に直し、
`edge_index` は存在しない自己辺を assert で拒否する。

- [コンパイル・回帰検証](validation.json): GCC/Clang、GNU++20/23、assert/release のヘッダ検査、
  既存4 verify のコンパイル、新旧 random test、ASan/UBSan。計33コンパイル・32実行が成功。
- [基準版との独立比較](audit-final.json): 1,600構築・1,225,456頂点対で、配置と callback の
  区間・向きが一致。9,803,648回の path 呼び出しと再入可能な callback も確認した。
- [公式 checker 検証](official-final.json): Vertex Add Path Sum 19ケース、Vertex Add Subtree Sum
  16ケース、Vertex Set Path Composite 20ケース、Jump on Tree 21ケースを各3回、計228回受理。
  保存済み公式データ・checker のハッシュを固定 revision と照合し、ネットワークを遮断して実行した。
  この実行時間は改善率の比較には使っていない。
- [ドキュメント検証](docs-validation.json): 全109ページの構造検査と、HLD の4つの使用例を
  GCC/Clang、GNU++20/23でコンパイル・実行。追加テストの検証一覧も再生成し、coverage check が成功。
- [全体のローカル確認](preflight.json): GCC/GNU++20 の `make check` が成功。
  unit test 99件、全109ページの117使用例、110ヘッダ・147 verify のコンパイル、
  58本の random test 各20 seedを確認した。HLD の追加テストも通常の検証対象に含まれる。

新しい random test は seed 1 で1,629構築・86,618頂点対を調べる。
小さいラベル付き木の全探索、任意の根、親→子リストと無向リスト、辺・部分木の空区間、
ACL と組み合わせた辺更新、20万頂点の鎖を含む。
個別ログは各 JSON に記載したローカル `.verification/` 以下に保持した。
全体の `make check` は約969秒で完了し、終了時の空き容量は約37.0 GiBだった。
HLD の公式4問題以外を含む全体の `make verify`、`make docs`、Jekyll build は実行していない。
