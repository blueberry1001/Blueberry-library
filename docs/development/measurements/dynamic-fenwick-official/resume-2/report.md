# Verification timings

Commit: 123365f2738e3d883c566def538e2833f411c317 / UTC: 2026-11-14T16:03:01.054404+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/math/taylor-shift.test.cpp / 0 | passed | 9.965804 s | 1.244088 s | 0.085796 s | 初回（基準なし） |
| verify/math/tetration-mod.test.cpp / 0 | passed | 8.082244 s | 0.104601 s | 0.010376 s | 初回（基準なし） |
| verify/string/aho-corasick.test.cpp / 0 | passed | 8.065632 s | 7.897993 s | 0.297375 s | 初回（基準なし） |
| verify/string/count-subsequences.test.cpp / 0 | passed | 7.742533 s | 0.497051 s | 0.064205 s | 初回（基準なし） |
| verify/string/eertree.test.cpp / 0 | passed | 6.510881 s | 1.689084 s | 0.142754 s | 初回（基準なし） |
| verify/string/enumerate-palindromes.test.cpp / 0 | passed | 7.405077 s | 0.494700 s | 0.041288 s | 初回（基準なし） |
| verify/string/kmp-search.test.cpp / 0 | passed | 6.324466 s | 0.200162 s | 0.040777 s | 初回（基準なし） |
| verify/string/longest-common-substring.test.cpp / 0 | passed | 7.441860 s | 1.066901 s | 0.091162 s | 初回（基準なし） |
| verify/string/lyndon-factorization.test.cpp / 0 | passed | 6.393872 s | 0.121091 s | 0.021818 s | 初回（基準なし） |
| verify/string/suffix-automaton-lcs.test.cpp / 0 | passed | 7.187520 s | 4.252034 s | 0.593036 s | 初回（基準なし） |
| verify/string/suffix-automaton-substrings.test.cpp / 0 | passed | 7.699003 s | 2.928275 s | 0.337530 s | 初回（基準なし） |
| verify/string/z-algorithm.test.cpp / 0 | passed | 7.871918 s | 0.373789 s | 0.023524 s | 初回（基準なし） |
| verify/utility/all-headers.test.cpp / 0 | passed | 11.600421 s | 0.975615 s | 0.169996 s | 初回（基準なし） |
| verify/utility/contest-fast-io-128bit.test.cpp / 0 | passed | 20.380014 s | 0.732769 s | 0.100847 s | 初回（基準なし） |
| verify/utility/contest-fast-io.test.cpp / 0 | passed | 17.517819 s | 0.819762 s | 0.141433 s | 初回（基準なし） |
| verify/utility/contest-template-128bit.test.cpp / 0 | passed | 15.919870 s | 1.748622 s | 0.246129 s | 初回（基準なし） |
| verify/utility/fast-io-128bit.test.cpp / 0 | passed | 4.983428 s | 0.451567 s | 0.060648 s | 初回（基準なし） |
| verify/utility/fast-io.test.cpp / 0 | passed | 7.418802 s | 0.310863 s | 0.050707 s | 初回（基準なし） |

## Environment

~~~json
{
  "cpu": "Intel(R) Core(TM) i7-14650HX",
  "os": "Linux-5.15.167.4-microsoft-standard-WSL2-x86_64-with-glibc2.39",
  "python": "3.12.3",
  "tools": {
    "online-judge-verify-helper": "5.6.0",
    "online-judge-tools": "11.5.1",
    "online-judge-api-client": "10.10.1"
  },
  "method": "oj-elapsed-log-v1",
  "recorder_sha256": "7c358016285643a5c9387f41f89a0c84bd800e29056d7d3795054ac46e4f2085",
  "jobs": 1,
  "tle_seconds": 60
}
~~~
