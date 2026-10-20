---
layout: page
title: FenwickTreeのSIMD化
---

# FenwickTreeのSIMD化

2026-10-20、実験版FenwickTreeの内部を16分累積和木へ変更しました。既存のSumSegmentTreeが持つAVX2のsuffix加算カーネルを再利用し、prefix探索も16個のuint64_tを並列比較して木を降ります。更新カーネルにはGNU asmの選択肢があり、非対応CPUにはscalar経路があります。APIとmod 2^64の演算規則は維持します。

一点取得と全体和はO(1)、更新・区間和・prefix探索はO(log(N+1))です。構築はO(N+1)、主な保持量は約16N/15語です。空入力と末尾ノードのpaddingを含む正確な保持量は[API文書]({{ '/blueberry/extra-fast/fenwick-tree.hpp.html' | relative_url }})に記載しています。

ユーザーの指定により、ローカル検証は既存の境界・ランダム比較を固定seed 1で実行する最小限の範囲に絞り、速度測定は行いません。従来版の改善率を今回の実装の結果として扱いません。全体のmake check / make verify / make docsもローカルでは再実行しません。公開前のrequired CIは通常の設定を維持します。

GCCとClangのgnu++20、scalar強制、GNU asmのIntel記法の4構成で、固定seed 1の境界・ランダム比較が成功しました。フラグは`-O2 -Wall -Wextra -Wshadow -Werror`で、asm構成のみ`-DNDEBUG -masm=intel`を併用しています。UINT64_MAX付近のprefix探索、16と256の境界も確認済みです。公式ドライバのコンパイルと小さい入力の出力照合も成功し、生成命令には更新のvpaddqと探索のvpcmpgtqがありました。文書の構造検査と独立したコードレビューでも問題は見つかっていません。
