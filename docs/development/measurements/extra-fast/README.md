---
title: 整数和の高速セグ木の比較測定
---

# 整数和の高速セグ木の比較測定

`SumSegmentTree`、ACLのFenwick Tree・Segment Tree、既存の`blueberry::SegmentTree`を、
同じ入力と`uint64_t`の加算契約で比較する。演算はすべてmodulo 2^64であり、
任意の型・モノイドへ対応する既存セグ木を代替する測定ではない。

## 測定方法

CPUはIntel Core i7-14650HX、実行環境はUbuntu 24.04のWSL2。
乱数seedは20261019、各候補を5回ずつ新しいプロセスで実行する。
候補順は入力と反復番号ごとに回転させる。初回はCPUを固定していない。
候補の採否と最終測定は`taskset -c 0`で論理CPU 0へ固定し、`sched_getaffinity`の結果を保存する。
OS全体の負荷制御は行っていない。
コンパイラのversion、flags、ソースのSHA-256、作業状態は各実行の`environment.json`に保存する。

N=4096、65536、1048576はQ=200000とし、広いランダム区間と更新を半々にする。
N=262144、Q=50000の問い合わせ9割のケース、N=65536で長さ0〜32の短区間、
更新9割のケースも用意する。通常の更新は`add`と`set`を半々にする。
最終測定にはN=65536、Q=200000、更新5割で`add`だけを使う`point-add`も追加した。
これはPoint Add Range Sumと同じ操作の組み合わせであり、他のケースでは公開APIの`set`も評価する。
各候補へ同じテキスト入力を渡し、入力hash・出力hash・入出力バイト数の一致を確認する。

生成済みテキストの`from_chars`によるparse、木の構築、操作、`to_chars`によるformatを分けて測る。
操作時間には回答配列の確保・書き込みを含め、すべての候補で揃える。
end-to-endはparse開始からformat終了までで、入力生成、OSへのファイル入出力、木の破棄を含まない。
各phaseの中央値の和は、end-to-endの中央値と必ずしも一致しない。

メモリは別プロセスで、初期値vectorと構築済みの木だけを保持して測る。
入力テキスト・操作列・出力配列は生成しない。実行ファイルやallocatorも含むプロセスのRSSであり、
データ構造の配列容量そのものとは区別する。ACL Fenwickの構築は公開APIの`add`をN回呼ぶO(N log N)、
他の3実装のvector構築はO(N)である。

## 初回の評価

[初回の生ログ](initial-gcc-o2/samples.jsonl)と[集計](initial-gcc-o2/summary.json)は、
GCC 13.3.0、`-std=gnu++20 -O2 -DNDEBUG`での結果。
この時点の既定は手書きasmで、全ノードの4つのvectorを更新する初期実装だった。
[旧headerのsnapshot](initial-gcc-o2/segment-tree.hpp.snapshot)とそのSHA-256を保存している。
300行の測定を完了し、入力・出力はすべて一致した。
以下はQ=200000、広いランダム区間、更新5割の操作時間の中央値。単位はms。

| N | SIMD asm | SIMD intrinsics | scalar | ACL Fenwick | ACL segtree | 既存SegmentTree |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 4096 | 2.108 | 2.119 | 4.408 | 3.493 | 6.622 | 6.593 |
| 65536 | 2.526 | 2.467 | 5.691 | 3.885 | 8.799 | 9.026 |
| 1048576 | 3.164 | 3.035 | 6.898 | 8.185 | 13.753 | 14.132 |

手書き`vpaddq`がintrinsicsより速いという結果は得られなかった。
この結果を受けて既定はintrinsicsとし、手書きasmを選べる実験用macroを残す。
速度差の中心は16分木のprefix配置とSIMDによるまとめた更新にある。

初回のRSSには`getrusage`を用いたが、Python親プロセスのexec前の最大RSSを引き継ぎ、
小さいNでは約19MiBが下限になることが分かった。小さいNの初回RSSからメモリ差を判断しない。
以後は当該実行ファイルのaddress spaceに対応する`/proc/self/status`の`VmHWM`を使う。
初回の値と環境情報は訂正で上書きせず保存した。

初回の起動時にはWindows作成worktreeのGitパスをWSLのGitが解釈できず、測定前に失敗した。
[起動失敗ログ](initial/bootstrap-failure.log)を保存し、runner側でパスを変換して再実行した。
この失敗を0msや成功回数に含めない。

## 改善候補の採否

### 変更しないvectorの省略

更新位置より前のmaskはすべて0になる。そこで内側のループの開始位置を`j=0`から`j=p&12`へ変え、
平均の更新vector数を4から2.5へ減らす候補を試した。
初回との単純な前後比較を避け、当時のheaderのループ開始位置だけを0へ戻した対照版を用意した。
memberの配置、既定backend、GCCのflagsは揃え、同じCPU 0・入力・実行内で比較した。

[350行の生ログ](skipped-vector-candidate/samples.jsonl)と[集計](skipped-vector-candidate/summary.json)を保存した。
すべて入力・出力が一致した。以下はintrinsicsでの操作時間の中央値。単位はms。

| 入力 | 固定4vectorの対照版 | vectorを省略する候補 |
| --- | ---: | ---: |
| N=4096、広区間・更新5割 | 1.992 | 3.291 |
| N=65536、広区間・更新5割 | 2.488 | 3.982 |
| N=65536、短区間・更新5割 | 2.384 | 3.771 |
| N=65536、広区間・更新9割 | 2.490 | 5.199 |
| N=1048576、広区間・更新5割 | 3.058 | 4.985 |

省略版は遅いため棄却し、固定4vectorへ戻した。
[GCC O2の固定版の命令列](update-disassembly/fixed-four.s)と[省略版](update-disassembly/skipped-vectors.s)では、
両方ともvector処理のループが残っている。固定版は32byteずつ進めて128byteで終了し、
省略版は点の位置から計算した添字で1〜4回反復する。
可変の反復回数による分岐予測や添字計算の負担が原因と考えられるが、性能counterでは確認していない。
「固定版だけループ展開される」とは説明しない。

### 構築時のprefixのSIMD化

Library Checker提出257591にもある構築時のprefix計算のSIMD化を、独立した
[候補コード](prefix-candidates.cpp)で評価した。16要素nodeの配置、確保・0初期化・入力copyを揃え、
scalarの逐次加算と、4lane内のscanとcarryを使うAVX2を比較した。
測定前には境界Nとunsignedの折り返しを含めて全内部prefixの一致を確認し、構築後の総和も確認した。

[120行の生ログ](prefix-candidate/samples.jsonl)と[集計](prefix-candidate/summary.json)を保存した。
N=1048576におけるprefix計算だけの中央値は次の通り。単位はms。

| コンパイラ・flags | scalar | SIMD |
| --- | ---: | ---: |
| GCC 13、portable O2 release | 0.334 | 0.315 |
| GCC 13、native O3 release | 0.416 | 0.366 |
| Clang 18、portable O2 release | 0.438 | 0.404 |
| Clang 18、native O3 release | 0.358 | 0.314 |

prefix自体は短縮したが、差は0.019〜0.050msだった。確保・初期化・copyの1.2〜1.8msが支配的で、
構築全体に対する差は小さい。構築全体の中央値には確保時間の揺らぎも含まれるため、
その差のすべてをSIMDの効果として説明しない。
二つの構築経路を維持する複雑さに見合わないと判断し、今回の公開実装には採用しなかった。

## 最終測定

最終版は固定4vector更新、intrinsicsが既定、手書きasmは明示指定とした。
GCC 13とClang 18それぞれで、portable O2 release、portable O2 assert有効、native O3 releaseを測る。
各入力・コンパイラ・flagsの中で候補を比較し、異なる入力の時間を合算しない。
CPU固定の条件が異なるため、初回から最終版までの改善率は算出しない。

Fenwickの`set`は公開APIの区間和から現在値を取得して差分を加算する。
したがって`add`と`set`を混ぜた結果を、Library CheckerのPoint Add Range Sumへそのまま一般化しない。
問題と同じ`add`と区間和だけの比較には`point-add`の行を使う。

[最終の生ログ](final/samples.jsonl)、[集計](final/summary.json)、[環境・hash](final/environment.json)、
[コンパイル記録](final/compile.log)を保存した。6構成×7入力で1980行の測定を完了し、
入力・出力の不一致と実行失敗は0件だった。ソースが測定中に変わっていないことも確認した。

N=65536、Q=200000の`point-add`、portable O2 releaseの中央値は次の通り。単位はms。

| 候補 | GCC 操作 | GCC end-to-end | Clang 操作 | Clang end-to-end |
| --- | ---: | ---: | ---: | ---: |
| 高速版 intrinsics（既定） | 2.442 | 10.817 | 2.118 | 9.003 |
| 高速版 手書きasm | 2.456 | 10.908 | 2.193 | 8.920 |
| 高速版 scalar | 6.342 | 14.919 | 5.068 | 12.157 |
| ACL Fenwick | 3.732 | 12.554 | 3.700 | 11.206 |
| ACL segtree | 9.394 | 17.834 | 9.218 | 16.464 |
| 既存SegmentTree | 9.274 | 17.963 | 8.493 | 15.505 |

同じ入力で高速版intrinsicsの各phaseを分けると、次の通りだった。
parseとformatの比率が大きいため、操作単体の短縮率をプログラム全体の短縮率として扱わない。

| コンパイラ・flags | parse | 構築 | 操作 | format | end-to-end |
| --- | ---: | ---: | ---: | ---: | ---: |
| GCC portable O2 release | 5.802 | 0.196 | 2.442 | 2.464 | 10.817 |
| GCC portable O2 assert有効 | 5.625 | 0.171 | 2.245 | 2.365 | 10.386 |
| GCC native O3 release | 5.064 | 0.163 | 1.883 | 2.252 | 9.394 |
| Clang portable O2 release | 4.656 | 0.156 | 2.118 | 2.043 | 9.003 |
| Clang portable O2 assert有効 | 5.135 | 0.166 | 2.150 | 2.028 | 9.235 |
| Clang native O3 release | 4.563 | 0.137 | 1.922 | 1.938 | 8.542 |

assert有効で少し短い行もあるが、別々の実行の揺らぎを含むためassertによる高速化とは判断しない。
手書きasmもすべての入力・flagsでintrinsicsより速いわけではない。
scalar版はAVX2非対応CPUでも動く経路として維持するが、この環境では更新の多い入力で遅い。

N=1048576、portable O2 releaseの独立したメモリ測定では、GCCのRSS中央値は
高速版intrinsicsが20096KiB、ACL Fenwickが19584KiB、ACL segtreeが27680KiBだった。
配列の要素だけなら高速版は8947840byte、ACL Fenwickは8388608byte、ACL segtreeは16777216byteとなる。
この入力では高速版の配列はFenwickより大きい。

## 再実行

`run.py`は空でない出力ディレクトリへの上書きを拒否する。
`environment.json`、`compile.log`、`samples.jsonl`、`summary.json`、`failures.json`と、
自作の測定対象・測定コードのsnapshotを保存する。第三者ライブラリはhashを記録する。

{% raw %}
```sh
python3 docs/development/measurements/extra-fast/run.py \
  --output .benchmark/extra-fast/local \
  --acl-root .deps/ac-library \
  --configuration portable-o2-release \
  --configuration portable-o2-debug \
  --configuration native-o3-release
```
{% endraw %}

指定しなければGCC・Clangの両方を使う。各コンパイラ内で同じflags・同じ入力を比較し、
異なるコンパイラの値を合算しない。`--repeats`で反復回数、`--quick`で最初の2入力への絞り込みを指定できる。
通常の最適化以外に、`BLUEBERRY_EXTRA_FAST_USE_ASM`、intrinsics、
`BLUEBERRY_EXTRA_FAST_FORCE_SCALAR`を分けてコンパイルする。
