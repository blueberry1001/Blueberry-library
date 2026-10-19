# Verification timings

Commit: cee0f9ec85da359e657798293583d6d30095f216 / UTC: 2026-10-19T07:04:48.995963+00:00
Baseline: なし（初回）

単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。
プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。

| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |
| --- | --- | ---: | ---: | ---: | --- |
| verify/math/sparse-exp.test.cpp / 0 | passed | 1.409565 s | 0.264992 s | 0.028160 s | 初回（基準なし） |
| verify/utility/all-headers.test.cpp / 0 | passed | 2.985673 s | 0.904907 s | 0.170048 s | 初回（基準なし） |

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
  "recorder_sha256": "55d07beba1c8a9f23e87f77888612fc48edfd43dc58e259585a43c1942fa34e6",
  "jobs": 1,
  "tle_seconds": 60
}
~~~
