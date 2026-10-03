# 代表的データ構造の測定 shortlist（未採用）

結論: `DynamicFenwickTree::get` の局所ブロック差分と `PersistentSegmentTree::get` の片側下降を、次回の実装候補として残す。今回変更したものはベンチマークと証拠だけで、production header は基準 `ca8a4df35269a6980cf0b9612878e420744fc860` のまま。dense DSU/Fenwick/SegmentTree の書き換えは提案しない。

- DynamicFenwick の get: 全24条件で中央値が短縮。release 59.4–91.6%、assert 有効 56.0–91.6%。全24条件で候補の最大値が基準の最小値を下回った。
- PersistentSegmentTree の get: 全24条件で中央値が短縮。release 19.5–56.6%、assert 有効 25.0–57.7%。23/24条件で候補最大値が基準最小値を下回った。
- Persistent の mixed（20% set / 80% get）: 全24条件で中央値が短縮。release 6.6–49.8%、assert 有効 7.6–48.1%。17/24条件で標本範囲が分離した。全 workload の一律高速化を意味しない。

## 実行条件と証拠

主測定は5回+warmup1回を実装/コンパイラ順を交互にして実行し、864 profiles（720測定+144warmup）を保存。同じ frozen binary による局所再確認は9回+warmup1回、140 profiles（126測定+14warmup）を別保存した。合計1004 profiles（846測定+158warmup）。別の48 allocation/arithmetic diagnostics は時間を採否に使っていない。全実装の入力/結果 checksum は一致した。主測定区間は2026-10-03 07:19:39–07:21:33 UTC、再確認は07:24:10–07:24:25 UTC。

GCC14.2 / Clang19.1.7、`-std=gnu++20 -O2 -Wall -Wextra`、release のみ `-DNDEBUG`。`-march=native` は不使用。Xeon Platinum8573C の共有ホストで、cgroup `cpu.max=400000 100000`（4 CPU相当）、cpuset `0-4`、memory.max16GiB。CPU固定はしていない。他workerのcompile/testを止めた窓で直列測定したが、他tenant・周波数変動まで管理したものではない。詳細は [host-limits.json](host-limits.json)、各 experiment の prepared/results/quiet-processes ファイルを参照。

28 binary が警告なしでcompile成功。最終runnerによる192の小ケースprofileで愚直oracle/checksum一致（失敗証拠保存を強化する前の192も別名で保持、計384実行）。[contract-checks](contract-checks/) は GCC/Clang の厳密compileと Clang UBSan を基準/候補双方で実施し、12組が成功。nonassignable additive value、signed座標境界、空/単一/非2冪の木、疎なidentity root、分岐version、非可換monoid等を確認した。これはproduction全体の公式judge gateを置き換えない。

準備時runner、更新前runner、二段のrunner-amendment、各prepared.jsonを保存している。候補生成/C++/header/binaryはその間変更せず、自己hash確認・小ケースgate・異常出力/不一致rowの保存を測定前に強化した。過去のarchive/report、production header、認証情報は変更していない。この性能調査ではネットワーク再試行やuploadは行っていない。

[plan.md](plan.md) に再実行コマンド・seed・stage定義・適用範囲を記載。全stageの正確なns値は [stage-comparisons.csv](stage-comparisons.csv)、再確認は [focused-stage-comparisons.csv](focused-stage-comparisons.csv)。各raw.jsonにはwarmupも含む全標本を保持。以下はms表記の中央値 [最小, 最大]、変化率は候補/基準−1（denseだけBlueberry/ACL−1）。入力生成・起動・出力・破棄は計時外、queryのchecksum/hash処理は計時内で双方同一。I/O・end-to-end改善率ではない。

## point get の全条件

DynamicFenwick はdomain2^40−1、初期add30,000、追加add50,000、get200,000、range25,000。broad/hotspot/2冪境界、uint64とmod998を分離。getだけを変更し、mixed操作列は測っていない。除去ブロックを `+=` で蓄積し最後に1回 `-` を行うため、Tの代入や `-=` を追加要求しない。除去部分は元のpref(p)の先頭部分列なので、元のsigned中間値が有効なら新しいoverflowを導入しない。浮動小数点の結合順・丸めは変わり得る。算術callbackの回数そのものはAPI保証ではない。

Persistent はN100,000、version生成50,000、get200,000、range25,000、mixed50,000。初期配列あり/implicit identity、直列/分岐version、uint64加算/非可換affineを分離。index/version assert・値返却・O(log N)を維持し、prod/set/applyには手を加えない。

### dynamic-fenwick

|mode|compiler|shape / scalar|baseline ms [min,max]|candidate ms [min,max]|変化|
|---|---|---|---:|---:|---:|
|assert|g++|broad / u64|235.663150 [229.228491, 268.396704]|19.807240 [19.378904, 21.539885]|-91.6%|
|assert|clang++|broad / u64|238.304920 [232.144994, 245.821825]|22.493057 [20.182515, 22.963752]|-90.6%|
|assert|g++|broad / mod998|238.938691 [233.399487, 245.136458]|22.912809 [20.918189, 26.425222]|-90.4%|
|assert|clang++|broad / mod998|230.552874 [215.359985, 246.851398]|22.860554 [22.647720, 23.210081]|-90.1%|
|assert|g++|hotspot / u64|9.035338 [8.476956, 9.863552]|3.276747 [3.028590, 4.089533]|-63.7%|
|assert|clang++|hotspot / u64|7.809252 [7.266512, 8.486033]|3.414381 [2.999108, 3.724551]|-56.3%|
|assert|g++|hotspot / mod998|9.195794 [8.829900, 9.245535]|3.237380 [3.070314, 3.629861]|-64.8%|
|assert|clang++|hotspot / mod998|8.611283 [8.286933, 8.870724]|3.344170 [3.234147, 3.369548]|-61.2%|
|assert|g++|boundary / u64|35.172512 [33.462629, 37.292605]|12.955366 [12.263357, 13.976013]|-63.2%|
|assert|clang++|boundary / u64|27.628469 [25.389481, 28.180917]|11.375094 [10.482281, 12.494485]|-58.8%|
|assert|g++|boundary / mod998|37.639226 [36.549436, 38.175606]|13.787417 [13.107574, 15.966350]|-63.4%|
|assert|clang++|boundary / mod998|26.568578 [25.653982, 31.221926]|11.679005 [11.520331, 13.307268]|-56.0%|
|release|g++|broad / u64|237.790592 [232.246531, 270.652612]|20.051332 [19.039606, 20.847362]|-91.6%|
|release|clang++|broad / u64|219.136239 [207.910995, 224.840014]|20.824406 [20.333460, 22.170627]|-90.5%|
|release|g++|broad / mod998|225.177864 [224.710286, 231.889116]|21.289604 [20.929677, 22.213387]|-90.5%|
|release|clang++|broad / mod998|224.909561 [222.567585, 233.434368]|20.655114 [19.875158, 22.181622]|-90.8%|
|release|g++|hotspot / u64|8.770631 [8.650796, 9.354986]|2.885206 [2.815337, 2.934145]|-67.1%|
|release|clang++|hotspot / u64|7.482336 [7.235168, 7.797727]|2.965788 [2.957526, 3.041655]|-60.4%|
|release|g++|hotspot / mod998|9.452445 [8.739020, 10.477813]|3.061333 [2.961982, 3.464344]|-67.6%|
|release|clang++|hotspot / mod998|7.815938 [7.443134, 7.991028]|3.061070 [3.025295, 3.199495]|-60.8%|
|release|g++|boundary / u64|34.550065 [33.399562, 36.682547]|12.718759 [12.249087, 13.373199]|-63.2%|
|release|clang++|boundary / u64|26.788944 [25.561113, 26.911148]|10.870438 [10.460173, 11.600444]|-59.4%|
|release|g++|boundary / mod998|36.802871 [34.940506, 39.411619]|13.753162 [13.462829, 15.591769]|-62.6%|
|release|clang++|boundary / mod998|27.300583 [25.827988, 32.063277]|10.905354 [10.364151, 12.277955]|-60.1%|

### persistent-segment

|mode|compiler|shape / scalar|baseline ms [min,max]|candidate ms [min,max]|変化|
|---|---|---|---:|---:|---:|
|assert|g++|dense-sequential / u64|99.401102 [91.923132, 107.851855]|61.617380 [57.663384, 68.493724]|-38.0%|
|assert|clang++|dense-sequential / u64|102.240218 [92.208336, 115.237031]|67.688885 [64.130756, 77.619966]|-33.8%|
|assert|g++|dense-sequential / affine-u64|193.131836 [167.210253, 255.861897]|81.709922 [80.376638, 84.014989]|-57.7%|
|assert|clang++|dense-sequential / affine-u64|164.301169 [154.535999, 165.420459]|88.026830 [84.073148, 89.365474]|-46.4%|
|assert|g++|dense-branch / u64|58.885509 [55.007610, 62.983697]|36.126946 [35.299113, 36.660342]|-38.6%|
|assert|clang++|dense-branch / u64|56.936086 [54.959534, 61.132693]|41.022710 [37.631063, 45.562858]|-27.9%|
|assert|g++|dense-branch / affine-u64|110.566315 [103.343200, 126.397846]|51.651626 [47.179620, 68.100688]|-53.3%|
|assert|clang++|dense-branch / affine-u64|92.571842 [83.955003, 100.418906]|54.914579 [48.533403, 56.127202]|-40.7%|
|assert|g++|sparse-branch / u64|21.586350 [20.003405, 22.815606]|16.184642 [15.365046, 17.153970]|-25.0%|
|assert|clang++|sparse-branch / u64|25.513119 [22.641448, 27.613109]|14.384162 [14.254236, 16.100015]|-43.6%|
|assert|g++|sparse-branch / affine-u64|37.402817 [35.841461, 38.578749]|20.047073 [19.056991, 22.375487]|-46.4%|
|assert|clang++|sparse-branch / affine-u64|27.640843 [27.018461, 34.051411]|17.923151 [17.191718, 20.566776]|-35.2%|
|release|g++|dense-sequential / u64|97.355274 [92.877312, 103.992945]|59.178994 [58.429688, 70.324013]|-39.2%|
|release|clang++|dense-sequential / u64|102.794997 [93.002547, 108.757170]|66.250621 [63.332098, 71.892778]|-35.6%|
|release|g++|dense-sequential / affine-u64|180.816767 [165.215802, 187.838352]|78.510861 [74.889308, 95.287932]|-56.6%|
|release|clang++|dense-sequential / affine-u64|164.802752 [156.579140, 170.006078]|93.893918 [80.214465, 126.684031]|-43.0%|
|release|g++|dense-branch / u64|59.344784 [51.726943, 71.493782]|33.061431 [31.378160, 37.581247]|-44.3%|
|release|clang++|dense-branch / u64|58.718543 [55.029569, 67.923098]|36.666157 [35.768581, 42.933566]|-37.6%|
|release|g++|dense-branch / affine-u64|114.441452 [107.106285, 118.333145]|50.062612 [41.161091, 52.065867]|-56.3%|
|release|clang++|dense-branch / affine-u64|85.677551 [77.698338, 95.344224]|49.906731 [47.877155, 53.849150]|-41.8%|
|release|g++|sparse-branch / u64|20.757290 [19.757606, 23.074538]|16.717651 [14.724441, 22.537123]|-19.5%|
|release|clang++|sparse-branch / u64|24.453374 [20.243430, 25.507418]|16.196182 [14.521876, 16.675007]|-33.8%|
|release|g++|sparse-branch / affine-u64|38.024210 [33.798584, 49.744240]|18.071776 [17.793923, 20.541119]|-52.5%|
|release|clang++|sparse-branch / affine-u64|28.594737 [27.683415, 30.545250]|18.687410 [17.087509, 20.749486]|-34.6%|

## 変更していないstage・dense controlの注意点

初回の Persistent GCC release / dense-sequential update は uint64で+14.5%、affineで+16.4%遅かった。Dynamic Clang assert / boundary uint64 range も+14.9%。これらを除外せず、同じbinaryで再確認したところ+2.8%、+4.6%、+4.0%に縮小し、各標本範囲は重なった。差の原因を断定せず、変更していないstageの短縮を候補の効果として数えない。初回と再確認は統合せず保持する。

最も大きなdense outlierはGCC release / random SegmentTree get。初回Blueberry 0.687180ms [0.205381,0.736424]、ACL0.207561ms [0.181799,0.209061]、+231.1%（差0.479619ms / 200,000get）。getはBlueberryのbuild+update+range+get各中央値の合計55.723745ms中約1.23%を占めた。これは各中央値の和による参考割合で、独立したend-to-end計時ではない。初回の範囲は端でわずかに重なり、他コンパイラ/モードで同規模の差は出なかった。9回再確認のGCC releaseは+2.0%、他3条件は−1.3%、−0.3%、−6.2%で全範囲が重なった。3.3倍という差は再現せず、原因未確定のままdense実装を書き換えない。

再確認表（denseはACL→Blueberry、それ以外はbaseline→candidate）:

|対象stage|mode / compiler|scalar|基準 ms [min,max]|比較先 ms [min,max]|変化|
|---|---|---|---:|---:|---:|
|segment-tree get_ns|release / g++|u64|0.208152 [0.187265, 0.232768]|0.212228 [0.189841, 0.268783]|+2.0%|
|segment-tree get_ns|release / clang++|u64|0.200454 [0.178353, 0.548218]|0.197776 [0.169810, 0.226178]|-1.3%|
|segment-tree get_ns|assert / g++|u64|0.223170 [0.207103, 0.328843]|0.222476 [0.205148, 0.521431]|-0.3%|
|segment-tree get_ns|assert / clang++|u64|0.225503 [0.187106, 0.413063]|0.211450 [0.195899, 0.335990]|-6.2%|
|persistent-segment update_ns|release / g++|u64|38.885329 [32.130697, 47.190635]|39.987288 [35.639441, 52.375311]|+2.8%|
|persistent-segment update_ns|release / g++|affine-u64|55.925863 [47.880916, 63.598627]|58.493980 [48.353383, 65.027556]|+4.6%|
|dynamic-fenwick range_ns|assert / clang++|u64|4.828156 [4.670666, 7.602681]|5.019847 [4.809973, 5.938860]|+4.0%|

Dense controlはuint64の共通操作部分に限定。DSUはmerge戻り値/代表元の違いを比較せずsameとcomponent sizeを消費。Fenwickは両方zero構築+同じN回addで、BlueberryのO(N)配列constructor対ACL反復addの不公平な構築比較をしていない。point get対ACL sum(p,p+1)は値が等しい操作比較であり、ACLに同名APIがあるという意味ではない。SegmentTreeのget参照返却の契約差は即座の値消費で限定し（all_prodはdense測定対象外）、generic monoid全般の性能へ外挿しない。既存のdense get短絡・bit_floor・no_unique_addressは再実装していない。

## 別計測の操作・allocation diagnostics

Dynamic getで呼ぶvalue_atの回数は各query座標からloop回数を正確に集計したもの（実際のbucket probeや衝突回数ではない）。`+=`と`-`、Persistentのmonoid opは別instrumented buildで計数した。以下のu64表の削減はmod998/affineでも対応する同一入力条件で確認している。

|family / shape|point reads|before|after|数える対象|
|---|---:|---:|---:|---|
|dynamic-fenwick / broad|200000|8005311|399609|value_at calls|
|dynamic-fenwick / hotspot|200000|2400081|398669|value_at calls|
|dynamic-fenwick / boundary|200000|4200851|1533873|value_at calls|
|persistent-segment / dense-sequential|200000|3337743|0|monoid op calls|
|persistent-segment / dense-branch|200000|3337829|0|monoid op calls|
|persistent-segment / sparse-branch|200000|925826|0|monoid op calls|

全48 diagnosticsでget中のnew/new[]は基準/候補とも0。build/init/update/mixed等のallocation呼出数・要求byte数も対応する基準/候補で一致した。get時allocation削減による効果とは主張しない。Persistentはget中の恒等元とのop呼出しが0になり、Dynamicはhash lookupと加算回数を減らしている。stageごとの詳細は各diagnostics.jsonに保存。

RSSはLinux process peakで、入力・allocator保持・runtime・全stageを含む。要求byte数は累積でpeak-live容量ではなく、RSS差をget固有のmemory削減として扱わない。dense allocation診断は未測定で0ではない。I/O処理は今回の対象外。

## 採否と次の範囲

二候補とも「測定済みの有力候補」として残し、今回のsurveyでは採用しない。実装に進める場合は別の変更単位でproduction差分・公開docs・必要なunit/random/official検証をレビューする。元のofficial全体gateの既知blockerをこの局所測定で解消したとは記載しない。dense controlから新たな最適化候補を追加せず、このbounded batchを終了する。
