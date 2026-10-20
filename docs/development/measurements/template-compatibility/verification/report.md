# Verification timings

Commit: f9ccb0f704ee47974b372062684b4295bdc0ebea / UTC: 2026-10-20T12:24:18.395029+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/utility/all-headers.test.cpp / 0 | passed | 2.882738 s | 11.210487 s | 2.525810 s | 初回（基準なし） |
| verify/utility/contest-fast-io-128bit.test.cpp / 0 | passed | 2.618996 s | 2.859998 s | 0.602028 s | 初回（基準なし） |
| verify/utility/contest-fast-io.test.cpp / 0 | passed | 2.608484 s | 3.568867 s | 0.790746 s | 初回（基準なし） |
| verify/utility/contest-template-128bit.test.cpp / 0 | passed | 18.140941 s | 17.675112 s | 3.883790 s | 初回（基準なし） |
| verify/utility/fast-io-128bit.test.cpp / 0 | passed | 0.929030 s | 4.085372 s | 0.716002 s | 初回（基準なし） |
| verify/utility/fast-io.test.cpp / 0 | passed | 0.814749 s | 2.336411 s | 0.644045 s | 初回（基準なし） |

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
