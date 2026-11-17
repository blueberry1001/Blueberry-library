# GCD / LCM 畳み込みの2配列変換（2026-11-17）

GCD / LCM 畳み込みは2本の入力を同じ約数・倍数ゼータ変換にかける。旧版は各配列で篩を別々に構築・更新した。候補は同じ素数・添字を訪れる1回の走査内で両配列を更新し、篩の処理を共有する。各配列の演算順は旧版と同じであり、入力はこれまでどおり値渡し、添字0は無視する。加算・減算・乗算とコピーを備える値型の環、空入力、各単独変換の契約と計算量は維持する。ACLに同等のGCD / LCM畳み込みはない。

## 参考実装と最速提出

[sotanishyの変換](https://sotanishy.github.io/cp-library-cpp/convolution/divisor_zeta_moebius_transform.hpp.html)は素数の篩と1配列の変換を同じ走査で行う。[suisenのGCD畳み込み](https://suisen-cp.github.io/cp-library-cpp/library/convolution/gcd_convolution.hpp.html)は演算を指定できるが、両入力の0番が零元であることを要求する。[maspypyの変換](https://maspypy.github.io/library/nt/zeta.hpp.html)は素数表を別に生成・キャッシュし、0番の零元を前提とする。本ライブラリは空配列と任意の0番を扱う。これらは構造と契約の調査であり、ソースは転載していない。

Library Checkerの[GCD](https://judge.yosupo.jp/problem/gcd_convolution)と[LCM](https://judge.yosupo.jp/problem/lcm_convolution)に対応する。GCDの[提出例 #153821](https://judge.yosupo.jp/submission/153821)を確認したが、現在の最速順位・提出ソースを取得できず、最速コードや上記外部実装との同条件直接比較は未実施。

## 実測

Windows 11、Intel64 Family 6 Model 183 Stepping 1、Unity同梱 Clang 18.0.3、MSVC ABI、`-std=gnu++20 -O2 -DNDEBUG`。旧版と公開ヘッダを同じバイナリへ組み込み、同一seed・入力で別プロセスを交互に起動。親と子をCPU affinity `0x2` に固定し、ウォームアップ2回後の7回中央値、最小・最大を記録した。時間は記載回数分の合計で、畳み込みAPIの値渡しコピー・内部確保を含む。入力生成、結果のハッシュ、結果vectorの破棄、プロセス起動とI/Oは含まない。

| 操作 | 型 | 配列長 | 反復 | 旧版 ms | 候補 ms |
| --- | --- | ---: | ---: | ---: | ---: |
| GCD | ACL modint998244353 | 1,000 | 1,000 | 9.092 (8.636–10.662) | 6.740 (6.372–6.814) |
| GCD | ACL modint998244353 | 100,000 | 15 | 21.048 (20.879–21.724) | 16.230 (15.875–16.754) |
| GCD | ACL modint998244353 | 1,000,000 | 2 | 37.317 (35.940–40.865) | 32.906 (32.325–37.460) |
| LCM | ACL modint998244353 | 1,000 | 1,000 | 8.786 (8.340–9.248) | 6.968 (6.217–7.238) |
| LCM | ACL modint998244353 | 100,000 | 15 | 21.571 (20.807–22.514) | 17.339 (16.792–18.361) |
| LCM | ACL modint998244353 | 1,000,000 | 2 | 39.817 (38.751–40.475) | 36.358 (34.077–44.802) |

大きい入力では一部の測定範囲が重なり、入力やCPUなどが異なる環境の速度差は断定できない。`uint32_t`、配列長0・1・2・20を含む全ケースと、単独の約数・倍数ゼータ/メビウス変換も測った。単独変換の100,000要素は範囲が重なり、小さな差を改善とは扱わない。N=1,000,000は配列全体の長さであり、公式最大入力そのものではない。

プロセス全体のpeak working set中央値は、ACL modint・配列長1,000,000でGCDが旧版18.969→候補18.973 MiB、LCMが旧版18.969→候補18.973 MiB。ランタイムと入力も含むため、ライブラリ単独のメモリ量ではない。取得失敗はエラーとして扱い、ゼロで埋めない。[全648試行・checksum・個別ピーク・実行条件](results.json)を保存した。

## 正しさと再現

長さ0〜100・各20ケースを固定seedで生成し、両畳み込みを二重ループの愚直解と照合。同一vectorを両引数に使う場合、任意の0番、ACL modint、`operator+=` / `-=` / `*=` が可変参照のみを受ける値型と、4種類の単独変換を旧版と比較した。公開ヘッダを直接includeするテストはClangのgnu++20 / gnu++23、`-Wall -Wextra -Wshadow -Werror`、assert有効/無効の4構成すべてで成功。実行コマンド、出力、終了コード、ソース・バイナリのSHA-256も`results.json`に保存した。

[旧版原本](baseline-original.hpp.txt)、名前空間だけ変更した[比較用旧版](baseline.hpp.txt)、[測定時の公開ヘッダ](candidate.hpp.txt)、[ドライバ](bench.cpp.txt)、[再現スクリプト](run.py.txt)を保存した。スクリプトは作業ツリーの公開ヘッダを直接コンパイルするため、再実行時は記録されたSHA-256との一致を確認する。実行ファイルはリポジトリに含めない。

予備試作の最初の出力は端末上だけで保存できなかったため、同じビルド済みバイナリで追試した[ログ](pilot-rerun.log)と[試作コード](pilot-bench.cpp.txt)・[実行スクリプト](pilot-run.py.txt)を別に残す。最終結果へ予備試行を混ぜていない。Linux GCC / Clang、公式入力、`make test`・`make verify`・`make docs`はこのWindows調査では未実行。
