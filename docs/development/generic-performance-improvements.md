---
title: 汎用性を保つ高速化の実装結果
---

# 汎用性を保つ高速化の実装結果

[Fastest調査]({{ '/docs/development/fastest-performance-plan.html' | relative_url }})で見つけた改善候補を、公開APIと対応型を維持して実装した。
基準版は`e0d8e71`。このページの時間は同じローカル環境での変更前後であり、
Library Checkerのランキング表示時間とは直接比較しない。

## 初回実装の完了条件

今後は「ACを確認した後で別作業として高速化する」を標準手順にしない。
実装開始時に対応型・代数・境界・更新と問い合わせの比率を決め、高速提出の工夫を分析し、
構築・操作・入出力・メモリを分けて測定する。既知の無駄や大きい説明不能な性能差は
初回の実装中に調査し、改善する。

すべてのCPU・型・入力で最適な実装は保証できない。
採用しなかった候補や遅くなる条件も記録し、将来同じ調査を繰り返さず判断できるようにする。
整数専用化、可換性の追加、同値の扱いの変更、特定ISAの必須化によって既存の汎用性を落とさない。
この手順を`IMPLEMENTATION_POLICY.md`と`AGENTS.md`に反映した。

## データ構造の変更

| 構造 | 減らした処理 | 維持する契約 |
| --- | --- | --- |
| LinearRMQ | 可変除算、余分な比較 | 線形構築・任意比較関数・同値なら左端 |
| SqrtTree | paddingのコピー・演算、不要なbetween、query時の層情報計算 | 任意monoid・非可換・空区間・既定構築不可型 |
| DisjointSparseTable | 入力と第0層の重複保存 | 単位元不要・結合法則のみ・非可換 |
| FastSet | 非空wordへの挿入時の不要な上位伝播 | 全universeと境界・重複挿入の返値 |
| WaveletMatrix | 座標圧縮時の二分探索（必要なbyteだけradix分配） | 符号付き/なし整数・重複・既存の全問い合わせ |
| StaticRangeLIS | 再帰ごとのscratch確保 | 比較可能な型・重複を除く狭義LIS |
| LinkCutTree | 存在しない子の単位元演算・途中の不要な再集約 | 非可換path積・root・link/cutの検査 |

各比較の環境・入力・中央値・最小値・全試行・source hashは、
[RMQ](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/rmq)、
[sequence構造](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/sequence)、
[Link Cut Tree](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/lct)、
[入出力](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/generic-performance/io)に保存する。
測定は固定CPUで逐次実行し、他のコンパイル・benchmarkを同時に走らせない。

LinearRMQは区間を左→中央→右の順に集約し、同値の場合の左端を1比較で保つ。
SqrtTreeはqueryで使わないpaddingや中間区間だけを省き、演算順は変えない。
LinkCutTreeは回転で降りる頂点の集約を先に確定し、上昇側は次に降りる時点またはsplay終了時に確定する。
反転の伝播は回転前に行い、非可換な順方向・逆方向の積を維持する。
単にassertや入力検査を削除して速くした変更ではない。

同じバッチ内の代表値（GCC 13.3、gnu++20、`-O2 -DNDEBUG`、単位ms）は次のとおり。
分布やN/Qが異なる行同士を比較する表ではない。

| 対象・条件 | 測定部分 | 変更前 | 変更後 |
| --- | --- | ---: | ---: |
| LinearRMQ、random、N=Q=500,000 | query | 16.38 | 8.92 |
| SqrtTree、非可換affine、N=524,289/Q=5,000,000 | 構築 | 59.01 | 28.23 |
| 同じSqrtTree条件 | query | 91.66 | 87.05 |
| WaveletMatrix、random、N=500,000/Q=200,000 | 構築 | 126.60 | 71.66 |
| StaticRangeLIS、random、N=65,536/Q=100,000 | 構築 | 408.58 | 155.35 |
| FastSet、universe=2^20、連続挿入1,000,000回 | 操作 | 3.68 | 2.26 |
| LinkCutTree、random木、非可換affine、N=8,192/Q=60,000 | 混合操作 | 42.98 | 35.88 |
| LinkCutTree、同じ条件の非可換2×2行列 | 混合操作 | 77.20 | 56.85 |

DSTはN要素の重複保存削減が主な改善。SqrtTreeは構築とメモリが主な改善であり、
すべてのquery分布が速くなったとは主張しない。SqrtTreeの途中候補で再現したquery回帰、
LinearRMQの比較順序による回帰、WaveletMatrixの比較sort候補の低σでの回帰も測定資料に残した。
WaveletMatrixは小さいfull-width入力も追加測定した。N=64の128bit配列では一律radixが
約3倍遅くなったため、Nと実際に必要な8bit桁数から比較sortへ切り替える。
符号や型の値域を制限せず、大きい入力だけに有利な固定閾値を避けた。
LinkCutTreeの混合操作はprodだけでなくset/get、evert/leader、same、失敗するlink/cutも含む。
上記affine/random条件の演算呼び出し回数は11,882,016→5,609,264回。
計数は別binaryで行い、計数の費用を速度測定へ混ぜていない。

## 入出力

新しい[Fast I/O]({{ '/blueberry/utility/fast-io.hpp.html' | relative_url }})は64 KiBのbufferを使い、整数出力を2桁ずつ変換する。
初期bufferの全zero-fillを省き、overflow境界は型ごとの定数として計算する。
通常ファイルだけでなくpipe、符号、128bit、EOF、文字列、浮動小数点も扱う。
対話問題は`FastInput<true>`と明示的な`flush()`で扱い、batch入力が64 KiBを待つ問題を避ける。
既存I/Oを強制的に置き換えることなく、必要な提出から短く使える独立headerとした。

I/Oの効果と構造の効果を混同しないよう、旧/新SqrtTreeとiostream/Fast I/Oを交差比較する。
file/pipe、GCC/Clang、assert有効/無効を分け、毎回すべての出力を公式期待出力と照合する。

公式Static RMQ `max_random_00`、N=Q=500,000、GCC `-O2 -DNDEBUG`、通常ファイル入力で、
旧Sqrt+iostreamは73.07ms、新Sqrt+Fast I/Oは31.31ms（入力・構築・query・出力flushの合計）。
同じ旧SqrtのままI/Oだけ変えても34.87msになるため、この改善の大部分はI/Oに由来する。
pipeでも84.51→38.16ms。Clangのfile条件は76.02→35.88msだった。
これは同環境の各5回の中央値であり、最速提出への追いつきを表す数値ではない。
360測定と72warmupの全出力、対話用pipeの往復も成功した。

## 正しさと公開時の確認

各構造はGCC/Clang×gnu++20/23、固定seedの愚直比較とASan/UBSanで確認した。
非可換な演算、既定構築不可型、同値の左端、失敗した木の操作後の根、整数の全幅を対象に含めた。
統合後の`make check`では102ライブラリの文書と106実行例、全header・139 verifyのコンパイル、
94 random programの各20seedが成功した。
GCCの`-DNDEBUG`設定で全headerの単独・重複includeも成功した。
問題別対応表は、補助utilityの除外と明示mappingの許可を追加した18件のunit testでも確認した。

公式verifyの実行状態・対象revision・測定値は[測定一覧]({{ '/benchmarks.html' | relative_url }})と
各verifyページに掲載する。対象外・未実行・失敗を成功や0msへ置き換えない。
補助I/Oを使っただけの問題に、Fast I/Oをその問題のデータ構造実装として表示しない。
