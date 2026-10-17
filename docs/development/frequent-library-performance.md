---
title: 頻出ライブラリの高速化調査と実装（2026-10）
---

# 頻出ライブラリの高速化調査と実装（2026-10）

競プロで繰り返し使う機会が多いと考えられる木・最短路・区間問い合わせ・
重み付きUnion Find・文字列・FPS・入出力を調べた。
利用頻度の実測データによる順位ではない。標準のDSU、SegTree、Fenwick、
modint、convolution、suffix array、Z algorithmはACLを優先する。

変更前は`d6af3e9`。公開名・引数・返り値・対応型・オンラインの操作順を維持し、
既存実装が行っていた不要な処理を減らす。

## 調査と採用範囲

| 対象 | 調べた方法 | 採用方針 |
| --- | --- | --- |
| Doubling LCA | 深さに応じた親表、queryの走査上限、set bitだけの祖先移動 | 同じ木とqueryで候補を個別比較 |
| RMQ LCA | Euler列の縮小、親preorderのmin、Fastestの固定ブロック | 任意root・頂点番号・非再帰構築を維持 |
| Li Chao Tree | 座標探索の重複削減、祖先の直線による線分の早期除外 | overflowの前提を変えず有効区間内で比較 |
| Potential Union Find | root直下で不要な単位元合成を省略 | 非可換群・矛盾検出を維持 |
| FPS | 積分の逆元漸化式、係数コピー、Horner法、疎密の切替 | 今回は積分の線形化を測定・実装 |
| Dijkstra | radix heap、d-ary heap、reserve、CSR | 型・親のtie・到達率による条件差を記録。今回は変更なし |
| Wavelet Matrix | rank配置、Weighted版の圧縮 | 通常版は改善済み。追加案は未測定として記録 |
| 文字列・高速I/O・Rollback UF | mapのコピー/探索、buffer処理、undoの重複書き戻し | 各候補の測定軸を記録。今回は変更なし |

[分野別の調査・一次資料・制約の比較](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/measurements/frequent-2026-10/research.md)に、
今回未採用の候補も残した。HLDには別の未統合branchがあるため本体を変更しなかった。

FPSの積分は、素数modulusとN<modという既存の条件のもとで、
逆元の漸化式を出力領域に計算した後で係数を掛ける。
計算量はO(N log mod)からO(N)となり、出力以外のO(N)配列や共有cacheを追加しない。
空の積分は従来どおり`{0}`を返す。

Potential Union Findのrootの重みは単位元である。
親がrootなら現在の重みが既にrootからの差分なので、再帰・群演算・同じ値の代入を省ける。
後からrootが別成分に併合された場合には通常の経路圧縮を行う。
非可換群での合成順序や償却計算量は変えない。

## 測定方法と記録

測定は分野ごとに順番を決め、別のbenchmarkやcompile/testを同時に動かさず実行する。
同じCPU・入力・compiler・flagsでbaselineと候補を比較し、warmup後の5試行を保存する。
GCC/Clangとassert有効/無効を区別する。構築と操作は別に計時する。
I/Oは今回の本体変更に含まれず、測定器の入力生成・最終出力は計時区間から外す。
process RSSは入力やruntimeも含むため、データ構造だけの使用量とは呼ばない。

- [LCAの候補・Fastest調査・生ログ](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/frequent-2026-10/tree)
- [Li Chao Treeの候補・生ログ](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/frequent-2026-10/li-chao)
- [FPS・Potential UFの測定器と生ログ](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/frequent-2026-10/core)

Library Checkerのランキング時間は異なる環境の最大ケース時間であり、
このローカル比較の集計値へ混ぜない。欠損や失敗を0秒として補わない。

## 実測結果

下表は各設定で5回測った中央値で、単位はms。
Core i7-14650HX / WSL2 / GCC13.3 / `-std=gnu++20 -O2 -DNDEBUG`。
分野ごとにCPU affinityを固定し、同じログ内の変更前後だけを比較している。
入力・操作数が異なる行同士の比較や、全環境での改善率の保証には使わない。

| 対象と条件 | 測定部分 | 変更前 | 変更後 |
| --- | --- | ---: | ---: |
| Doubling LCA、random木、N=200000/Q=500000 | LCA操作 | 50.740 | 18.720 |
| 同じ木と操作数 | 祖先操作 | 5.400 | 2.910 |
| RMQ LCA、同じ木と操作数 | 構築 | 23.480 | 7.150 |
| 同じRMQ LCA | 構築＋操作 | 28.420 | 12.040 |
| 圧縮Li Chao、random-segment、追加/query各100000回 | 混在操作 | 55.434 | 27.226 |
| 動的Li Chao、同じ操作列 | 混在操作 | 88.511 | 9.298 |
| FPS、N=262144、8回 | 積分＋checksum | 135.130 | 6.084 |
| FPS、N=65536、8回 | log＋checksum | 105.437 | 71.070 |
| Potential UF、加算群、N=65536/Q=1000000 | merge/same/diff混在 | 6.819 | 5.187 |
| 同じN/Q、2×2行列群 | merge/same/diff混在 | 22.642 | 10.664 |

RMQ LCAの同じrandom木のprocess peak RSSは80368→33508KiB。
動的Li Chaoのrandom-segmentは87008→10004KiBだった。
前者は共通入力とruntime、後者は共通入力・runtimeと測定器が作る2個の木を含む。
データ構造単体のメモリや、両者の比率を示す値ではない。

Clang18.1.3でも、FPS積分は149.566→5.807ms、logは113.793→76.857ms、
行列群UFの混在操作は25.589→12.117msだった。
GCCのassert有効時も別に測定し、結果を各分野の生ログに保存した。

## 回帰と不採用候補

- RMQ LCAは頂点への逆引きが1load増え、一部のquery単体は遅くなる。
  例としてClangのパス木では4.82→5.32msだが、構築込みでは18.49→9.65ms。
  逆引きをなくす64bit配置も測り、int版よりRSSが約13MiB増えて合計も概ね悪化したため棄却した。
- 圧縮Li Chaoの葉→親query案はClangのduplicate-lineで2.039→2.888msとなり棄却した。
  採用した単一下降案ではCPU2固定の同条件で1.928→1.672msだった。
  初回と追加測定はCPU条件が違うため、それぞれのログ内のbaselineだけと比較する。
- 採用版にも、Clangのquery-heavy混在処理13.783→14.292msという中央値の回帰が残る。
  全5試行の範囲は重なり、query単独は12.024→11.860msだった。
  狭い線分の更新単独にも追加比較の負担がある。広い線分だけの数字を全入力へ一般化しない。
- 変更しないFPS eval/multiplyにも数%の差が出る。測定やcode配置の揺れを含むため、
  それらを今回の高速化の成果とは扱わない。

分布・compiler・assert設定別の最小値、全試行、棄却理由は測定ディレクトリを参照する。
