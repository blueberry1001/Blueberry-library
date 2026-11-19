# Verification timings

Commit: efa8b4e97ce68395657c643fd6c04f2fba56b826 / UTC: 2026-11-17T03:30:12.693971+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/data-structure/aggregate-queue.test.cpp / 0 | passed | 5.354329 s | 4.845474 s | 0.863212 s | 初回（基準なし） |
| verify/math/factorize.test.cpp / 0 | passed | 4.399679 s | 0.645698 s | 0.051411 s | 初回（基準なし） |
| verify/math/primality-test.test.cpp / 0 | passed | 4.115199 s | 1.766018 s | 0.648549 s | 初回（基準なし） |

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
