---
title: Static RMQ以外のFastest提出から見える改善点
---

# Static RMQ以外のFastest提出から見える改善点

2026-09-19にLibrary Checker公開REST APIから4問題について、AC・実行時間昇順・投稿者重複除外で上位3提出ずつを取得した。比較対象のBlueberry revisionは`2f36f54a1f814c21c2603d9a9a01b531998e62ef`。以下はソースを読んで分かった設計差であり、各工夫の速度への寄与は未測定である。**提出の最大ケース時間と、ローカルverifyの全ケース合計を割って性能差としない。** この調査ではCPUベンチマークを実行していない。

改善対象は、既存APIの型・演算・入力範囲・問い合わせ順序・失敗時の動作を維持できる実装とする。全クエリ先読み、可換演算限定、固定上限などは最速提出との条件差を理解する資料として扱い、今回の高速化の優先項目には含めない。

| 問題 | 上位3提出の最大ケース時間 | 現行APIと比較するときの注意 |
| --- | --- | --- |
| Range Kth Smallest | [403008: 20 ms](https://judge.yosupo.jp/submission/403008)、[393928: 35 ms](https://judge.yosupo.jp/submission/393928)、[376681: 35 ms](https://judge.yosupo.jp/submission/376681) | 3提出とも全クエリを先読みしてレベル単位に処理。1件ずつ答えるWaveletMatrix APIとは処理条件が異なる |
| Predecessor Problem | [403023: 16 ms](https://judge.yosupo.jp/submission/403023)、[278227: 20 ms](https://judge.yosupo.jp/submission/278227)、[269275: 20 ms](https://judge.yosupo.jp/submission/269275) | 64分木というアルゴリズムはFastSetと同系統。初期化・整数I/O・階層伝播の違いを分離する |
| Dynamic Tree Vertex Add Path Sum | [270626: 174 ms](https://judge.yosupo.jp/submission/270626)、[332878: 182 ms](https://judge.yosupo.jp/submission/332878)、[284283: 187 ms](https://judge.yosupo.jp/submission/284283) | 加算の可換性と有効なlink/cut入力を利用。非可換積・失敗検出付きAPIのコストを含まない |
| Static Range LIS Query | [331206: 164 ms](https://judge.yosupo.jp/submission/331206)、[393940: 201 ms](https://judge.yosupo.jp/submission/393940)、[382253: 209 ms](https://judge.yosupo.jp/submission/382253) | 上位2件は右端で並べるオフライン処理。現行は構築後に任意順のクエリを受け付ける |

取得条件・時間・メモリ・言語ID・ソースhashは[調査記録](https://github.com/blueberry1001/Blueberry-library/tree/main/docs/development/measurements/fastest-study/other)に保存した。他者ソースは配布せず、上の提出URLを参照する。投稿者重複除外のため、表の2・3件目が通常のFastest表示の2・3位とは限らない。保存されたsource profileは単語検索の手掛かりであり、mmap入力をfast I/Oとして検出できないなどの偽陰性・未使用コードによる偽陽性がある。以下では全提出を同じ深さで分析したとはせず、具体的に読んだ部分と対応する提出を示す。

## Wavelet Matrix: 圧縮とrankの配置を改善する

[403008](https://judge.yosupo.jp/submission/403008)は各ビット段の索引を作り、全クエリをその段で進め、索引領域を再利用する。永続的なWavelet Matrixを全段保持しない。左右端・順位・答えを別々の連続配列に置き、AVX2のgatherと比較によって複数クエリを進める。値域が小さければオフセットだけで符号化し、疎な値域では値と元の位置をradix sortする。段ごとのprefix表現もクエリ数などに応じて選択している。

[393928](https://judge.yosupo.jp/submission/393928)のmainは構築済み行列に`kth_batch`を呼び、同じ段を全クエリで共有している。[376681](https://judge.yosupo.jp/submission/376681)も同様で、rank用の64bitワードと32bit累積値を同じ構造体に入れ、レベル内のアクセスを近づける。後者はpopcnt指定、2桁単位の入力変換、4桁出力表、mmapも使う。

現行`wavelet-matrix.hpp`は圧縮のためにsortした辞書へ各要素のlower_boundを行い、各段でビット列・累積値を別vectorに持ち、1クエリが全段を降りる。改善候補は次の順序になる。

1. 値と元位置を一度sortしてrankを直接書き戻し、N回のlower_boundを除く。符号付き型・重複・最大値の順序を維持する。
2. bitsとprefixの配置を比較する。同居させるとアクセス回数を減らせる可能性がある一方、paddingで容量が増えるため、必ずNと値域を振る。
3. 構築時の作業領域や段ごとの走査を見直す。get・count・range_freq・kth・前後値検索を含む全操作の意味と計算量を維持する。

batchやAVX2による複数クエリ処理は、表の20–35 msと通常APIの条件差を説明するための事実である。前の答えが次のクエリを決める用途にも対応する現行APIの改善計画には、その処理方式への変更を含めない。

## FastSet: 同じ64分木でも不要な上位書き込みを減らせる

[403023](https://judge.yosupo.jp/submission/403023)は全階層を単一vectorに格納し、offsetで各段を参照する。insertでは葉ワードが空から非空になった場合だけ上へ進み、既に非空の親で止める。探索は同じ葉ワード内を先に処理し、上位段の昇降をテンプレートで展開する。要素数16以下では、更新で無効化する小さいsorted cacheを使い、AVX2比較で順位を求める。この特殊化は疎な集合向けであり、更新直後のcache再構築も費用に含める必要がある。

[269275](https://judge.yosupo.jp/submission/269275)は上限1e7専用の固定4階層と固定配列を使う。二進文字列から葉を作る処理、非空ワードから上位を作る処理をAVX2でまとめて行う。mmap入力、8byte単位の整数変換、4桁出力表も含む。

現行`fast-set.hpp`は動的な階層数とvectorの配列を持ち、insert時にcontainsを調べた後、全階層を書き込む。**最初に検討するのは空→非空の遷移時だけ上位へ伝播する変更**であり、APIを増やさず改善できる。次に連続配置、葉内探索、初期文字列のワード単位変換を個別に比較する。動的なuniverse、size、重複更新、境界を含むnext/prevの意味を維持する。固定上限や小集合専用方式は採用項目に含めず、上位提出の条件差として記録する。

## Link-Cut Tree: 演算回数とaccess回数を数える

[270626](https://judge.yosupo.jp/submission/270626)は添字で参照する固定Node配列にsumを1つだけ保持し、補助木内の子方向を小さい整数で記録する。rotateは下がった親の集約を更新し、accessの終わりで最終集約を更新する。点加算はsplayした点の値とsumへ直接加算し、根までのaccessをしない。入出力はmmapと複数桁変換で、データ構造部分にSIMD演算は見当たらない。自動profileのSIMDフラグだけでSIMD木と判断してはいけない。[284283](https://judge.yosupo.jp/submission/284283)はポインタ版の固定Node配列とsumを使い、点更新ではexposeを実行する。上位でも実装の選択は一つではない。

現行`link-cut-tree.hpp`は任意の非可換opのためforward/backwardを両方保持する。rotateごとに親と子の両方をpullし、splay前には祖先をstackへ集めてpushする。setはaccessする。linkは失敗を返す契約のため接続確認を行う。また`prod`の接続確認はassert内なので、NDEBUGの有無で実処理量が変わる。単に提出の加算専用コードへ置換すると非可換性や失敗検出の契約を失う。

改善の検証順は、release/debugを分けたaccess・rotation・op回数の計数、同じ不変条件を保つpull削減、任意の非可換opでも正しい点更新時のaccess削減とする。forward/backwardと失敗検出の契約は維持し、可換専用化や有効入力限定化は採用項目に含めない。splayやlazy伝播の変更は非可換path composite、反転連鎖、失敗するlink/cutも含むrandom比較が必須。加算専用最速提出との速度差を「テンプレートが遅い」とは結論しない。

## Static Range LIS: 最速の処理条件を通常APIへ混ぜない

[331206](https://judge.yosupo.jp/submission/331206)は全クエリを右端でbucket順に並べ、右端を一度だけ進める。値域をsqrt個のブロックに分け、heapによる遅延処理とFenwick Treeで答える。固定長配列・fread/fwriteのバッファを使う。[393940](https://judge.yosupo.jp/submission/393940)も同系統のbumping処理で、ブロックの最大heap・遅延置換heap、右端のCSR bucket、Fenwickを使う。いずれも公式入力の順列という条件と、全クエリが既知という条件を利用する。

[382253](https://judge.yosupo.jp/submission/382253)は現行に近いseaweed/MongeとWaveletの構成だが、作業配列を先に確保し、反復doublingで使い回す。ソース内のO(N log N)というコメントをそのまま採用すべきではない。外側doublingの各マージで内側doublingを行う構造があり、この調査ではより良い計算量の証明として扱わない。また17bit固定の値域前提がある。

現行`static-range-lis.hpp`はMonge積の再帰ごとに複数vectorを作り、再帰的な部分列・位置対応表を構築する。最初の候補はscratch buffer再利用と部分配列のview化で、次に既に整数順位になったdiagramをWavelet側で再圧縮しない内部経路である。重複値を持つ一般配列のstrict LIS、空区間、任意順問い合わせは維持する。右端順の別方式や17bit固定は最速提出の条件差として扱い、今回の実装方針には含めない。

## 横断して守る計測順序

これら4問題では高速I/Oを含む提出が多く、通常の非同期iostreamだけと比べたend-to-end差を全てデータ構造へ帰属できない。全体時間、I/Oのみ、構築、操作を分け、同一入力でI/Oだけ・構造だけを入れ替える。unsigned専用、入力桁数の上限、ファイル入力、AVX2、固定配列、全クエリ先読みをそれぞれ独立した条件として記録する。

採用順序は、**全API保証を保つ不要処理の除去 → メモリ配置とscratch再利用 → 同じ意味を保つ構築・探索の改善**とする。利用側への追加の前提や使い分けを増やさずに改善する。CPU依存命令は全対応環境で同じAPIを維持できる場合だけ後段で検討する。mmap専用や未整列読み込みのコードをそのまま共通I/Oに入れず、通常ファイル・pipe・EOF境界・符号・overflowを扱うportable経路を確保する。Fastestの数値は調査の入口であり、改善率は同一環境の再測定から報告する。
