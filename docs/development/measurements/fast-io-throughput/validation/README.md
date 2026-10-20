# 検証記録

実装commitは`8729666f80f04e080f945506fd3cb94be7081185`。
WSLのGCC 13.3 / Clang 18.1で検証した。ソースhashは`header.sha256`と各logに記録した。

| 確認 | 結果と記録 |
| --- | --- |
| 追加境界・乱択 | GCC/Clang × GNU++20/23、各seed 1..20成功。`new-api-final.sh` / `.log` |
| ASan/UBSan | seed 1の固定バッファ境界も含め成功。同上 |
| `make test` | 149 unit tests、116ライブラリ文書・117実行例に成功。`make-check-interrupted.log` |
| 全体compile | ヘッダ116本・verifyコード154本に成功。同上 |
| 全体random | 最初の`make check`がrandom中にHangup。再開した58本×20seedは成功。`random-resumed.log` |
| release include | `-DNDEBUG`でヘッダ116本の単独・重複includeに成功。`release-include.log` |
| 公式verify | 6ドライバ×全ケース×3回成功。`make-verify.log` / `official-results.json` / `official-runs/` |
| 性能ログ監査 | 主要8runの7,200行の出力照合・summary再計算・snapshot hashに成功。`measurement-audit.log` |

`make check`の最初のプロセスは正常終了していない。完了したunit/docs/compileと、
同じ実装を使う再開後のrandomを分けて記録し、中断したrunを成功扱いにしない。
`validate-fast-io.sh`、`resume-validate-fast-io.sh`、`verify-docs-fast-io.sh`が実行手順を表す。
`make docs`・Jekyll build・`check_site.py`はこの後に同じ一時checkoutで実行し、
PR/mainのCIでも公開前ゲートとして確認する。

公式verifyの初回再開では、非login shellのPATHに`~/.local/bin`がなく`oj`を起動できなかった。
`missing-oj/`へ失敗log・失敗reportを保存し、PATHを修正した後に全6ドライバをやり直した。
成功したreportが参照する一時パスのJSONは、basenameを保って`official-runs/`へ複製した。

ローカル公式verifyの入力はNTFS上の公式cacheで、assert有効。
性能比較のLinux `/tmp`入力・`-DNDEBUG`とは条件が異なるため、同一測定として集計しない。
このreportは対象6本だけの検証であり、全verifierの公開証拠としては使わない。
ローカルdocsのpreflightではreportを退避して`check_site.py --without-metrics`を使い、
完全な検証stateと公開結果はPR/mainのCIで確認する。

最初のdocs生成は、今回保存した3本のPython検証スクリプトがライブラリ扱いされ、
importlabの依存解析でtimeoutになった。`make-docs-python-analysis-timeout.log`を保持し、
既存の測定用スクリプトと同様に`.verify-helper/docs/_config.yml`で対象3本を除外した。
`docs-only-fast-io.sh`がこの設定修正後の再開手順である。

短い入力の回帰と追加2候補の採否は[inlining-review.md](inlining-review.md)を参照。
