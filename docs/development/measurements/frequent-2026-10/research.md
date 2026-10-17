# 高頻度ライブラリの追加高速化調査（2026-10-17）

この資料は実装と一次資料を読んだ候補調査である。使用頻度の実測データはないため、最短路、木、静的区間問い合わせ、重み付きUnion Find、文字列、FPSと共通I/Oを競プロで繰り返し利用しやすい対象と判断した。ACLの標準データ構造・畳み込み・suffix array・Z algorithmは置き換えない。調査担当はCPUを使う測定・コンパイルを行っておらず、この資料の「候補」は採用済み・高速化確認済みを意味しない。採否と変更前後の数値は同じディレクトリの実装担当の測定記録を参照する。

## 既存の改善を重ねない

[2026-09の改善記録](../../generic-performance-improvements.md)では、WaveletMatrixの圧縮をN回の二分探索から必要なbyteだけのradix分配に変え、構築126.60→71.66msを記録している。LinearRMQ、SqrtTree、DST、FastSet、StaticRangeLIS、LinkCutTree、portableなFast I/Oも既に改善済みである。これらは当時の同一環境の比較であり、今回のrevisionの速度とは比較しない。

現行Dijkstraは整数Costについて、加算の前に宛先の現在距離で非改善とoverflowを除外する。Fast I/Oは64KiBバッファ、初期zero-fill省略、型ごとのoverflow境界、2桁出力を備える。FPSのinv/expもACL内部の変換を再利用している。「bufferを付ける」「radixを導入する」「Newton法にする」だけでは新しい改善にならない。

## 優先順位と採用判断の軸

| 優先 | 対象 | 同じ契約で試す候補 | 主な確認事項 |
| --- | --- | --- | --- |
| 1 | FPSの積分 | 各係数の逆元計算を1..Nの逆数漸化式へ変える | prime modulus、N<mod、空入力、複数modulus。log/powへの波及も測る |
| 1 | PotentialUnionFind | 親がrootなら単位元との合成・代入・再帰を省く | 非可換群、root/tie、矛盾merge、演算型の要件を増やさない |
| 1 | LCA | 深さに応じたダブリング表、DFS行きがけ順の親番号によるRMQ | 非0root・頂点番号の置換・有向/無向・祖先・パス/星/平衡木。RMQのO(1)を維持 |
| 2 | FPSのコピー | convolution結果と商をmoveで受け取り、同じ係数列の再コピーを省く | 自己乗算・自己除算、vectorとして公開された参照の扱い |
| 2 | SuffixAutomaton | cloneの2回目のmapコピーをmoveへ変更、同一遷移の二重探索を省く | cloneが多い入力、任意の全順序Symbol、オンラインappend、出現数再構築 |
| 2 | WeightedWaveletMatrix | 元位置をsortしてrankを書き戻し、N回のlower_boundを省く | Valueは整数限定でない。比較sort経路を維持し、追加メモリと小さい入力の回帰を測る |
| 3 | WaveletMatrixのrank | bitとprefixの配置、層の連続配置、共通prefixの利用を比較 | random/短区間/低σ、get/count/freq/prev/nextを含む。padding増加も測る |
| 3 | FPSの単点評価 | Horner法と現行のpower累積を比較 | modintでは厳密に同じ値。依存鎖が長くなるため乗算数だけで速度を決めない |
| 3 | Dijkstra | heapのreserve、小さい分岐数のheap、queue比較回数を調べる | 疎/密・0辺・到達率、queue peak、Cost全般、親経路tie、追加メモリ |
| 3 | Fast I/O | buffer内の連続走査、文字列のまとまりappend、4桁出力tableを個別比較 | EOF/pipe/失敗値不変/符号付き最小値/128bit、短tokenと長token、40KiB tableのcache費用 |
| 4 | RollbackUnionFind | 失敗mergeのundo時に重複書き戻しを省く候補 | 失敗mergeも1stateを消費する現行契約は維持。経路圧縮は入れない |
| 4 | AhoCorasick・KMP・Manacher | allocationと入力分布を測った後で判断 | 既に線形・連続配置が中心。低根拠の書き換えを避ける |

優先1は処理そのものを省く根拠が明確な候補である。優先2以降は費用の比率や入力分布で採否が変わる。表の順位は普遍的な使用頻度や改善率を保証しない。

## FPS

現行`integral()`は係数ごとに`/ Mint(i+1)`を実行する。ACLのprimeなstatic_modintでは除算が逆元を呼び、その逆元は二分累乗である。従ってN個の独立な逆元を計算する現行のO(N log mod)には、明確な不要処理がある。[ACLのmodint実装](https://raw.githubusercontent.com/atcoder/ac-library/master/atcoder/modint.hpp)

[NyaanのFPS](https://nyaannyaan.github.io/library/fps/formal-power-series.hpp.html)は、出力領域に1..Nの逆数を漸化式で構築してから係数を掛け、積分をO(N)にしている。Blueberryもprime modulusとN<modを既に要求するため、この考え方は契約を狭めず利用できる。共有のstatic cacheを追加せず、呼び出しごとの領域で完結させる。出力領域を使う場合、逆数を係数で上書きする前に必要な逆数の構築をすべて終える。商の小さい次数では通常の多項式長除算も比較候補だが、閾値は移植しない。

現行`operator*=(FPS)`はACLのconvolutionが返したvectorを`assign`で再コピーし、`operator/=`も完成した商を再コピーする。結果のmoveは同じ係数列を維持しやすい候補である。ただしFPSはvector継承であり、利用者が係数への参照を保持できる。APIページのinvalidation契約を確認し、参照を維持すると約束した操作まで不用意にstorageごと置換しない。

inv/expについては[NTT用FPS](https://nyaannyaan.github.io/library/fps/ntt-friendly-fps.hpp.html)が変換の再利用を示しており、Blueberryは既に同系統の改善を備える。今後は変換回数・初期化byte数・scratch再確保回数を数え、残る支配項が分かった範囲を変える。独自NTTや独自modintへ置き換える案はACL優先方針に反するため採用候補にしない。

[maspypyのfps_inv](https://maspypy.github.io/library/poly/fps_inv.hpp)は非zero項数で疎漸化式とdense NTTを切り替える。Blueberryでも適応分岐なら入力を制限せず利用可能だが、疎密判定のO(N)走査、degreeと元配列長の差、選択閾値、小さい入力の費用を測る必要がある。特定modint・独自convolutionで得た閾値や速度はそのまま移植しない。

Horner法は逆順に`result=result*x+coefficient`とすれば乗算数を減らせるという本実装からの推論である。modintでは値を維持するが、乗算の依存鎖が長くなる。単点evalの改善として独立に測り、NTT部分まで速くなるとは主張しない。今回Horner法と結果vectorのmoveは採用せず、追加候補として残す。

## LCA

[KACTLのLCA](https://raw.githubusercontent.com/kth-competitive-programming/kactl/main/content/graph/LCA.h)はDFSで親の行きがけ順番号を並べ、RMQの最小値からLCAを復元する。Euler列の深さと頂点のpairを約2N個保存する方法より、RMQ対象を小さくできる。同ソースはCC0・folkloreを明記する。参照実装のrootは0で再帰DFSを使うが、Blueberryでは任意rootと深い木での非再帰構築を維持する必要がある。

Blueberryの候補では行きがけ順と頂点番号を別配列で相互変換する。最小の頂点番号をLCAと誤解しないこと、異なるu/vでは区間を`(first[u],first[v]]`にすること、同じ頂点では空RMQを呼ばないことが境界条件となる。ダブリングの必要な最大bitも頂点数ではなく最大深さから決められる。頂点番号をshuffleした星・パス・平衡木、非0root、parent→childの入力、各2冪前後を重点に調べる。

## Dijkstra

[Nyaanのradix版Dijkstra](https://nyaannyaan.github.io/library/shortest-path/dijkstra-radix-heap.hpp.html)と[RadixHeap](https://nyaannyaan.github.io/library/data-structure/radix-heap.hpp.html)は整数の単調keyを利用する。参照実装のbit判定にはlzcnt属性があり、同距離bucketは末尾から取り出す。Blueberryはfloating/custom Costの経路と距離・親の復元を持つため、そのまま置き換える契約ではない。整数だけを内部分岐させる場合でも、同距離頂点の順序が親の選択に与える影響、全幅のunsigned infinity、0重みの連鎖を検証する必要がある。

CSRへの全置換も公開`WeightedGraph=vector<vector<WeightedEdge>>`を変えてしまう。呼び出しごとにCSRへ変換すると単発最短路の前処理・メモリが増えるため、heap本体と独立の案として扱う。queueのreserveはAPIを変えず試せるが、N個予約するとほとんど到達しないグラフで費用が増え、M個予約ではさらに大きい。改善の判定には実際のpeak queue長が必要である。

## Wavelet Matrix

[Luzhiledの実装](https://ei1333.github.io/library/structure/wavelet/wavelet-matrix.hpp.html)もbit列とprefix-popcountでrankを求める。MAXLOG固定の非圧縮版と圧縮版を持つため、Blueberryの符号あり全幅・実値域に応じた高さと同等の条件かを区別する。この資料からは、prefixを32bit単位から64bit単位にすれば常に速いとは判断できない。

Blueberryの通常版は圧縮と分配を既に改善している。次に試すのはrankの二つのloadと階層ごとの依存の費用である。bitとprefixを同じ小さな構造体へ入れると近接性は改善し得るが、alignmentで12byte相当が16byteになり、全体のメモリtrafficを増やす場合がある。全層の一括allocationも構築時のallocationを減らせる一方、可読性・空配列・move時の管理を複雑にする。測定前に採用しない。

Weighted版にはsort後にN回lower_boundする圧縮が残る。通常版の「元位置のsortからrankを書き戻す」考え方は整数に限らないが、radix専用化はValue要件を変える。重みのFenwick構築・各層のweight copyが支配的な場合は圧縮の改善が全体へ及ぼす効果も小さくなるため、構築を分離する。

過去の[Fastest調査](../../fastest-other-structures-study.md)ではRange Kthの全クエリ先読み・層ごとの索引再利用・AVX2を確認済みである。これらは任意順のオンライン問い合わせができる現行構造と別契約であり、数値をそのまま実装差とみなさない。

## 文字列

SuffixAutomatonではclone生成時にmapを元状態からlocalへコピーした後、localをvectorへ再コピーしている。2回目をmoveにする案は必要なcloneの独立性を保つ。未登録遷移の`contains`と`emplace`も同じmapを二度探索する。これは公開APIや計算量を変えず、mapの戻り値を使って省ける候補である。入力長が分かるconstructorだけ状態数の上限をreserveする案もあるが、input_rangeをsized_rangeへ狭めず、過剰予約によるメモリ増を測る。

[maspypyのSuffix Automaton](https://maspypy.github.io/library/string/suffix_automaton.hpp)は固定sigmaの配列遷移を使う。Blueberryの任意の全順序Symbolに対し、26文字などへ限定して同じAPIの高速化と報告しない。unordered_mapもSymbolにhashを新しく要求するため同様である。配列やhashの別実装を作るより先に、現行mapでの不要なコピーと探索を測る。

AhoCorasickは既に固定alphabet配列で全遷移をbuildし、countを逆BFS順で集約する。KMPとManacherも線形走査が中心である。ACL標準のsuffix array/Z algorithmを自作して最適化する優先度はない。[ACL string実装](https://raw.githubusercontent.com/atcoder/ac-library/master/atcoder/string.hpp)

## Rollback / Potential Union Find

PotentialUnionFindは群の合成が単なる整数加算より重い場合がある。rootのweightはidentityなので、直上の親がrootなら既に相対weightが完成している。統合担当がこの経路を評価する。非可換の積の順序は変えず、比較・hashなどの新要件も追加しない。

[NyaanのRollbackUnionFind](https://nyaannyaan.github.io/library/data-structure/rollback-union-find.hpp.html)は各mergeの2箇所を記録し、失敗mergeも履歴を進める。Blueberryは既にvectorのChangeにまとめており、一般のstackからvectorへ変える改善は済んでいる。失敗mergeの履歴を丸ごと省くとstate/undoの意味が変わる。経路圧縮を追加するとundoのO(1)保証が崩れる。重複rootのundo書き込み省略程度は同じ契約の候補だが、分岐増加に見合うかは失敗merge率を変えて判断する。

## Fast I/O

[NyaanのFast I/O](https://nyaannyaan.github.io/library/misc/fastio.hpp.html)はbuffer末尾に余裕を確保した連続走査と、4桁・40,000byteの出力tableを使う。Blueberryへ移す際は、入力が必ず正しいことや桁数が固定であることを暗黙に仮定しない。全整数範囲、EOF、誤token、pipe、複数FILE、対話版の契約を残す必要がある。

現行の1文字ごとの`next()`はbuffer境界を毎回確認する。buffer内だけを走査する速い経路と、境界・EOFを扱う遅い経路に分ける案は同じ契約を保てる。文字列は連続部分をappendしてpush_back回数を減らせる。4桁tableは現行2桁tableより除算が減る一方でcache占有が増える。64bit/128bitの長い出力と、小さい整数の短い出力を分けて測る。mmap専用、AVX2必須、globalの単一stream専用化は汎用実装の置き換えにしない。

## 測定と次の課題

採用対象は同じ入力・compiler・flags・CPUで逐次測定し、warmupと複数回の全sampleを保存する。構築・操作・parse/format・end-to-end・peak RSSを区別する。今回の調査だけから高速化率を付けない。GCC/Clang、assert有効/無効、gnu++20/23、randomと境界、重い型・非可換演算を必要な対象で確認する。

一次資料は2026-10-17に閲覧した公開ソースであり、ランキングの最新最速を断定するものではない。採用しなかった案も、単に「遅い」とせず、契約が違う、未測定、回帰があった、効果より複雑化が大きい、という理由を記録する。
