---
title: "整数キーのset/map"
---

# 整数キーのset/map

## 調査と契約

2026-10-20にLibrary CheckerのFastest各5件を調べた。取得順位・表示値・ソースのhashは[ordered_set](research-ordered_set.json)、[associative_array](research-associative_array.json)、[predecessor_problem](research-predecessor_problem.json)に保存する。表示時間は環境の異なるローカル測定との倍率に使わない。第三者原本はignoredな`.benchmark`にだけ保持し、配布しない。APIから返ったUTF-8の原本byteに対応するhashは`source_bytes_sha256`欄で、改行を変換したファイルと混同しない。

- [ordered_set 409360](https://judge.yosupo.jp/submission/409360)と[409538](https://judge.yosupo.jp/submission/409538)は30bit固定の上位directory・部分和木と、下位12bitの適応bucketを使う。bucketは少数キーのpacked表現、32要素のSIMD配列、4096bitのbitmapを切り替える。sortedな初期集合のbulk構築と8件先のquery prefetchも組み合わせる。単純な汎用B-treeだけの比較ではない。
- [410556](https://judge.yosupo.jp/submission/410556)などは全queryを読んだ後の座標圧縮を使う。LCの入出力全体では比較できるが、未知のキーを追加するオンラインAPIの計算時間とは区別する。
- [associative_array 362188](https://judge.yosupo.jp/submission/362188)は固定容量・乗算hash・線形探索・使用bit列を使う。表本体をasmでSIMD化したものではなく、I/Oの最適化も大きい。削除・動的成長は提供しない。
- predecessorの上位群はbounded universeの多段bitmapで、通常版FastSetと同じ方向である。整数集合だからという理由だけで同等品を増やさない。

[Abseil B-tree](https://abseil.io/about/design/btree)の多分木によるcache localityと、[Swiss Table](https://abseil.io/about/design/swisstables)のタグ同時比較を設計資料として調査した。いずれも実装コードを転載せず、本ライブラリ向けに独立に記述する。

## 実装と契約

OrderedMap/Setは平衡B+木を使う。キーは葉に持ち、内側のノードには子の最大キーと部分木サイズを保持する。ノードの分割・borrow・mergeで平衡を保つ。順位・k番目・inclusive floor・lower/upper_boundに対応し、整数の全範囲を保存できる。これらの値検索はoptionalの所有コピーを返す。Valueはtrivialなコピーを持つ型に限定する。

ユーザーの追加指定に合わせ、begin/end、逆反復子、find、erase(iterator)を追加した。葉の前後リンクを使う双方向反復子で、Mapは実際のpair<const Key,Value>への参照、Setはconst Key参照を返す。Mapの通常のrange-forで値を更新できる。Mapでは連続SIMDキー列とpair内のキーが重複するため、反復子追加前よりノード保持量が増える。Setはmarker型で従来の短い葉領域を維持し、キーを重複保存しない。挿入・消去・代入などで既存反復子が失効するため、std::map/setの完全互換ではない。

HashMap/Setは16個の7bitタグをまとめて比較する。未使用と削除済みのタグを区別し、三角数のprobeでpower-of-two個のgroupを走査する。負荷率7/8を上限とし、成長やtombstoneの再配置は一時表を完成させてからswapする。予約キーを使わず、削除と動的容量を提供する。固定hashが十分分散する場合の平均計算量と、最悪線形探索・最悪二乗再配置をAPIページで区別する。

Map/SetのSIMDは型ごとの比較だけに閉じ込める。OrderedはAVX2の実行時判定、HashはSSE2対応コンパイル環境の判定を持ち、明示scalarも使える。既定はintrinsicsで、GNU asm版は同じ比較命令の別表記である。asmを指定するだけで速くなるとはしない。

## 初期pilot

[pilot](pilot/summary.json)はGCC13、gnu++20、portable `-O2 -DNDEBUG`、N=50000、Q=100000、64bit乱数キー、3回の測定。共通の検索・代入・削除と、順序付き版のlower_boundを標準コンテナと照合した。全36標本の出力digest・出力数・最終sizeが一致した。反復子追加前の候補であり、最終版の性能とは区別する。

| 種類 | 標準 構築ms | 新版 構築ms | 標準 操作ms | 新版 操作ms |
| --- | ---: | ---: | ---: | ---: |
| ordered map | 8.733 | 7.827 | 16.574 | 10.322 |
| ordered set | 5.666 | 7.650 | 14.050 | 9.463 |
| hash map | 1.891 | 2.184 | 2.747 | 2.286 |
| hash set | 2.101 | 1.207 | 3.165 | 1.899 |

unordered_map/setは同じN+Qをreserveしてから構築した。ordered setとhash mapの構築は標準の方が速く、すべての場面で代替できるという結果ではない。順序の必要な処理でhashの速度を代替値にしない。

## 候補比較と採否

[iterator-candidates](iterator-candidates/summary.json)は反復子を追加した後の7構成、GCC/Clang、32/64bitキー、4分布、各5反復を比較した。28 buildと1280実行に成功し、独立oracleとのdigest・出力数・最終sizeが全て一致した。候補順は構成ごとで、全構成を毎反復rotateしてはいない。CPU0固定でも他スレッドの文書生成やOSの負荷は統制できず、数%の差を確定した改善率として扱わない。

| 候補 | 採否 | 結果と理由 |
| --- | --- | --- |
| SIMD用キー領域24個→B+1個 | 採用 | 余分なpaddingを削減。探索時は分割済みでn<=Bのため最後のloadも配列内。速度は概ね同等で、ノード保持量を減らせる |
| boundが葉の前後リンクを使う | 採用 | 反復子が維持するリンクを再利用し、祖先から降り直す処理を削減。コードも短くなる。速度は分布・compilerにより増減 |
| B=32 | 棄却 | Clangの64bit乱数では改善したが、他ケースで約33%の悪化。小さいB=16を維持 |
| 問い合わせ全体でAVX2 dispatch | 棄却 | GCCの構成別比の中央値は約8%悪化、最大約30%悪化。常に速くならずコードも増える |
| hash検索時の削除タグ調査を省略 | 採用 | 挿入だけに必要なtombstone候補の収集をtemplateで分離。操作比の中央値はGCC約0.94、Clang約0.95。ただし回帰ケースもあり一律の改善率ではない |
| hashのキー・値をAoS化 | 棄却 | GCCでは改善する場合もあるが、Clangの操作比の中央値は約1.07。32bitキーのpaddingとSetの保持量も増える |

比較の基準は各compiler/型/分布のselected構成で、上記の比は候補時間/基準時間。Mapではpairの実参照を保持する反復子契約を全ordered候補で維持した。最終比較は採用した組合せを公開headerへ反映し、format後のsourceから計り直す。

[before-iterators](before-iterators/failures.json)の初回候補比較は、AoS試作でHashSetが二重定義され4 buildが失敗した。成功した960実行は照合一致したが、反復子追加前かつ一部build失敗のため最終採否の根拠に使わない。重複定義を除いた再測定が上記1280実行である。

[initial-validation](initial-validation/commands.json)のClang失敗8件は、テスト用trivial型の暗黙コピー代入がdeprecated-copy警告になったもの。明示defaultコピー代入を追加し、[iterator-smoke](iterator-smoke/results.json)でGCC/Clangの両テストとGCC ASan/UBSan、全10コマンドが成功した。各失敗時のsourceとログも保持する。最終版の完全matrixは別に記録する。

### 返却時の型契約の修正

追加の静的レビューで、trivialなconstコピーと例外を送出するforwarding constructorを併せ持つValueについて、`kth`とboundsのpair一時オブジェクトからoptionalへの移動が、意図しないconstructorを呼び得ることが分かった。[optional-return-fix](optional-return-fix/before.json)では公開の3つの型制約を満たすValueを使い、GCCで4操作すべての例外43を再現した。Clangでは同じ例が正常に返り、compilerによる差もそのまま残した。最初のrunnerは両compilerで再現するとの誤ったassertを含み、失敗ログとrunnerを保持する。

optional内でpairを直接構築し、Valueのconst参照からコピーする3行へ修正した。[修正後](optional-return-fix/after.json)は両compilerで4操作とも正常。100キーのkth/lower_bound/upper_bound/floorをrandom testの型回帰にも追加した。[binary比較](optional-return-fix/binary-equivalence.json)ではClangの4構成は同一だったが、GCCの4構成は変化したため、修正前の1920標本は[final-before-optional-fix](final-before-optional-fix/summary.json)へ保存し、最終phaseを全構成・全入力で計り直した。比較runnerの全binary同一assertの失敗も隠さず保存した。中断したwholeの35標本も最終集計に混ぜない。

## 最終比較の設計

候補比較では、B=16/32、キー領域の余分なpadding、AVX2関数へ問い合わせ全体をまとめる構成、hashのキー・値のAoS/SoAを分けて測る。測定していない構成を最速と呼ばない。

最終phaseはGCC/Clang、portable O2 release/native O3 release、32/64bitキー、小さいlookup中心・乱数混合・密なキー・単調な成長と削除を比較する。各5反復で標準・SIMD・scalarの順を回す。初期構築、操作、共通FastInput、共通to_chars整形を分離し、process wallも保存する。rank/kthをstd::setの線形走査で代用して倍率を作らず、これらを含む全体比較には通常OrderedSetを使う。

Python側は整数キーを全体で圧縮し、独立したFenwickの個数とdictからonline操作の期待値を計算する。問い合わせの答えはFNV digest・出力数・最終sizeで照合する。全値・符号・削除・コピーなどの正しさは別の標準コンテナ照合テストでも確認する。

RSSは子プロセスの`/proc/self/status`のVmHWMを読む。全体RSSは入力・query・出力bufferも含む。memory modeはqueryを読まず初期要素だけで構築し、入力vectorとI/O bufferを含むpeakを測る。同じN+Qのreserve方針を保つため、保持要素のpayloadだけの値ではない。

CPU0固定だがOS全体の負荷は統制しない。失敗・回帰・中断を残し、欠測を0で埋めない。

## 最終コードの検証

[validation](validation/commands.json)の88コマンドはすべて成功した。GCC/Clang、GNU++20/23、intrinsics/強制scalar/GNU asm両dialectのmixed random testを各3seed、単独・重複includeのrelease確認、3公式driverのcompile、両compilerのASan/UBSanを含む。trivial型の例外経路回帰も含めた。専用の[iterator matrix](../ordered-iterator/README.md)は初期10＋最終42コマンドがすべて成功し、実参照・const変換・順逆・split/merge・消去後の返却反復子を照合した。公式judgeの実行結果は統合検証に別記する。

## 最終phaseの結果

[final](final/summary.json)は型契約修正後の公開headerから、GCC13.3/Clang18.1、GNU++20、portable O2 release/native O3 release、32/64bit、4分布、標準/SIMD/scalar、各5回を測定した。8 buildと1920実行が成功し、全出力を独立oracleと照合した。標準・SIMD・scalarの順序を反復ごとにrotateしている。以下はportable `-O2 -DNDEBUG`の中央値msで、randomはN=50000/Q=100000、denseはN=500000/Q=100000。

| compiler | key bit | 種類 | 分布 | 標準 構築 | 新版 構築 | 標準 操作 | 新版 操作 |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |
| g++ | 32 | map | random | 7.768 | 6.607 | 14.527 | 7.292 |
| g++ | 32 | set | random | 5.725 | 5.939 | 12.422 | 6.944 |
| g++ | 64 | map | random | 7.681 | 7.558 | 14.103 | 9.292 |
| g++ | 64 | set | random | 5.757 | 6.191 | 13.556 | 8.213 |
| g++ | 64 | hash | random | 1.603 | 1.823 | 2.709 | 1.853 |
| g++ | 64 | hashset | random | 1.702 | 1.194 | 2.503 | 1.616 |
| g++ | 64 | map | dense | 220.328 | 139.301 | 64.566 | 22.305 |
| g++ | 64 | set | dense | 146.266 | 110.975 | 44.586 | 16.019 |
| clang++ | 32 | map | random | 6.406 | 6.820 | 9.962 | 7.430 |
| clang++ | 32 | set | random | 5.647 | 6.092 | 9.468 | 6.994 |
| clang++ | 64 | map | random | 6.480 | 6.762 | 9.760 | 9.712 |
| clang++ | 64 | set | random | 5.799 | 6.135 | 9.201 | 9.103 |
| clang++ | 64 | hash | random | 1.773 | 1.870 | 2.519 | 1.663 |
| clang++ | 64 | hashset | random | 1.962 | 1.162 | 2.503 | 1.527 |
| clang++ | 64 | map | dense | 186.083 | 120.943 | 37.780 | 20.125 |
| clang++ | 64 | set | dense | 134.360 | 102.776 | 29.802 | 16.609 |

Map/Setはstd::map/set、HashMap/Setはstd::unordered_map/setとの比較であり、順序契約を混ぜない。32bit乱数のordered mapはGCCで約2.0倍、Clangで約1.3倍だった。64bit乱数のClang map/setはほぼ同等で、構築が遅い場合もある。小さいlookup主体ではClangの64bit orderedやhashが標準より遅いケースもあり、全場面での置き換えは勧めない。SIMDとscalarの比較もsummaryに残した。

Mapの反復子は実pairを保持するため、キーをSIMD配列とpairの両方に置く。64bit randomのmemory modeでは、GCC std::map/newが7160/8244KiB、Clangが7156/8196KiBだった。一方64bit denseのGCCは42352/40824KiBで、ノード充填率・capacity・型によって相対量が変わる。これは入力bufferを含む初期構築だけのVmHWMを各組1回測った値であり、要素だけの保持量や5回の中央値ではない。全体process RSSの中央値もraw/summaryに別に保存する。

HashMapはreserveを含む構築・検索・削除を比較し、成長分布では予約を行わない。64bit randomでは標準より操作が速くても構築が遅い場合があり、容量やキー分布に依存する。固定splitmixの最悪ケースや非trivialな値は契約の対象として区別する。

## 全体比較

[whole/summary.json](whole/summary.json)はGCC13.3/Clang18.1、GNU++23 `-O2 -DNDEBUG -march=native`、CPU0固定、各5反復のprocess wallである。候補の順番を反復ごとに回し、binary・入力・出力は同じLinux `/tmp`に置いた。[environment.json](whole/environment.json)にcompiler版、flags、全入力・source・binaryのSHAを保存する。OrderedSetの通常版にはrank/kthを持つtreapを使い、std::setの線形走査で代用していない。新版と通常版・標準版のI/Oは同じFastInput/FastOutputで、上位提出は原本のI/Oを含む。

ordered_setはN=0/Q=20000、N=65536/Q=200000の混合、N=200000/Q=200000の検索中心、N=Q=500000の混合を測った。associative_arrayはQ=200000の混合・hot key、Q=1000000の90%更新を測った。全て公式問題の入力制約内である。ordered_setの答えは独立したPythonの座標圧縮＋Fenwick、mapはdictで計算し、出力整数の全件hashと件数を照合した。23/24 buildに成功し、測定した395標本の出力が全て一致した。

N=Q=500000のordered_set混合における中央値は次の通り。時間はms、RSSはKiBで、RSSはI/O bufferなどを含むprocess peakである。

| compiler | 通常treap 時間 / RSS | 新OrderedSet 時間 / RSS | 原本409360 時間 / RSS | 原本410556 offline 時間 |
| --- | ---: | ---: | ---: | ---: |
| GCC | 260.650 / 36604 | 139.990 / 28100 | 34.003 / 19288 | 24.313 |
| Clang | 259.476 / 36840 | 141.069 / 28672 | 26.657 / 19424 | 24.047 |

新版は通常treapの約1.84〜1.86倍速かったが、原本409360には約4.12〜5.29倍の差が残る。原本は30bit専用の階層と適応bucketに加え、初期整列集合のbulk構築、問い合わせ先読み、prefetch、mmap I/Oを使う。全範囲の32/64bit整数・未知キーへのonline操作を持つ公開APIとは条件が異なる。409538も同じ傾向で、410556は全問い合わせを先読みして座標圧縮するoffline solverである。これらの差をasmだけの効果とはしない。新OrderedSetの初期要素は通常のinsertで構築し、bulk専用APIは追加していない。

Q=1000000・更新90%のassociative_arrayにおけるprocess wall中央値は次の通り。`reserve(q)`は成長を含む既定版と別の候補として扱う。

| compiler | std::map | 新OrderedMap | std::unordered_map | std unordered reserve(q) | 新HashMap | 新Hash reserve(q) | 原本362188 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| GCC | 485.268 | 273.913 | 176.764 | 128.340 | 66.067 | 66.782 | 26.175 |
| Clang | 500.535 | 274.626 | 206.324 | 160.709 | 79.690 | 74.102 | compile失敗 |

新版OrderedMapはこの問題でstd::mapの約1.77〜1.82倍速く、新版HashMapの既定版はstd::unordered_mapの約2.59〜2.68倍速かった。GCCの原本362188にはなお約2.52倍の差が残る。原本は固定2^20 slotの乗算hash・線形探索と専用I/Oを使い、削除・動的成長を持たない。Clangでは原本のconstexpr LUTが未初期化std::array要素をコピーするためcompileに失敗した。[compile log](whole/clang++-original-362188-fixed.compile.log)を保持し、欠測を0に置き換えていない。

速度・保持量は常に改善しない。GCCのhot keyでは新HashMapに`reserve(q)`を指定すると12.932msで、stdのreserve版11.259msより約15%遅かった。新Hashの既定版は10.674msだった。Q=1000000で新OrderedMapのRSSはGCC/Clangで61732/61148KiB、std::mapは46072/46020KiBとなり、約33〜34%多い。新Hashの既定版は29820/29928KiB、reserve版は38048/38028KiBで、必要以上のreserveには保持量の不利もある。RSSは大きいPython親processをforkした値を使わず、小さい`/usr/bin/time` helperから起動して測った。

[whole-before-optional-fix](whole-before-optional-fix/interrupted.json)はoptional<pair>の例外経路修正のため中断した35標本で、全一致でも最終集計には使わない。修正後にwholeを全件測り直した。その後のerase(iterator)のGCC警告修正はwholeのdriverでは未使用であり、[4 binaryのbyte同一性](whole/current-header-equivalence.json)を確認した。測定時と現在のheader snapshot・SHAをそれぞれ保存し、異なるbinaryの測定を混ぜていない。

## 反復子の比較

[iteration/summary.json](iteration/summary.json)はuint64_tのキー・値、GCC13.3/Clang18.1、GNU++20 portable `-O2 -DNDEBUG`、CPU0固定、各5反復の測定である。標準map/setと新版の実際のbegin/end・逆反復子・erase(pos)を直接使った。Mapは`for (auto& [k, v] : map) v += k;`で実際のpairへの参照を更新する。構造変更で既存反復子が全て失効する契約に従い、消去時は返された反復子だけを保持し、end()を取り直した。

N=50000の乱数・単調、N=500000の乱数について、parse、構築、const順走査、const逆走査、Map値更新、更新値照合、erase(pos)による隔個削除、削除後の両方向照合を分離した。値加算はuint64_tでwrapする。Pythonのdict＋sortによる独立oracleで全digest・件数・最終sizeが一致し、2 build・120標本が成功した。Setに存在しない値更新の時間はnullで保持する。[environment.json](iteration/environment.json)、[driver snapshot](iteration/driver.cpp.snapshot)、[runner snapshot](iteration/runner.py.snapshot)も保存する。

N=500000・乱数の中央値を示す。時間はmsで、各欄は標準→新版である。走査には共通の順序依存digest計算を含み、値更新後の照合時間は値更新そのものから分離した。

| compiler / 種類 | 構築 | const順走査 | const逆走査 | Map値更新 | erase(pos) | process wall |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| GCC map | 187.156→126.424 | 54.385→2.741 | 52.326→3.334 | 51.866→1.735 | 39.849→48.670 | 540.150→213.363 |
| GCC set | 116.601→106.728 | 35.663→1.378 | 33.544→1.339 | 対象外 | 29.178→43.550 | 305.069→178.125 |
| Clang map | 164.298→122.700 | 49.980→2.680 | 58.591→3.366 | 54.604→2.359 | 45.227→45.320 | 567.623→220.560 |
| Clang set | 122.635→101.712 | 36.796→1.353 | 31.556→2.075 | 対象外 | 33.491→43.263 | 324.733→187.304 |

大きい乱数集合では葉内配列と葉リンクによる局所性が有効で、順走査は約18.6〜27.2倍、Map値更新は約23.1〜29.9倍速かった。これはasm指定の効果を測った値ではない。process wallは多数の走査と削除後照合、入力、整形、破棄、起動を含む同じ一連の処理であり、単一APIの速度として使わない。parse時間も両候補で別に記録し、例えばGCCの大きいmapは22.227→22.315msだった。

erase(pos)には不利がある。新版は位置からキーを取り出してO(log N)の検索・平衡化・次位置の再検索を行う一方、stdの位置消去は償却O(1)である。N=50000の単調入力では新版の位置消去が標準の約8.7〜10.8倍遅く、一連のprocess wallも約1.28〜1.44倍遅かった。小さい乱数でも位置消去は約3.8〜4.4倍遅い。小Nの構築は約1.12〜1.46倍遅く、Clangの小さい単調mapの逆走査も0.170→0.181msと僅かに悪化した。走査が速いことだけを根拠にstd::map/setの全面代替とはしない。

RSSはN=500000乱数mapでGCC42312→40472KiB、Clang42296→41012KiB、setで34372→28272KiB、34468→28496KiBだった。一方N=50000乱数mapでは約22〜24%多かった。入力vectorなども含むprocess peakで、コンテナ本体だけのbyte数ではない。phase順は固定のため前の走査によるcacheの影響も含み、候補順のrotateだけでOS負荷までは統制していない。

[iteration-before-warning-flags](iteration-before-warning-flags/failures.json)の初回はGCC13の`-Werror=maybe-uninitialized`でcompile失敗し、Clang60標本のみ全一致した。原因はerase(pos)の条件付きoptional<Key>に対するGCCの警告だった。警告を残して実行した[第二phase](iteration-before-erase-warning-fix/summary.json)は120標本一致したが、採用版には使わない。公開headerをboolのhas_nextと初期化済みKeyへ最小修正し、最終phaseはGCC/Clangとも`-Wall -Wextra -Werror`で成功した。失敗・再試行のsourceとログを残した。
