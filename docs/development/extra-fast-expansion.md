---
title: 高速版の追加と403014を参考にした区間和の改善
---

# 高速版の追加と403014を参考にした区間和の改善

2026年10月19〜20日。既存の汎用ライブラリを維持し、専用セクションの区間和を改善するとともに、
静的RMQとbitsetを追加した。通常の一覧、検索、`blueberry/all.hpp`には追加していない。
測定条件・候補の採否・失敗ログは[比較記録](measurements/extra-fast-expansion/README.md)、
検証と公開手順の結果は[検証記録](extra-fast-expansion-validation.md)に置く。

## 抽象化できる範囲

[Library Checker提出403014](https://judge.yosupo.jp/submission/403014)は、大きな共通フレームワークを持つが、
実際に使う区間和はunsigned整数の加算専用である。fanout・最大長・段数・backendをテンプレートから決め、
固定回数の処理をコンパイル時に展開する。テンプレートによる抽象化が、そのまま実行時の負担になるわけではない。

一方、prefixの差から区間の答えを求める方法は、逆演算のないminや任意のモノイドには適用できない。
SIMDの加算・XOR・minも異なる命令であり、一般の構造体やユーザー定義演算を同じlaneへ自動的に詰めることはできない。
今回は型と演算の条件を絞り、短いクラスの内部でコンパイル時の選択を使う。
RMQのsigned/unsigned比較や、bitsetのOR/XOR/ANDは同じ処理を共有し、関数ポインタや型消去を挟まない。

## 区間和: 公開契約を保った短い変更

403014の左右のprefix経路を並行して読む構成と、固定回数の展開を参考にした。
既存のinclusive-prefix配置、`get` / `all_prod`のO(1)、動的サイズ、uint64全域、実行時CPU判定は維持する。
上位段の寄与が0になる場合はmaskで消し、問い合わせ中の分岐を減らす。同じ葉ブロックでは直接差を取る。
更新では固定4回のvector処理を展開する。大きな共通フレームワークや固定最大長は追加しない。

maskが0になる場合もload自体は行う。上位段のoffsetは16以上なので、添字`offset + 0 - 1`も
直前の段の確保済みセルを指す。maskによって範囲外loadを安全にする実装ではない。
演算は従来通り2⁶⁴を法とし、合流した左右の経路は同じ値の加減算で相殺される。

## 静的RMQ

`blueberry/extra-fast/static-rmq.hpp`の`StaticRangeMin<T>`は、int32/uint32の最小値専用である。
16要素ごとのprefix/suffixとブロック上のSparse Tableを持ち、同じブロック内は短い走査かSIMD minを使う。
構築時のprefix/suffixもSIMDで計算し、公開の`prod`をGCC/Clangでは明示的にinline展開する。
測定用アダプタの関数呼び出しが残る場合があったため、実APIを直接呼ぶ比較と機械語で確認した。
任意の比較器・左端argmin・厳密な線形前処理を持つ既存LinearRMQとは契約を分ける。

## bitsetと手書きアセンブラ

`blueberry/extra-fast/bitset.hpp`の`Bitset`は、動的長のビット集合を保持する。
部分和DPなどの`B |= B << k`を、上位から走査する`shift_or(k)`として融合し、一時配列を作らない。
OR/XOR/ANDも4ワードずつ処理する。末尾の余剰bitは常に0に保ち、0bit・任意のshift・自己代入にも対応する。

[Matrix Product (Mod 2)の上位10提出](measurements/extra-fast-expansion/research-matrix-product-mod2.json)も調査した。
[246187](https://judge.yosupo.jp/submission/246187)と[406212](https://judge.yosupo.jp/submission/406212)は、
`vpxor`を使うコードを入力に応じて組み替えて実行するJITを持つ。固定の大きな行列向けのブロック処理と、
実行可能メモリを使う方式である。今回のbitsetは通常のSIMD XORを独立実装し、JITや固定次元は導入しない。
上位提出全体と同じ行列積アルゴリズムを実装したという意味ではない。

全高速版で`BLUEBERRY_EXTRA_FAST_USE_ASM`による手書き命令を比較できる。
使う命令は区間和の`vpaddq`、RMQの`vpminsd` / `vpminud`、bitsetの`vpor` / `vpxor` / `vpand`。
既定はintrinsicsであり、手書きasmの存在だけを高速化の根拠にはしない。
AVX2非対応時や明示指定時は通常のループへ切り替える。

第三者ソースのライセンスが不明な部分は転載しない。調査元のURL・hashと採用したアルゴリズムを記録し、
測定用の元ソースはgit管理外に保持する。
