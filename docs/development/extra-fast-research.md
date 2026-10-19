---
title: さらに高速なライブラリのFastest調査
---

# さらに高速なライブラリのFastest調査

2026年10月19日、Library Checker公式REST APIから4問題の上位AC提出を取得した。
セグ木本体では、手書きアセンブラよりも、SIMD命令を使いやすい多分木・prefix配列の設計が目立つ。
手書きアセンブラを使う提出も確認できたが、今回見つかった箇所は高速I/Oのシステムコールだった。
既存の汎用ライブラリは維持し、型・演算・CPUの条件を明示した独立版の設計資料とする。

## 調査範囲と記録

`status=AC&order=+time` で、Point Add Range Sum、Point Set Range Composite、Static RMQは各50件、
Range Affine Range Sumは30件、計180提出を取得した。ユーザーの重複は除外していない。
順位は同率を含む取得時のAPIの並び順で、公開時間は最大テストケース時間である。
提出の言語タグ・時期・I/O・前提条件が異なるため、表示時間だけから実装の優劣や改善率を決めない。

取得には `scripts/fetch_lc_fastest.py --save-source` を使用した。
ソースの全180件を機械検索し、以下の代表提出とアセンブラを含む提出を読んだ。
取得日時、API query、提出日時、ユーザー、言語、公開時間、メモリ、保存ソースのSHA-256、
命令名・ライセンス表記の手掛かりを次に保存した。第三者ソースそのものはgit管理外の `.benchmark/extra-fast/` にだけ置く。

- [Point Add Range Sumの50件](measurements/extra-fast/research-point_add_range_sum.json)
- [Point Set Range Compositeの50件](measurements/extra-fast/research-point_set_range_composite.json)
- [Static RMQの50件](measurements/extra-fast/research-staticrmq.json)
- [Range Affine Range Sumの30件](measurements/extra-fast/research-range_affine_range_sum.json)

初回のsandbox内取得はDNS解決に失敗した。権限付きの読み取りで再実行し、上記の取得はすべて完了した。
これらはソース調査の記録であり、ローカル測定の代わりにはしない。

| 問題 | 順位 | 提出・ユーザー | 提出日 UTC | 公開最大時間 | ソースで確認した点 |
| --- | ---: | --- | --- | ---: | --- |
| Point Add Range Sum | 1 | [403014 / nandhagk](https://judge.yosupo.jp/submission/403014) | 2026-09-16 | 23 ms | 16分木、unsigned prefix sum、AVX2のmasked加算 |
| Point Add Range Sum | 2 | [258491 / mukundan314](https://judge.yosupo.jp/submission/258491) | 2024-12-26 | 23 ms | 16分木、GCCの32-byte vector型、64-byte alignment |
| Point Add Range Sum | 4 | [257591 / mukundan314](https://judge.yosupo.jp/submission/257591) | 2024-12-21 | 23 ms | 同方式、構築時にもAVX2/SSE prefix計算 |
| Point Set Range Composite | 1 | [402223 / nandhagk](https://judge.yosupo.jp/submission/402223) | 2026-09-14 | 49 ms | inverse prefix tree、AVX2 Montgomery、batch inverse、先読み |
| Point Set Range Composite | 4 | [278231 / chaihf](https://judge.yosupo.jp/submission/278231) | 2025-04-09 | 71 ms | 葉を逆順配置した二分木、bit走査で必要なnodeだけ訪問 |
| Point Set Range Composite | 7 | [268862 / chaihf](https://judge.yosupo.jp/submission/268862) | 2025-02-21 | 73 ms | 同系統の二分木、modulusと係数範囲を固定 |
| Point Set Range Composite | 27 | [333229 / nandhagk](https://judge.yosupo.jp/submission/333229) | 2025-12-01 | 88 ms | blazingioのinline asmによるreadシステムコール |
| Static RMQ | 1 | [402567 / nandhagk](https://judge.yosupo.jp/submission/402567) | 2026-09-15 | 19 ms | blocked sparse table、AVX2 min、8問い合わせ単位の処理 |
| Static RMQ | 2 | [362370 / IceKylin](https://judge.yosupo.jp/submission/362370) | 2026-03-26 | 20 ms | 16要素のprefix/suffix、短区間はscalar走査、SWAR I/O |
| Range Affine Range Sum | 1 | [402323 / nandhagk](https://judge.yosupo.jp/submission/402323) | 2026-09-14 | 82 ms | 16分木の遅延伝播、AVX2 block処理、4操作先prefetch |
| Range Affine Range Sum | 2 | [408770 / nandhagk](https://judge.yosupo.jp/submission/408770) | 2026-10-08 | 88 ms | 多分木の遅延伝播、SIMD、全操作を先読みしてprefetch |

## 手書きアセンブラとSIMDを区別する

180件のソースに対する `asm` / `__asm__` / `__asm` の検索では、Point Set Range Compositeの7件が該当した。
[333229](https://judge.yosupo.jp/submission/333229)、[333232](https://judge.yosupo.jp/submission/333232)、
[331457](https://judge.yosupo.jp/submission/331457)、[333338](https://judge.yosupo.jp/submission/333338)、
[333335](https://judge.yosupo.jp/submission/333335)、[333572](https://judge.yosupo.jp/submission/333572)、
[333337](https://judge.yosupo.jp/submission/333337)のembedded blazingioである。
x86-64用の `syscall`、x86-32用の `int $128`、AArch64用の `svc 0` が条件付きで書かれている。
I/Oでlibcを経由せずOSを呼ぶ仕組みであり、セグ木の結合処理をアセンブラへ置き換えた例とは区別する。
同じソース内にある別ISAの命令が、その提出の実行時にも使われたとは限らない。

一方、Point Add Range Sumの上位50件には、これらのinline asm表記はなかった。
[258491](https://judge.yosupo.jp/submission/258491)はGCCの `vector_size(32)` で4本の64-bit整数をまとめて加算する。
[403014](https://judge.yosupo.jp/submission/403014)はintrinsicsで同じ種類のSIMD演算を行う。
`#pragma GCC target`、intrinsics、コンパイラのvector型、手書きinline asmはそれぞれ別の手段である。
`fetch_lc_fastest.py` の `uses_simd` は浅い文字列検索なので、採用判断には実際の呼び出し箇所を確認した。

独立版でGNU inline asmの `vpaddq` を試す場合、それは上位提出の同一行を移植したものではない。
SIMDで処理できるデータ配置を学び、同じ加算をintrinsicsと手書き命令で実装して比較する実験になる。
同じ機械語になる場合や遅くなる場合もあるため、asmを含むこと自体を高速化の根拠にしない。

## セグ木へ取り入れる技術

16分木では、各nodeに16個の子のprefixを連続配置する。
点加算で変化するのは、その点以降のprefixだけなので、lane maskと一括加算で更新できる。
区間和は左右のprefixの差を取る。木の高さが小さくなり、問い合わせで読む値と依存するloadの回数が減る。
この設計と比較の背景は、提出内でも参照される
[AlgorithmicaのSegment Trees](https://en.algorithmica.org/hpc/data-structures/segment-trees/)に説明されている。
命令とISAの対応は[Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)で確認できる。

この方法は加算と減算ができる整数和向けである。任意のモノイドの積や非可換演算へはそのまま拡張できない。
固定 `MAX_N=500000`、global storage、32-bitの更新引数を使う提出もあるため、
独立版では動的サイズ、インスタンスごとの所有権、空入力・空区間、uint64全域の値と更新を別途設計する。
`uint64_t` の演算をmodulo 2^64で定義すれば、SIMD laneのwraparoundと一致する。
通常の符号付き整数和と同じ契約だとは説明しない。

推奨APIは専用namespace/パスに置いた点加算・区間和の型で、短い `add`、`set`、`get`、`prod`、
`all_prod`、`size` を持たせる。構築O(n)、操作O(log n)、メモリO(n)を目標とする。
AVX2を選ぶ方法、非対応CPUの扱い、GCC/ClangとGNU拡張への依存を公開ページに書く。
汎用セグ木やACLの利用案内、既存の入口ヘッダは維持する。

## RMQ・関数合成・遅延伝播の判断

Static RMQの上位には、各ブロックのprefix/suffixと、ブロック単位のSparse Tableを組み合わせたものがある。
短い区間と構築にSIMD minが使える。min専用の独立版には適するが、任意の冪等演算や比較器、
左端argminなどの既存保証まで自動的に満たすものではない。
詳しい構造とI/Oの分離は[既存のRMQソース調査](fastest-rmq-source-study.md)も参照する。

関数合成の[402223](https://judge.yosupo.jp/submission/402223)は、更新係数と回答の分母の逆元をまとめて計算する。
全操作を保持し、更新の局所差分を先に求め、数操作先のメモリをprefetchする。
係数の逆元が存在するという問題側の条件、固定modulus、入力全体が既知であることに依存する。
AVX2を入れた汎用オンラインsegtreeとの同条件比較にはならない。
[278231](https://judge.yosupo.jp/submission/278231)のbit走査は汎用化の調査候補だが、
提出は関数をxへ直接適用する契約でもあるため、合成結果全体を返すAPIとは測定を分ける。

遅延伝播の[402323](https://judge.yosupo.jp/submission/402323)は、16分木の子要約とlazy tagをAVX2で処理する。
modular multiplyでは32-bit laneを偶数・奇数に分けて64-bit積を取り、Montgomery reductionを行う。
全操作を先読みしたprefetchと、範囲・桁数を固定したI/Oも含む。
可換性・作用の合成順・区間長・法の条件を整理する必要があり、最初の整数和の独立版と一度に混ぜない。

## 出典・ライセンスと採用条件

代表提出258491、403014、402223、402567、402323の保存ソースから、提出全体に適用できるライセンス表記は確認できなかった。
[376181](https://judge.yosupo.jp/submission/376181)などにはIceKylin由来I/OのMIT表記があるが、
その表記だけで提出全体の権利を判断しない。
inline asmを含む提出には[blazingioのリポジトリ](https://github.com/purplesyringa/blazingio)への出典がある。
ライセンスが未確認のコードは転載せず、今回も第三者コードをライブラリへコピーしない。

採否は同じコンパイラ・フラグ・CPU・入力で、構築・操作・I/O・メモリを分けて決める。
点加算/区間和の割合、nとq、短区間/広区間/偏った添字、GCC/Clang、assert有効/releaseを区別し、
intrinsics、inline asm、scalar、ACLを比較する。組み立てた命令列も確認する。
ここに記載したLibrary Checkerの時間をローカル版の高速化率として使わない。
測定の欠損や失敗は欠損・失敗のまま残し、別の入力やrevisionの結果で埋めない。
