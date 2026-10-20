---
title: "NTT kernel方向性の初期pilot"
---

# NTT kernel方向性の初期pilot

2026-10-20、CPU0。GCC/Clang、gnu++20 -O2、各3回。check.cpp.snapshotのuint32_t Montgomery入力を用いたkernelはNTT表構築を分離し、ACLのmodint入力の積と全係数を照合した。canonical uint32_tを受ける公開API全体の比較ではない。GCC/Clangのbaselineはいずれも後に実行しているため、実行順を回転するfinalを採用判断とする。

別担当のWavelet compile/random開始と重なった可能性がある。CPU負荷を統制できていないため、速度比の確定根拠にしない。results.logは仮測定のまま保存する。

再実行: `g++ -std=gnu++20 -O2 -I.deps/ac-library check.cpp -o check`、`taskset -c 0 ./check 524288`（スナップショットを同じディレクトリへ元の拡張子で復元）。表構築・kernel・ACLをmsで表示。乱数seed20261020。
