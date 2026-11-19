# 永続 Union Find の2点同時更新（2026-11-18）

成功した merge の2回の永続配列更新を1回にまとめ、根から分岐点までの重複コピーを減らす。
配列更新は private で、公開 API と版番号は変えない。ACL DSU は永続化に対応しない。
[Nyaan](https://nyaannyaan.github.io/library/data-structure/persistent-union-find.hpp) の16分岐と
[Luzhiled](https://ei1333.github.io/library/structure/union-find/persistent-union-find.hpp.html) の8分岐配列も参考にした。
Library Checker の最速提出については下記のとおり設計を比較した。同条件での時間差は未評価。

2026-11-19 00:56 JST 時点の [Persistent Unionfind 最速提出 #409831](https://judge.yosupo.jp/submission/409831)
は 15 ms・6,852,608 B。全操作を先読みして必要な版の木を逆順に作り、DFS と rollback、
枝をまたがない限定的な経路圧縮、バッファ I/O を用いる。これは入力全体を先に受け取るオフライン処理で、
任意の過去版にオンラインで操作できる本ライブラリとは契約が異なる。提出時の judge 記録と
以下の WSL 実測は環境・入出力経路が異なり、直接比較できない。取得結果と調査は
`fastest.json` と `fastest-report.md` に保存。調査した提出ソースの SHA-256 は
`6afeed1fa9597a54433e8eb00c1db918a45d40235d6d9eebe5157efc60b72840`。

## 条件と結果

WSL2、Intel Core i7-14650HX、logical CPU 2 に固定し、GCC 13.3／Clang 18.1、
`gnu++20 -O2 -DNDEBUG`、同じ入力を3版のランダムな順序で実行。warmup 2回、測定15回の中央値。
旧版、参照渡しの初期候補、値渡しの採用候補の順。入力生成と `same` は時間に含めない。

| コンパイラ | 入力 | 旧版 ms | 参照渡し ms | 値渡し ms |
| --- | --- | ---: | ---: | ---: |
| GCC | 直前版から乱択併合20万件 | 226.745 | 225.015 | 222.366 |
| GCC | 過去版から乱択併合20万件 | 157.028 | 144.680 | 141.709 |
| GCC | 自己併合300万件 | 21.070 | 24.043 | 18.099 |
| GCC | 隣接成分を平衡に併合131071件 | 74.557 | 39.776 | 38.307 |
| Clang | 直前版から乱択併合20万件 | 244.652 | 244.422 | 233.889 |
| Clang | 過去版から乱択併合20万件 | 165.424 | 153.451 | 150.621 |
| Clang | 自己併合300万件 | 31.378 | 31.205 | 31.533 |
| Clang | 隣接成分を平衡に併合131071件 | 84.047 | 51.774 | 48.232 |

隣接成分のケースは共有経路が長く、計数専用ビルドでノード数は旧版4718556個、候補2621403個。
過去版からの乱択では7199928個から6800209個。平衡ケースのプロセス全体の最大 RSS 中央値は
GCC で 109908 KiB から 59908 KiB。vector の容量拡大の閾値と入力分布に依存する。
自己併合では両版とも版管理配列の再確保が起こり、小さな絶対差は環境にも左右される。

公式 Persistent Unionfind の14入力を同じ GCC・CPU 固定で各版5回と warmup 1回ずつ実行し、
全252実行が checker に成功した。実行時間には起動と I/O を含む。max_random では入力ごとに
優劣が入れ替わり、安定した差は確認できない。採用版の論理的な問い合わせ処理は変更していない。

## 保管ファイルと注意点

- `byvalue/byvalue-pinned-gcc-isolated.json` と `byvalue/byvalue-pinned-clang.json` に全サンプル、
  compiler、フラグ、CPU、affinity、時刻、入力条件、source/binary hash、RSS を保存。
- `byvalue/byvalue-pinned-gcc.json` は他プロセスの小規模読み取りと重なった可能性があり、
  干渉ありの参考記録として保持。独立した追試の代わりには使わない。
- `byvalue/byvalue-official-gcc.json` は GNU time が負の経過時間を返し途中停止した失敗記録。
  数値を補完せず保存し、別の `byvalue/byvalue-official-gcc-retry.json` で全ケースを再実行した。
- `initial/` は前段の実験結果、`byvalue/` は3版比較の結果。各ディレクトリに実際の比較元ヘッダ、
  ドライバ、生成用コード、計数用コードをそのまま保存。正式ヘッダはコメントの削除と private
  前提 assert の削除を含むが、測定した `-DNDEBUG` の動作は同じ。assert 有効の
  debug 構成は測定ソースと正式ヘッダが異なり、その性能は比較していない。
- GCC の直接差分比較は5 seed、ASan/UBSan は旧版と候補で各5 seed 成功。GCC/Clang の
  gnu++20/23 は実装後の独立テストで各5 seed 成功。正式な対象 verify は両ドライバ各3回成功。
- `validation/` に実装後のコンパイラ・random テストと対象 verify のログを保存。
  ドキュメント生成は依存解析中に WSL が再起動して中断したため、途中ログを保管し成功として扱わない。

参照渡し候補では GCC が `merge` に stack canary を付け、値渡し候補では `merge` 自体の
インライン化判断も変わった。自己併合の差を一つの命令へ帰属させる根拠はない。WSL 再起動前後は
実行環境が変化しており、異なる回の絶対時間を直接比較しない。
