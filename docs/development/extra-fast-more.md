---
layout: page
title: Wavelet Matrix・FPS・集合の高速化調査
---

# 用途別の高速化調査

2026-10-20にLibrary CheckerのFastestを調査し、通常のライブラリを置き換えず、右上の「さらに高速なライブラリ」に実験版を追加しました。標準的な用途では引き続きACLと通常版を利用できます。`blueberry/all.hpp`には追加しません。

## 実装の範囲

| 実装 | 高速化の方針 | 主な条件 |
| --- | --- | --- |
| [Wavelet Matrix]({{ '/blueberry/extra-fast/wavelet-matrix.hpp.html' | relative_url }}) | rank情報とbit列の同居、POPCNT、座標圧縮の構築改善 | 静的な整数列、オンライン検索を維持 |
| [FPS]({{ '/blueberry/extra-fast/formal-power-series.hpp.html' | relative_url }}) | AVX2 Montgomery NTT、小段のlane shuffle、Newton反復の変換再利用 | 法998244353、uint32_t係数 |
| [Fenwick Tree]({{ '/blueberry/extra-fast/fenwick-tree.hpp.html' | relative_url }}) | 添字のpaddingと線形構築 | uint64_tの加算、prefix探索も提供 |
| [Affine Sum Segment Tree]({{ '/blueberry/extra-fast/affine-sum-segment-tree.hpp.html' | relative_url }}) | 16分の遅延伝播、SoA、AVX2 Montgomery | 法998244353、区間アフィン変換と区間和 |
| [Range Add Sum]({{ '/blueberry/extra-fast/range-add-sum.hpp.html' | relative_url }}) | 差分・重み付き差分を2本の多分木で保持 | 加算と和に限定、遅延伝播は使わない |
| [Ordered Map / Set]({{ '/blueberry/extra-fast/ordered-map.hpp.html' | relative_url }}) | 動的B+木とSIMDによるノード内探索 | 32/64bit整数キー、自然順序、順位検索あり |
| [Hash Map / Set]({{ '/blueberry/extra-fast/hash-map.hpp.html' | relative_url }}) | 16タグの同時比較、動的容量と削除 | 64bit以下の整数キー、順序なし |

Mapの値はtrivialな型に限定します。OrderedMap/Setには双方向反復子を追加し、Mapの実pairをrange-forで参照・更新できます。挿入・消去などで既存反復子が失効するため、標準コンテナの参照安定性とは異なります。整数キーの最小値・最大値を予約値にせず、そのまま保存できます。各APIページに空入力、overflow、コピー・移動、参照の失効条件を記載しています。

## Fastestのどこが速いか

FPSの[409662](https://judge.yosupo.jp/submission/409662)や[409370](https://judge.yosupo.jp/submission/409370)は、SIMD Montgomery乗算に加え、不完全NTT・高radix・最終Newton段の専用アルゴリズム・確保領域の再利用を組み合わせています。数千行の実装全体を移植せず、短いNTTと変換再利用に絞りました。残る差と棄却した候補も測定記録に残します。

Waveletの[403008](https://judge.yosupo.jp/submission/403008)・[393928](https://judge.yosupo.jp/submission/393928)は、問い合わせをすべて読んでから段ごとにまとめて処理します。通常のオンラインAPIとの倍率に混ぜず、`kth_smallest_batch`を別操作として追加しました。頻度問題のhashやoffline sweepによる専用解も、一般的なWaveletの高速化とは区別します。

順序付き集合の[409360](https://judge.yosupo.jp/submission/409360)は30bit固定の上位directory・部分和木と、下位12bitの小さい配列・SIMD列・bitmapを使い分ける構造です。sortedな初期集合のbulk構築、問い合わせ先読みとprefetch、専用I/Oも組み合わせています。全範囲の32/64bitキーを扱う汎用B+木とは探索構造が異なります。[410556](https://judge.yosupo.jp/submission/410556)などの全問い合わせの座標圧縮も、未知のキーを追加するオンラインAPIとは契約が異なります。今回は[Abseil B-tree設計](https://abseil.io/about/design/btree)も参考に、動的なノード分割・併合を独立に実装しました。

連想配列の[362188](https://judge.yosupo.jp/submission/362188)は、表本体が固定容量の線形探索で、SIMDの多くはI/Oにあります。新HashMapは[Swiss Tableのタグ比較](https://abseil.io/about/design/swisstables)を調査し、動的容量・削除に対応する表を独立に実装しました。名前にasmが現れるだけで、そのデータ構造がSIMD化されているとは判断しません。

BITでは依存するメモリ読み出しが中心です。無理に手書きasmを入れず、配置を改善しました。bounded universeのpredecessor最速群が使う多段bitmapには通常版FastSetが対応しているため、同じ構造の重複追加はしていません。

第三者の提出ソースは配布せず、取得先・日時・hash・分析を保存します。実装はアルゴリズムから独立に記述しています。

## 測定方法とログ

GCC / Clang、portable / native、assert有無を区別します。同じcompiler・flags・入力で複数回実行し、中央値と最小値を記録します。CPU affinityは固定しましたが、OSや他アプリの負荷までは制御していません。最終比較では担当間で重い測定を直列にし、測定中に別担当のコンパイルを重ねません。

構築・操作・入力・整形を分離した測定と、Fastest原本を含む入出力込みの測定を区別します。入力は固定seedで再生成でき、独立oracleとの出力照合を行います。保持量の式、構築中の一時領域、プロセス全体のpeak RSSも別の情報です。Python親プロセス由来の`ru_maxrss`が混入した初期測定は無効と明記し、原記録を残して別phaseで計り直します。

- [Waveletの測定記録](measurements/extra-fast-more/wavelet/README.md)
- [FPSの測定記録](measurements/extra-fast-more/fps/README.md)
- [区間加算・BIT・遅延セグ木の測定記録](measurements/extra-fast-more/lazy-bit/README.md)
- [set/mapの測定記録](measurements/extra-fast-more/sets/README.md)

提出ページの表示時間は環境が異なるため、ローカルとの倍率には使いません。失敗・中断・回帰の記録を削除せず、欠測を0で埋めません。

性能測定を完了した後、mainの既存FastIO改善PR55を統合しました。最終wholeやset/map・lazy測定はbase617acd6、FastIO SHA `b8b9c274f93c3906b8f6fc26dd945ec0679b3c246685da3108789aa677d73b30`の同一条件で行ったものです。新FastIO SHA `8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406`におけるI/O込み性能へ読み替えません。[旧版](measurements/extra-fast-more/fast-io-before-pr55.hpp.snapshot)と[統合後](measurements/extra-fast-more/fast-io-after-pr55.hpp.snapshot)を保持しました。公式verifyと統合テストは新FastIOで実行します。FPS・Waveletの前段階のI/O差も各記録で区別しています。

## 代表ケースの結果

条件を揃えた各測定の中央値です。表の行どうしは入力・flagsが異なるため、実装間の順位には使いません。

| 対象・条件 | 比較元 → 追加版 | 読み方 |
| --- | --- | --- |
| FPS convolution、GCC O2 release、入力長500000と499999 | 操作37.770 → 17.998ms | canonical係数の変換、root表、allocation込み |
| FPS inv / log / exp、同条件、出力長500000 | 54.686 / 97.096 / 94.769 → 23.964 / 41.226 / 39.588ms | 小さい入力やscalarでは改善しない場合もある |
| Wavelet online kth、GCC native、N=200000/Q=100000 | 操作23.709 → 19.175ms | 構築とオンライン操作を別測定。range_freqには回帰もある |
| 区間加算・和、GCC portable O2 release、N=500000/Q=100000 | 操作24.238 → 4.492ms | 汎用ACL lazyとの比較。加算だけに限定した構造の効果を含む |
| BIT、同条件 | 操作2.346 → 2.199ms | 改善は約6%。既存の多分木の方が速いケースもある |
| アフィン変換・和、GCC portable O2 release、N=500000/Q=100000 | 操作52.62 → 22.30ms | 法998244353に限定した遅延セグ木。Clangは54.09 → 17.79ms |
| OrderedMap、GCC portable O2 release、32bit乱数、N=50000/Q=100000 | std::mapの操作14.527 → 7.292ms | 64bit乱数のClangではほぼ同等。標準の全機能や参照安定性は提供しない |

FPSは全係数照合3840標本、Waveletは最終phase1200標本、区間加算・BITは840標本で出力が一致しました。Fastest原本との入出力込み比較では、FPSにまだ約2.5〜3.3倍の差が残ります。半分の係数だけを求める専用変換や最終Newton段の専用化などがあり、短い完全radix-2実装へ無条件に移せる差ではありません。保持量・RSSの増加、候補の回帰、原本とのAPI条件差は各測定記録に記載しています。

## 検証

固定seedの標準コンテナ・通常版・愚直解との照合、GCC/Clangとgnu++20/23、スカラー・intrinsics・GNU asmの両dialect、sanitizerによる個別検証が成功しました。Ordered/Hashの最終88コマンド、反復子42コマンド、lazy/BITの最終129コマンドもすべて成功しています。

最新FastIOを使い、新規13本と依存更新の影響を受けた既存分、計38本で公式全ケースを実行し、すべてACとなりました。FPSの警告修正後には影響する4本を新sourceから再実行してACを確認しました。無変更の過去記録を含め、167本すべてに現在の依存ソースと一致するAC記録があります。公式の時間は各1反復で、5反復の専用ベンチマークや旧I/Oの時間とは混ぜません。

初回のoj起動パス不足、生成一覧の隔離確認、GCCのshadow警告と修正履歴も統合検証ログに残します。公式ドライバのcompile成功と、公式checkerによる全ケースACは区別しています。

`make check`は149単体テスト、124文書例、123ヘッダ、167 verify compile、132 random programsの各20 seedで成功しました。GCC/ClangとGNU++20/23のrelease include検査、make docs、Jekyll、サイト検査、Edgeでの専用セクション・モバイル・キーボード・JavaScript無効時の確認も成功しています。[統合検証の原ログと再実行記録](measurements/extra-fast-more/integration-validation/README.md)を保存しました。
