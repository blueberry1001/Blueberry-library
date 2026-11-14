# Dynamic FenwickのCI性能比較

`dynamic-fenwick-performance` jobは、現在の1次元Dynamic Fenwickと、Nyaanライブラリの固定revision `b3981adc80a800b2584980b01821324ea6c77183` を同じrunner・コンパイラ・入力で測定します。2つの参照ヘッダのSHA-256も検査します。

既存の `benchmark/run_dynamic_fenwick_1d.py` を使い、固定seed 1729、30万操作、`gnu++20 -O2 -DNDEBUG`、固定区間の `contest-near`、`contest-diffuse`、`contest-broad` を対象にします。各実装について分布ごとに1回のwarmupと3回の測定を行い、順序を入れ替えます。入力生成とI/Oは計測時間に含みません。実装間・実行間のchecksumも一致させます。

各分布で現在の実装の中央値がNyaanの中央値の2倍を超えた場合、jobを失敗させます。ローカルの[比較記録](../../benchmark/dynamic-fenwick-1d.md)では今回の実装の中央値は対象3分布すべてで参照より短く、2倍はその結果から大きな余裕を取った検出基準です。共有runnerの負荷や参照実装の乱数により揺れるため、失敗時は保存された全サンプル、環境、checksumとメモリを確認し、同条件で再測定します。

参照のcheckout失敗、checksum不一致、測定の欠落や無効な値、ソースハッシュの不一致は成功と扱いません。jobは失敗し、取得可能な記録をartifactに残します。結果はこの型と入力分布の性能を示すもので、全ケースの性能保証や提出先の実行時間を示すものではありません。checksum比較は正答性テストを置き換えません。
