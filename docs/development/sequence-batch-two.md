# Sequence batch two: candidate research and measurements

2026-09-18。公開コードをコピーせず、次の一次資料を比較して独立実装した。

| Library | Primary implementations inspected | Decision |
| --- | --- | --- |
| Disjoint sparse table | [Luzhiled](https://ei1333.github.io/library/structure/others/disjoint-sparse-table.hpp.html), [Alex Li](https://algorithms.alexli.ca/disjoint-sparse-table/) | 結合的半群、非空区間。lookup 配列を持たず C++20 bit_width。通常の segtree はメモリ O(N)・query O(log N)、DST は O(N log N)・query O(1)。 |
| LIS | [Luzhiled](https://ei1333.github.io/library/dp/longest-increasing-subsequence.hpp.html), [sotanishy](https://sotanishy.github.io/cp-library-cpp/dp/lis.hpp) | 二分探索 + predecessor 添字。数値番兵を避け任意の比較可能型と端点値を扱う。O(N log N) 時間・O(N) メモリ。 |
| Prefix function / KMP | [KACTL](https://github.com/kth-competitive-programming/kactl/blob/main/content/strings/KMP.h), [cp-algorithms](https://cp-algorithms.com/string/prefix-function.html) | pattern と text を連結せず走査。補助領域 O(M)、区切り文字に使えない値の制約がない。 |
| Cartesian tree | [Nyaan](https://nyaannyaan.github.io/library/tree/cartesian-tree.hpp.html), [sotanishy](https://sotanishy.github.io/cp-library-cpp/tree/cartesian_tree.hpp.html) | vector stack による O(N) 時間・メモリ。左端 tie、根自身を親として LC 出力に合わせる。後者の右端 tie とは仕様が異なる。 |

KACTL の参照ファイルは CC0。他のコードの直接再利用はない。

## Reproducible candidate measurements

[benchmark source](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/sequence-batch-two.cpp) と
[raw log](https://github.com/blueberry1001/Blueberry-library/blob/main/benchmark/sequence-batch-two.log) を保存。
同一 GCC 13.3.0、WSL2 Ubuntu、gnu++20 -O2 -I.、固定 seed 20260918、N=4000、各3回。
候補は参照ライブラリの転載ではなく単純な比較アルゴリズムを独立実装したもの。
LIS は O(N²) DP、Cartesian tree は区間最小値で再帰分割、KMP は文字ごとの愚直照合、DST は iterative segment tree と比較。
DST は構築 + 同じ4000区間の問い合わせを含む。KMP は text が同一文字列で pattern 最後だけ不一致の入力。
Cartesian tree の比較 checksum は親配列先頭と再帰ノード添字和で異なるが、測定外の random test で完全な親配列を照合する。

初回の計測値（ms、median/min）は以下。小規模な候補選定用の測定で一般的な優位や公開 judge の速さを保証しない。

| Candidate | median ms | min ms |
| --- | ---: | ---: |
| LIS binary reconstruction | 0.104545 | 0.098809 |
| LIS quadratic DP | 15.0303 | 13.971 |
| Cartesian stack | 0.032720 | 0.031191 |
| Cartesian recursive min | 0.111227 | 0.107900 |
| KMP scan | 0.012757 | 0.012617 |
| Naive matching | 0.978431 | 0.978125 |
| DST build + queries | 0.082568 | 0.081851 |
| Segment tree build + queries | 0.165942 | 0.163869 |

LIS の測定後、GCC の signed length 推論による free-nonheap-object 警告を避けるため vector のサイズに元の size_t を直接渡すよう変更した。
この初回値を最終 revision の測定値として読み替えない。最終 revision の再測定は sequence-batch-two-final.log に分けて保存する。

## Verification

公式 LC: static_range_sum / longest_increasing_subsequence / cartesian_tree。
KMP は公式 AOJ ALDS1_14_B（LC 問題ではない）。
random test は CLI seed、空入力、重複、整数端点、復元の順序、NUL、非可換 string 集約をチェックする。
公式実行結果は .verification/batch-two-sequence に保存し、統合レポートで完了状態を確認する。

最終 revision の再測定（ms、median/min）は別ログの値:

| Candidate | median ms | min ms |
| --- | ---: | ---: |
| LIS binary reconstruction | 0.202098 | 0.159609 |
| LIS quadratic DP | 20.1584 | 19.9275 |
| Cartesian stack | 0.033846 | 0.028843 |
| Cartesian recursive min | 0.126515 | 0.120697 |
| KMP scan | 0.014799 | 0.014426 |
| Naive matching | 1.76545 | 1.23708 |
| DST build + queries | 0.161809 | 0.137602 |
| Segment tree build + queries | 0.243849 | 0.243844 |

初回と最終回は並列開発中の負荷が異なるため、revision 間の性能改善・退化として比較しない。
公式4 driver は初回 single-repeat ですべて passed。LIS の allocation サイズ記述変更後は strict GCC random test を再実行し passed。
