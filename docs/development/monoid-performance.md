# モノイド recipe 初回性能調査（2026-09-29）

対象は `blueberry/algebra/monoids.hpp`。ACL の木を置換せず、`op` / `mapping` などの定数時間の代数演算を提供する。同等契約の比較では、同じ ACL `lazy_segtree`、同じ `modint998244353`、同じ `long long len` を使い、独立に書いた affine sum の式と recipe を比較した。

## 再現条件とログ

`benchmarks/monoids.cpp`、seed 712367、GCC 13.3.0 / Clang 18.1.3、`-O2 -std=gnu++20`。assert 有効と `-DNDEBUG` を分けた。Windows 上の WSL2 Linux 5.15.167.4 x86_64 で計測。N=1024 / 100000、Q=N / 4N、更新・問合せを交互に実行し、5 回ずつ計測順を交替。入力生成は測定外。構築は入力ノード生成を含み、操作から分離した。ログの checksum は両実装で一致する。

生ログは [GCC assert](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/monoid-measurements/g++-assert.log)、[GCC release](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/monoid-measurements/g++-release.log)、[Clang assert](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/monoid-measurements/clang++-assert.log)、[Clang release](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/monoid-measurements/clang++-release.log)。小さい入力、N/Q 比違い、全反復と I/O 単独測定を保持する。

[環境・ソースハッシュ](https://github.com/blueberry1001/Blueberry-library/blob/main/docs/development/monoid-measurements/environment.json)に CPU と測定対象を記録した。ログの時刻から、これらの測定は統合側の make check / docs / verify 開始前に完了していることを確認した。

| compiler / mode | recipe 構築中央値 ms | direct 構築中央値 ms | recipe 操作中央値 (min) ms | direct 操作中央値 (min) ms |
| --- | ---: | ---: | ---: | ---: |
| GCC assert | 0.605 | 0.543 | 161.017 (158.403) | 158.596 (146.287) |
| GCC release | 0.596 | 0.515 | 159.654 (146.243) | 159.245 (157.134) |
| Clang assert | 0.582 | 0.667 | 160.945 (148.945) | 163.755 (155.660) |
| Clang release | 0.643 | 0.602 | 161.090 (155.585) | 155.472 (153.506) |

表は N=100000,Q=400000。数 % の差には実行間の揺れがあり、recipe 固有の大きなオーバーヘッドは観測しなかった。木内部のデータ配列 + lazy 配列の payload は両者 5,242,880 bytes（allocator overhead、入力 vector、query vector、プロセス RSS は含めない）。I/O は tree から独立して 10 万整数の stream parse / format を測り、約 2.2 ms ずつ。ディスク・ネットワーク I/O や end-to-end LC の性能を表す値ではない。

再実行例:

{% raw %}
```sh
g++ -O2 -DNDEBUG -std=gnu++20 -I. -I.deps/ac-library benchmarks/monoids.cpp -o /tmp/monoid-bench
/tmp/monoid-bench
```
{% endraw %}

## Fastest ソース調査と採否

`scripts/fetch_lc_fastest.py` に `--limit 2 --save-source` を渡し、公式 API から取得した。取得時 leaderboard は [affine](monoid-measurements/fastest-affine.md) / [composite](monoid-measurements/fastest-composite.md)。第三者コードのコピーは行っていない。保存したソースはローカル `.benchmark/monoids/fastest-*` にある。

- [range affine range sum 402323](https://judge.yosupo.jp/submission/402323): AVX2 の wide lazy segment tree、500000 の固定容量プロファイル、全 query の先読みと 4 件先の prefetch、専用整数 parser / formatter。402233 も SIMD を利用する。一般の型・演算に載せる ACL recipe と契約が異なるため、その 82 ms を今回の操作時間と比較しない。
- [point set range composite 402223](https://judge.yosupo.jp/submission/402223): 全 query の保存、更新を delta に変換、逆元の一括処理、先読み prefetch、回答の後処理。402225 も SIMD を含む。オンラインで任意の ring の係数を扱う composition recipe の置換にはしない。

改善候補として、構造体を介さない直書き式に替える案を同条件で実測した。表の direct がその候補であり、速度差は小さく一貫しないため、API の短さを保つ recipe を採用した。`len` を `int` に狭める案は保証を弱めるため不採用。I/O の高速化は recipe 外であり今回のヘッダに混ぜない。8 recipe の演算は全て allocation を行わず、数個の算術・比較だけで構成される。Beats は既存の実装を使い、recipe の追加に木の再実装を含めない。

これは affine sum の代表測定であり、全 recipe / 全型の最速性を主張しない。Fastest の専用木を同等契約として再測定した結果でもない。初回の小さな callback 抽象化のコストを、直接式との比較で確認したものである。
