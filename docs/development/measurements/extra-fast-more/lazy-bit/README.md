# 遅延更新・BITの調査

2026-10-20にLibrary Checker REST APIからrange_affine_range_sumとpoint_add_range_sumのFastest各5件を取得しました。提出時の表示時間は実行環境が異なるため、ローカル測定との倍率には使いません。

- [affine 402323](https://judge.yosupo.jp/submission/402323)は16分木、32bit MontgomeryのAVX2 8lane演算、sum/multiplier/translationのSoA、係数の事前準備を使います。高さと最大要素数500000はコンパイル時に固定し、全操作を先読みして4件先をprefetchします。独自SIMD整数入力と出力も含み、全体の速度差をtreeだけの差と解釈できません。
- [affine 408770](https://judge.yosupo.jp/submission/408770)もwide lazy/Montgomery/専用I/Oを含みます。全体を392行に圧縮していますが約70KBあり、行数だけでは短さを評価できません。
- [sum 258491](https://judge.yosupo.jp/submission/258491)と[403014](https://judge.yosupo.jp/submission/403014)の16分prefix構造は既存SumSegmentTreeの調査でも扱っています。新しいBITは同じSIMD木を重ねず、省メモリとprefix探索を目的にします。

取得したソースはライセンスが確認できないため配布しません。取得時の順位、表示値、source hashはresearch-affine.json/research-bit.jsonに保存しています。第三者ソースはignoredな.benchmark内だけに置き、実装はアルゴリズムから独立に記述しています。

## 候補

- FenwickTree: uint64_tのmod 2^64加算。添字iをi+(i>>10)に写すpadding、O(N)構築、Fenwickの部分木からgetを直接復元、lower_boundをO(logN)で行います。prefix探索だけは非負・総和非overflowが必要です。BITの依存したロードを手書きasmへ変えるだけではSIMD化できないので、無理なasmは入れません。
- RangeAddSum: 差分dとi*dを2本の既存SIMD prefix木で保持します。区間加算を差分の端点更新に変え、prefix(r)=r*sum(d[0:r])-sum(i*d[i],i<r)を使います。遅延伝播のない加算専用構造です。wrapper約90行に加えて、依存するSumSegmentTree約177行も必要です。
- affine SoA: 998244353固定の実際の遅延セグ木候補です。sum/mul/addを別配列にし、区間長は階層から求めます。まず二分木によるscalar候補とACLを同じ入力で比較し、複雑なSIMD実装を増やす根拠があるか判断します。

## pilot測定

[pilot](pilot/summary.json)は624標本、各候補3反復です。GCC 13.3 / Clang 18.1、gnu++20、assert有効-O2とnative -O3 -DNDEBUGを分けました。N=65536のrandom/short/update90とN=500000 random、Q=100000、固定seed 20261020です。CPU 0に固定し、担当間で重い処理の時間を分けています。OSや他のbackground負荷は制御していません。

BITはpadding9/10/なし、ACL、既存wide木とscalar経路、区間加算は2本の各BIT/wide木とACL lazy、affineは二分SoAとACLを比較しました。全groupでquery checksumは一致しました。ただしaffine SoA試作はpublic引数assertをまだ省いているため、assert profileの差を公開実装の採用値にはしません。

## 最終測定（区間加算・BIT）

[final](final/summary.json)は840標本、5反復です。GCC/Clangそれぞれでassert -O2、portable -O2 -DNDEBUG、native -O3 -DNDEBUGを分けました。pilotとは異なるPython生成入力で、独立Pythonの1本/2本Fenwick oracleから全出力のSHA256を作り、全840標本が一致しています。

同一FastInput/FastOutputでparse/build/operations/formatを分け、入出力込みのprocess wallとLinux VmHWM（/usr/bin/timeの%M）も保存しました。VmHWMは入力・query・出力bufferと構築時の一時領域を含み、header単体の保持量とは異なります。parseとformatを除外した値を全体の倍率として扱いません。

N=500000、Q=100000、random、portable -O2 -DNDEBUGの中央値です。単位はms、RSSだけKiBです。

|compiler|candidate|build|operations|parse〜format全体|RSS|
|---|---|---:|---:|---:|---:|
|g++|bit-acl|3.700|2.346|20.331|14644|
|g++|bit-pad|1.268|2.199|18.882|14624|
|g++|bit-wide|1.063|1.149|17.142|14872|
|g++|range-acl|7.036|24.238|47.205|38696|
|g++|range-wide|3.791|4.492|23.279|22688|
|g++|range-pad|4.374|5.384|24.721|22140|
|g++|range-scalar|4.620|12.580|31.635|22648|
|clang++|bit-acl|4.260|2.581|26.914|14644|
|clang++|bit-pad|1.582|2.300|24.572|14636|
|clang++|bit-wide|1.040|1.331|23.407|14904|
|clang++|range-acl|7.522|25.776|54.420|38812|
|clang++|range-wide|4.020|4.467|29.248|22576|
|clang++|range-pad|4.473|5.438|30.046|22056|
|clang++|range-scalar|4.174|11.141|35.837|22632|

区間加算の操作ではGCCで約5.4倍、Clangで約5.8倍でした。入出力と構築を含む全体は約2.0倍、約1.9倍です。uint64_tの加算作用だけに限定する差分恒等式とSIMD prefix木の効果で、汎用lazyの代替という倍率ではありません。2本のpadded BITも比較し、update90のN=65536ではこちらがwide木より速い場合がありました。

新BITの同条件の操作時間短縮は約6%（GCC）/11%（Clang）で、既存wide木の方が速いです。N=65536 shortではportable Clangが1.944→2.052msへ悪化しています。prefix探索と約0.1%のpaddingで済む保持量が目的で、どの入力でもACLより高速とはしません。既存wide木の保持量も約1.067N語なので、省メモリ差は約6%です。

- 通常のBITはN語、新BITはN+floor(N/1024)+1語、wide木は各段の16語paddingを除く漸近値で16N/15語です。
- RangeAddSumはwide木2本で約32N/15語、vectorからの構築では一時差分vectorも必要です。
- scalar fallbackも全入力で検証しました。非AVX2で同じ改善率になるとは限りません。

## affineの残る差

二分SoA試作は短くできますが、N=500000/Q=100000 randomのoperationsはGCC assert -O2で47.83→42.19ms、Clangで56.21→37.86msでした。SIMD・wide化を行わずに済ませる候補としては公開を保留しました。

[affine-whole](affine-whole/summary.json)では上位[402323](https://judge.yosupo.jp/submission/402323)原本を変更せず、両者をgnu++23 -O2 -DNDEBUG -march=nativeで比較しました。原本はportable指定ではAVX2必須のcompile errorとなり、その失敗ログも保存しています。nativeではGCC/Clangともcompileと全44実行に成功しました。N=37/Q=200では独立bruteを照合し、N=500000 randomとN=65536 shortは各5反復で全出力が一致しました。

randomの全体中央値はSoA/上位原本がGCC 73.69/27.99ms、Clang 59.96/23.87msで、約2.5〜2.6倍の差が残ります。入出力、固定最大数、先読み、SIMD Montgomeryを含む差であり、tree単体の差ではありません。この差を受けて、動的8/16分木とAVX2 Montgomeryを独立実装しました。`wide-pilot-1`の624標本では16分木が概ね良く、公開APIは`AffineSumSegmentTree`としました。通常剰余のsum、Montgomeryのlazy tag、leafのlazy省略を組み合わせ、350行で実装しています。`wide-validation-1`のGCC/Clang・ASAN/UBSAN計12コマンドはすべて成功しました。

`wide-reference-core-1`は同じFastIO・query bufferへ上位402323のcoreを接続した診断比較です。gnu++23 -O2 -DNDEBUG -march=native、5反復で、N=500000 randomの操作中央値はACL/動的16分木/上位coreがGCC 49.46/22.56/10.30ms、Clang 49.50/17.28/9.25msでした。固定最大長などの差は残りますが、入出力だけでは説明できない差があるため、動的木の階層ごとの可変除算を次の改善対象としました。

## affineの可変除算除去

`shift-validation-1`と`shift-pilot-1`では、16分木の階層幅が16の冪であることを使い、runtime除算・剰余をshift/maskへ置換しました。最大NがINT_MAXでもshiftは28以下で、中間のceil計算にはuint64_tを使います。公開契約や動的サイズの範囲は同一で、階層幅の配列も不要になり、コードは1行短くなりました。

GCC/ClangのbruteとGCC ASAN/UBSANの6コマンドはすべて成功し、3反復の312標本も全出力が一致しました。測定枠は2026-10-20 06:48:50〜06:50:07 UTCで、他担当のCPU処理は停止、OS負荷は未制御です。`shift-assembly`の同じapply/prod probeでは可変除算命令がGCCで6→0、Clangで12→0でした。この数は静的な命令数で、1操作あたりの実行回数ではありません。

portable -O2 -DNDEBUGの操作時間中央値は次の通りです（ms）。

| compiler | 入力 | 除算版 | shift版 |
| --- | --- | ---: | ---: |
| GCC | N=500000 random | 23.12 | 20.40 |
| Clang | N=500000 random | 22.40 | 21.15 |
| GCC | N=65536 short | 7.29 | 5.11 |
| Clang | N=65536 short | 7.04 | 5.37 |

実装が短くなり、shortでも改善したためshift版を採用しました。公開ヘッダを直接使う最終5反復と全backend検証は、この候補測定とは別に記録します。第三者原本のfixed-height coreとの差をすべて埋めたという結果ではありません。
## affineの16lane kernel集約

最後の1候補として、16laneのfold/transformを1回のtarget AVX2関数へまとめ、係数broadcastを共有しました。`packed-validation-1`のGCC/Clang bruteとGCC ASAN/UBSAN計6コマンド、`packed-pilot-1`の234標本はすべて成功しました。3反復の大入力24条件中21条件で操作時間が改善し、2行短くなるため採用しました。

portable -O2 -DNDEBUGの操作時間中央値はN=500000 randomでGCC 19.71→18.86ms、Clang18.54→17.22ms、N=65536 shortでGCC4.96→4.57ms、Clang4.88→4.77msです。3条件の回帰もsummaryへ残しています。公開ヘッダのfreeze SHA256は`816947aa7f352111bb5c28df42e4328828a332576fa43202e9532fc2e3d47f62`です。
## 公開affine版の最終測定

`affine-final-1`は公開ヘッダを直接includeする独立driverで、GCC/Clang × assert -O2 / portable -O2 -DNDEBUG / native -O3 -DNDEBUG -march=native / portable GNU asmを比較しました。大入力は各5反復、小入力は独立bruteを1回照合し、504標本すべて成功しました。各値は同じparse・query buffer・output処理を通し、構築/操作/parse/format/全体/process wall/RSSを別に保存しています。

portable -O2 -DNDEBUG、N=500000/Q=100000 randomの中央値です（時間ms、RSS KiB）。

| compiler | 実装 | 構築 | 操作 | parse〜format全体 | RSS |
| --- | --- | ---: | ---: | ---: | ---: |
| GCC | ACL | 3.97 | 52.62 | 70.46 | 23468 |
| GCC | 公開SIMD | 1.08 | 22.30 | 34.77 | 9984 |
| GCC | 公開scalar | 1.12 | 35.13 | 47.86 | 9932 |
| Clang | ACL | 6.70 | 54.09 | 73.01 | 23424 |
| Clang | 公開SIMD | 0.92 | 17.79 | 30.92 | 10000 |
| Clang | 公開scalar | 0.88 | 33.82 | 47.78 | 9928 |

同じaffine/sum操作で、ACL比の速さは操作が約2.4倍/3.0倍、parse〜format全体が約2.0倍/2.4倍でした。固定modulus・作用と、暗黙の区間長・wideなSIMD配置を利用した結果で、任意monoid/作用の改善率ではありません。RSSは入力・query・回答vectorやACL adapter構築時の一時vectorも含み、tree単体ではありません。

GNU asm profileのN=500000 randomはSIMD操作がGCC19.19ms/Clang18.08msでした。intrinsics profileよりGCCは短くClangは長く、profile間の負荷変動もあるためasmの普遍的な優位性は主張しません。既定はintrinsicsです。

`affine-public-whole-1`は実際の公開verifierと原本402323を同じgnu++23 -O2 -DNDEBUG -march=nativeで新しくcompileし、3入力×GCC/Clang×両実装を各5回、計60標本実行しました。全出力が一致し、small37は独立bruteでも確認しました。旧SoA版・旧div版の結果は混ぜていません。

| 入力 | compiler | 公開verifier全体ms | 上位原本全体ms |
| --- | --- | ---: | ---: |
| N=500000 random | GCC | 53.28 | 46.57 |
| N=500000 random | Clang | 64.02 | 28.33 |
| N=65536 short | GCC | 21.88 | 21.82 |
| N=65536 short | Clang | 21.54 | 14.60 |

原本には固定最大数・固定高さ・全query先読み・prefetch・専用I/Oがあり、公開版と同じAPIの比較ではありません。またprocess wallは起動・OS schedulingを含み、短い実行ほどばらつきがあります。上位に完全には到達していませんが、既知の可変除算とkernel境界を評価して除き、動的サイズとconst queryを保った短い実装として採用しました。測定枠の開始/終了は各environment.json、他担当のCPU処理停止とOS負荷未制御も同ファイルへ記録しています。
## 検証ログ

[validation/commands.json](validation/commands.json)にはGCC/Clang、gnu++20/23、intrinsic/scalar/asm/Intel dialectの2random test、GCC ASAN/UBSAN、verify compileの70 commandを記録しています。再実行はすべて成功しました。初回Clangはテストの明示自己代入copy=copyを-Werrorで拒否し、alias経由に直しました。初回失敗とその時のテストをvalidation/initialへ残しています。

`final-validation/commands.json`では公開3headerに対し、GCC/Clang、gnu++20/23、intrinsics/scalar/GNU asm/Intel dialect、release、GCC ASAN/UBSAN、verify3本compileの129コマンドがすべて成功しました。公開版affineの移動元空状態・copy/move・任意uint32_t値・境界と更新合成も含みます。

Fenwick/RangeAddSumの文書の最小mainは直接compile/runしました。catalog登録前のcheck_docs.pyが「Register every modern header」で停止した初回ログも残しています。統合後のstructure-onlyは123 library docsで成功しました。公式問題実行と全体のmake check/verify/docsはroot担当が行い、この担当のverify compile成功とは区別します。

## FastIOの測定条件

このfolderの全性能測定はbase `617acd6`、FastIO SHA256 `b8b9c274f93c3906b8f6fc26dd945ec0679b3c246685da3108789aa677d73b30`の条件で完了しました。終了後に既存PR55のFastIO改善がmainへ入り、統合後のFastIOは異なります。[旧FastIOのsnapshot](../fast-io-before-pr55.hpp.snapshot)も保持しています。最新main `a54fa8c`のFastIO SHA256は`8f08950cfc12f2babdd0eaa6ed0b38ce90e999ce7e24ee5105cdbb4cfc7ad406`です。新mainのI/O条件で再測定した数値ではなく、以前の共通I/Oでの比較として保持します。最新I/Oで行う公式verifyの時間と同一測定としてまとめません。

## 再実行

記録したheaderは各folderの*.hpp.snapshot、実行したharnessとrunnerはこのfolder直下の*.snapshotです。第三者原本の取得用APIとhashはresearch-*.json、compiler/flags/CPU/seed/入力・出力hashは各environment/samplesにあります。

runnerは測定を残すfolderへ追記するため、再実行時は出力先を新しいfolderへ変更してから使ってください。既存測定を上書き・混合しないでください。*.cpp.snapshotを元の.cpp名へ、*.py.snapshotを元の.py名へ戻してignoredな.benchmark/extra-fast-more/lazy-bitへ配置し、リポジトリrootからpython3で実行します。ACLは.deps/ac-library、公開headerはsnapshotと同じrevisionを使います。
