# Link-Cut Tree: 同じAPIで不要な集約計算を減らす実験

baselineは`e0d8e71`の`blueberry/graph/link-cut-tree.hpp`。第三者コードは含まない。
`candidate.hpp`は測定時点の候補を固定し、実際の公開headerと両方のhashを記録する。
公開API、非可換の順序、反転、根、失敗するlink/cutの動作は変更しない。

候補は次の4条件。

- `baseline`: 変更前。
- `identity-only`: 存在しない子に対する単位元との結合を省く。単位元則を使い、演算の可換性を仮定しない。
- `rotation-only`: rotationでは降りた親だけpullし、上昇中の頂点はsplayの最後にpullする。後続rotationはその頂点の古い集約を参照しない。
- `candidate`: 上記2点に加え、auxiliary rootなら祖先stackを作らずpushのみ行い、accessで右子が変わらなければpullを省く。

stack自体は残す。遅延反転を祖先から順に伝播する契約を維持する。
演算呼び出し回数は副作用のないモノイド演算としての最適化対象であり、opの可換性やSのtrivial性、default constructorは新たに要求しない。

## 測定方法

Linux上で、他のCPU負荷の高い測定が停止していることを確認してから、リポジトリrootで実行する。
各実行の出力ディレクトリは新規にし、過去の結果を上書きしない。

```sh
python3 docs/development/measurements/generic-performance/lct/run.py \
  --output .build/lct-quick \
  --profiles gcc-release --variants baseline,candidate \
  --n 4096 --q 20000 --repeats 3

python3 docs/development/measurements/generic-performance/lct/run.py \
  --output .build/lct-full --count-ops

python3 docs/development/measurements/generic-performance/lct/validate.py \
  --output .build/lct-validation-new \
  --problem-root "$HOME/.cache/online-judge-tools/library-checker-problems"
```

全測定はGCC release／assert有効／Clang release、4候補、sum／非可換affine／非可換2x2 matrix、path／star／random／dynamicの組合せ。
既定ではN=8192、Q=60000、seed=20260919、各条件warmup1回と測定5回。
条件の順序を固定seedで毎周shuffleし、同じ論理CPUへ固定して逐次実行する。OS負荷や周波数変動までは排除できない。

入力・トレース生成は計測外。buildにはコンストラクタと初期辺のlinkを含む。
operationsにはパス積、set/get、evert/leader、same、失敗link/cut、およびchecksum更新を含む。
dynamicは固定されたcoreの木に付くleafを切って別のcore頂点につなぎ直す。
任意の内部辺の再接続を代表するとは主張しない。それは2種類の公式動的木問題で別途検証する。
path／star／randomは辺集合を維持するが、evertと失敗操作でもpreferred pathは変わる。

全条件のchecksumと操作の成否を照合する。ただしbaselineとの一致は独立なcorrectness証明ではないため、
`tests/random/link-cut-tree.cpp`の文字列連結+BFS比較、および公式sum／composite全ケースを別途実行する。
ランダムテストは逆向き、同値頂点、失敗辺操作後の根、copy/moveの独立性も確認する。

`--count-ops`は別の計数用binaryを使用する。計数binaryの実行時間を速度測定に混ぜない。
`counts.json`には構築中／操作中のop呼び出し回数のみを残す。
プロセス時間には起動・生成・破棄・出力を含み、RSSはプログラム全体の最大値である。
中央値だけでなくmin/maxと全sampleを保存し、compiler、flags、CPU、全ソースhashを記録する。

[結果と採否](REPORT.md)に、全候補を組み合わせた改善の採用理由と、加算の小さな悪化・測定の揺らぎも記録した。
既存の`results/validation/results.json`はLCT本体・random test・2本のverify driverのhashを記録している。
初回validation scriptはFastIOの検索パスが誤っていたため、その依存headerのhashを記録していなかった。
これは出力照合結果を変えないが、当該manifestだけではFastIO依存の版を固定できない。
再現用scriptのパスは修正済みで、次回以降はFastIOのhashも記録する。旧結果は書き換えていない。
