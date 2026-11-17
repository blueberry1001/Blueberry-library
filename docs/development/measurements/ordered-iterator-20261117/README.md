# OrderedMap / OrderedSet の位置消去

`erase(iterator)` は削除対象を検索し、平衡化後に後続キーを再検索していた。削除前の葉に余裕があれば借用・結合が発生しないため、元の後続位置を補正して返す。葉の結合や借用が起こり得る場合は従来どおり再検索する。根が葉で空になる場合や、後続が次の葉または `end()` の場合も同じ返却契約を保つ。`erase(key)` と共通の内部削除処理を使う。計算量は O(log N) のまま。

2026-11-17、Windows 11、Intel Core i7-14650HX、MSVC 19.43、`/std:c++20 /O2 /DNDEBUG /EHsc`。baseline と候補を同一入力で順序を入れ替えて各5回測定し、その前に各1回warmupした。100000個の異なる64bit整数を乱数順で挿入し、操作時間と構築時間を分離した。下表は操作時間の中央値（ms）。forward は先頭から全削除、alternate は1つおき、reverse は末尾から全削除、random はキーを乱数順に `find` して位置消去する。入力生成・構築は操作時間に含めない。

| 操作 | set 変更前 | set 変更後 | map 変更前 | map 変更後 |
| --- | ---: | ---: | ---: | ---: |
| forward | 12.94 | 11.39 | 14.49 | 13.18 |
| alternate | 9.32 | 7.67 | 10.01 | 8.29 |
| reverse | 9.83 | 9.67 | 9.79 | 9.73 |
| random | 37.28 | 31.25 | 37.70 | 32.61 |

同時に別セッションのテストも動作しており、CPU負荷は隔離できていない。特に数%以内の差は優劣を断定できない。[全576標本](results.json)には構築時間、64・4096要素、昇順挿入、全warmup、出力digestを保存し、各対のdigestと最終sizeが一致した。`setup-*-failed.log` と `failed-incomplete-vs18.json` は測定前の環境構築・コンパイル失敗で、値を補完していない。RSSは未測定。Linux/GCC/Clangでの性能比較も未実施。

既存の[公開高速提出の調査](../extra-fast-more/sets/README.md)では ordered_set の上位提出が固定値域に合わせた複数段の表現やクエリ先読みを利用している。今回の `erase(iterator)` は後続反復子を返す別の契約で、当該提出には同等操作がないため、ランキングの時間を同等APIの比較に使っていない。既存のmap/set比較は[以前の測定](../extra-fast-more/sets/iteration/summary.json)を参照。

正しさは既存の `extra-fast-ordered-iterator.cpp` と `extra-fast-associative.cpp` をMSVCのC++20 / C++latestでそれぞれコンパイルし、seed 1・17・20261020で計12実行した。順逆走査、削除後の後続、異なる葉への移動、借用・結合、root collapse、空化などを照合している。GCC/Clang・公式ケースの再実行は統合検証で行う。

候補ヘッダの測定時のバイト列は `candidate.hpp.snapshot` に保持している。公開ヘッダとの差は改行コードのみで、Git格納時はLFに統一する。
