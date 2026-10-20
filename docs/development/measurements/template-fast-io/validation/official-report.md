# Verification timings

Commit: 46a723272443c91bef717730e7931d4492e51236 / UTC: 2026-10-20T04:02:29.053871+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/utility/all-headers.test.cpp / 0 | passed | 6.739513 s | 16.495104 s | 4.249325 s | 初回（基準なし） |
| verify/utility/contest-fast-io-128bit.test.cpp / 0 | passed | 10.461090 s | 9.100966 s | 1.814923 s | 初回（基準なし） |
| verify/utility/contest-fast-io.test.cpp / 0 | passed | 13.861466 s | 6.844520 s | 1.839962 s | 初回（基準なし） |
| verify/utility/contest-template-128bit.test.cpp / 0 | passed | 15.022081 s | 41.608978 s | 8.517636 s | 初回（基準なし） |
| verify/utility/fast-io-128bit.test.cpp / 0 | passed | 4.503251 s | 11.323122 s | 1.882160 s | 初回（基準なし） |
| verify/utility/fast-io.test.cpp / 0 | passed | 6.812946 s | 7.601503 s | 1.805377 s | 初回（基準なし） |

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
  "recorder_sha256": "3ee628d2aa014ded1096cfaa43a315a355f3aa75c0fe7cc598efb0184a1c26b6",
  "jobs": 1,
  "tle_seconds": 60
}
~~~
