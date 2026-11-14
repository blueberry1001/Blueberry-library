# 過去の検証記録による制限時間の点検

2026-11-14に、ローカルに残っていた `.verification/current.json` を点検しました。記録された実行は2026-10-17、Blueberry revision `a9d8741d19555021f0e42c10285d11ecf4f829f8`、GCC 13.3.0、WSL2 Ubuntu、Intel Core i7-14650HXです。各ケースの正答性タイムアウトは60秒です。上位4件はいずれも1回だけの測定です。

- 入力の保存先: [input-current.json](input-current.json)。SHA-256 `778c52a6078260d2d46c169aa5d5fe524dc50cba82d86d22a7c7fea3c7e8d5a9`。
- ケース別の記録: [run-0giio6vq](run-0giio6vq/)。入力JSONに記された同名ディレクトリからコピーしています。
- 制限時間: 公式 [library-checker-problems](https://github.com/yosupo06/library-checker-problems) revision `73876f1cbcf082d2b22722924b924d3d65a2f00e` の各 `info.toml`。対象ごとのパスとハッシュは結果JSONにあります。
- 結果: [screening.json](screening.json)（SHA-256 `f2024f33b1f46261cdd5fbdf889f4a6921159e132228a3d3f79b9cfbc8df8124`）、[screening.md](screening.md)。

点検時のBlueberry HEADは `123365f2738e3d883c566def538e2833f411c317` で、作業ツリーに変更がありました。測定時とは異なるrevisionです。以下は再測定の優先順位であり、現在の性能やjudgeの判定ではありません。

| verify | ローカル最大ケース | 公式制限時間 | 比 |
| --- | ---: | ---: | ---: |
| dynamic-rectangle-sum | 5.863秒 | 5秒 | 1.17 |
| dynamic-sqrt-tree | 3.961秒 | 5秒 | 0.79 |
| ordered-multiset-composite | 3.199秒 | 5秒 | 0.64 |
| dynamic-point-add-rectangle-sum | 3.104秒 | 5秒 | 0.62 |

点検対象169件のうち、比が1以上は1件、0.5以上1未満は3件、0.5未満は33件でした。残り132件はこの古い入力記録に測定値がなく、現在の未測定件数を示すものではありません。時間は単発の最大ケース値であり、マシンやデータの違いからjudge上のACやTLEを断定できません。

現在のツリーで同じ点検を再実行する場合は、公式リポジトリを上記revisionに固定し、リポジトリのルートから次を実行します。ドライバやライブラリが当時から変わると、出力や分類も変わります。

```sh
python3 scripts/audit_verify_limits.py \
  --report docs/development/measurements/verify-limit-screening/input-current.json \
  --problem-repo /path/to/library-checker-problems \
  --output .verification/reproduced-limit-screening
```
