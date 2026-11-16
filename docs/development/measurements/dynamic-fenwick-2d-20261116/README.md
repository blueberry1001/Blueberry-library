# Dynamic Fenwick Tree 2D の矩形和（2026-11-16）

整数の矩形和で、左右のx方向Fenwick経路が合流したら残りを省き、各セルのy方向はAVL木の区間を直接集約する。y方向が全域の場合は根の部分木和を使う。符号付き整数でも旧実装で扱える範囲を損なわないよう、中間和は同じ幅の符号なし整数で計算する。C++20の整数変換では、最終結果がTの範囲に収まる場合、剰余から元の値を得られる。浮動小数点・modint・ユーザー定義型は従来の4回のprefixによる計算順を維持する。API、対応する座標型、計算量、保持するノードは変えていない。

## 再現方法と条件

以下は同一の入力・同じコンパイラ・同じマシンで比較したローカル計測であり、Library Checkerの提出時間ではない。

```sh
mkdir -p /tmp/df2d-base/blueberry/data-structure
git show 5b1ac35cbe23dcdba1ca2613236b728cf6346dd1:blueberry/data-structure/dynamic-fenwick-tree-2d.hpp > /tmp/df2d-base/blueberry/data-structure/dynamic-fenwick-tree-2d.hpp
python3 benchmark/run_dynamic_fenwick_2d_sum.py \
  --baseline /tmp/df2d-base --candidate . --updates 20000 --queries 12000 \
  --repeats 5 --warmups 1 --scenarios dense,sparse,huge,huge64 --cpu 0 \
  --output .benchmark/dynamic-fenwick-2d-sum.json
```

CPU番号は実行環境に合わせて変更する。GCC 14.2.0、`-std=gnu++20 -O2 -DNDEBUG`、Xeon Platinum 8573C、単一CPUに固定し、warmup 1回の後に実測5回を交互に実行した。型は`long long`、seedは1729、入力生成は計時外。20,000点の更新は一様ランダムで、各形状につき12,000矩形の問合せを行う。短区間は長さ0〜16、広区間は領域の約半分以上、全域は`[0,n)`。狭い区間かつ疎な入力では、含まれる点が少ないケースが多い。各実装の入力・出力チェックサムはすべて一致した。

次の時間は**矩形和とチェックサムの計時のみ**の中央値（ミリ秒）で、前処理、更新、入出力、解放を含まない。全試行・最小値・最大値・各hashは [results.json](results.json) にある。

| 領域と矩形形状（x / y） | 変更前 | 変更後 |
| --- | ---: | ---: |
| 256² / 短・短 | 17.018 | 7.493 |
| 20,000² / 短・短 | 36.313 | 4.322 |
| 20,000² / 広・広 | 44.141 | 37.909 |
| 20,000² / 全・全 | 2.545 | 0.202 |
| (10⁹)² / 短・短 | 59.483 | 2.027 |
| (10⁹)² / 広・広 | 84.767 | 72.525 |
| (2⁶⁰)² / 短・広 | 85.278 | 1.906 |

20,000更新の中央値は順に12.058/12.058ms、36.271/35.580ms、70.524/68.894ms、150.303/154.070ms。更新実装は変更しておらず、共有マシンの揺らぎを含む。構築は空構造の生成だけを計時した。`/usr/bin/time`が利用できずRSSは未測定。メモリ・入出力・end-to-end・異なる値型での性能改善は主張しない。

## 試作と比較

[初回測定](trials-initial.json)は旧版、2つのy-prefix差をまとめた試作、AVL区間を直接集約する試作を比較した。たとえば20,000²の短・短は36.313msではなく別測定の38.657msから、それぞれ6.160ms、3.926ms。単純なAVL区間集約は20,000²の全・全で2.623→3.028msと回帰した。[次の測定](trials-full-y.json)ではy全域を根の合計で返し、この回帰を解消した。これらは最終測定とは別の試行であり、条件が同じでも個々の数値を混合しない。

試作の差分は [prefix差の統合](trial-fused.patch)、[AVL区間](trial-range.patch)、[y全域の特例](trial-range-full-y.patch)。当時のrunnerとdriverは、それぞれ`trial-run.py.txt`、`trial-benchmark.cpp.txt`として同じディレクトリに保存した。再実行時は`run.py`、`benchmark.cpp`に戻す。最終版は空の根を扱う分岐とコメント整理を加え、改めて`results.json`を測定した。
