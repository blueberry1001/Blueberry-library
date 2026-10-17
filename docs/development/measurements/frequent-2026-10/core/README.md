# FPS・Potential Union Findの比較

基準は`d6af3e9`。同じ入力・compiler・flagsで、公開APIを維持した変更前後を比較した。
FPSは`integral()`のみ、Potential Union Findは`leader()`のroot直下の早期returnのみを変更した。
ACLは`864245a00b00dd008d1abfdc239618fdb7d139da`を両方で利用した。

## 条件と再現

リポジトリのルートで次を実行する。`--prepare`でbaselineをgitから抽出し、
比較用の両実装を同じbinaryへコンパイルする。測定中は別のcompile/test/benchmarkを動かさない。

{% raw %}
```sh
python3 docs/development/measurements/frequent-2026-10/core/run.py --prepare
python3 docs/development/measurements/frequent-2026-10/core/run.py
```
{% endraw %}

WSL2 / Core i7-14650HX、CPU0固定、seed=20261017。
GCC13.3/Clang18.1.3の`-std=gnu++20 -O2 -DNDEBUG`と、GCCのassert有効を分けた。
各条件は1回warmup後に5試行、各roundでbefore/afterの実行順を交互にする。
576回の全出力checksumが一致した（warmupを含む）。

- `final-environment.json`: compiler、flags、CPU、入力条件、測定sourceと対象headerのSHA256。
- `final-samples.csv`: warmupをrun=-1として含む全試行。FPSの構築時間は未測定の空欄。
- `final-summary.csv`: warmupを除いた中央値・最小値。未測定の構築は`not_measured`。

入力生成と最終出力のformatは計時外。FPSの計時には各操作の実行・返却領域の確保/解放・
結果のchecksum走査を含む。N=0/1/64/262144の積分、N=64/65536のlog、
変更しないevalとmultiplyを対照として測った。N=64は1000回、N=65536/262144は8回の操作合計である。
N=0は1000回、N=1は10000回の積分合計であり、異なるNの行同士を速度比較しない。

UFはN=65536/Q=1000000。入力ペアと重みの生成を除き、構築と操作を別に測った。
flatは全頂点をrootへ接続、balancedは2冪幅で併合、mixedは空のUFからmerge/same/diffを混在させる。
加算群はACL modint、行列群はdeterminant=1の2×2行列で、異なる上下shearを使って非可換な値も含める。
mixedには接続済み・未接続・矛盾するmergeが含まれる。

## 代表値

単位ms、各5回の中央値。FPSは表記した反復分の合計であり、UFは100万操作の合計。

| 条件 | 操作 | 変更前 | 変更後 |
| --- | --- | ---: | ---: |
| GCC release、N=262144、8回 | integral | 135.130 | 6.084 |
| Clang release、同条件 | integral | 149.566 | 5.807 |
| GCC assert有効、同条件 | integral | 133.028 | 5.982 |
| GCC release、N=65536、8回 | log | 105.437 | 71.070 |
| Clang release、同条件 | log | 113.793 | 76.857 |
| GCC release、加算群flat | diff | 4.634 | 2.754 |
| GCC release、加算群mixed | merge/same/diff | 6.819 | 5.187 |
| GCC release、行列群flat | diff | 20.224 | 5.880 |
| GCC release、行列群balanced | diff | 21.114 | 7.362 |
| GCC release、行列群mixed | merge/same/diff | 22.642 | 10.664 |
| Clang release、行列群mixed | merge/same/diff | 25.589 | 12.117 |
| GCC assert有効、行列群mixed | merge/same/diff | 23.549 | 11.235 |

積分はO(N log mod)からO(N)へ変えた。局所の出力bufferへ逆元をすべて構築してから係数を掛け、
追加のN要素bufferや共有static cacheを使わない。空入力は従来どおり`{0}`であり、
今回の極小計測の差から速くなったとは主張しない。
logの改善にはその内部から呼ぶintegralの効果が含まれる。

UFはroot直下で群の単位元と再合成する処理を省き、既存の経路圧縮と合成順を維持する。
単位元の合成が重い行列群で効果が大きい。全型・全分布に表と同じ改善率を保証しない。

変更しないeval/multiplyでも数%の差が出る試行があり、測定順序・code配置・runtimeの揺れを含む。
これらの差を今回の高速化の成果とは扱わない。Horner法、convolution結果のmove、
疎密でのNewton法切替は[調査候補](../research.md)にとどめ、今回の測定済み・採用済みとは扱わない。
この測定ではparse/formatやRSSを計測しておらず、それらの改善値を推定で補わない。

## 正しさ

積分は独立な係数ごとの除算と照合する。空・全zero・mod17の全有効長と、
NTT境界付近を含む既存FPSの逆元・log/exp/pow/sqrt・除算のテストを実行する。
UFは既存のBFSによる非可換群の愚直比較で、根の変更・合成順・矛盾を検証する。
GCC/Clang × gnu++20/23の4設定で、FPSとdata-structure-expansionの2プログラムを
各20seed、計160回実行して通過した。GCCのASan+UBSanでも両プログラムを
seed=20261017で実行して通過した。実行コマンドと終了状態は`validation.json`に保存した。
統合テストと公式checkerの最終結果は[全体レポート](../../../frequent-library-performance.md)に記録する。
