# Library Checker fastest: `range_kth_smallest`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [403008](https://judge.yosupo.jp/submission/403008) | nandhagk | cpp | 20.0 ms | 20.0 ms | 9224192 B |
| 2 | [393928](https://judge.yosupo.jp/submission/393928) | chaihf | cpp | 35.0 ms | 35.0 ms | 9355264 B |
| 3 | [376681](https://judge.yosupo.jp/submission/376681) | (anonymous) | cpp | 35.0 ms | 35.0 ms | 12046336 B |
| 4 | [401815](https://judge.yosupo.jp/submission/401815) | toomer | rust | 44.0 ms | 44.0 ms | 10174464 B |
| 5 | [391951](https://judge.yosupo.jp/submission/391951) | firiexp | cpp | 80.0 ms | 80.0 ms | 6012928 B |
| 6 | [208756](https://judge.yosupo.jp/submission/208756) | oldyan | cpp | 82.0 ms | 82.0 ms | 5464064 B |
| 7 | [117672](https://judge.yosupo.jp/submission/117672) | TumoiYorozu | cpp | 83.0 ms | 83.0 ms | 6955008 B |
| 8 | [272332](https://judge.yosupo.jp/submission/272332) | Richard1211 | cpp | 90.0 ms | 90.0 ms | 6520832 B |
| 9 | [88941](https://judge.yosupo.jp/submission/88941) | nor | cpp | 91.0 ms | 91.0 ms | 8413184 B |
| 10 | [401375](https://judge.yosupo.jp/submission/401375) | Rohan_Kapri | cpp | 97.0 ms | 97.0 ms | 7847936 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 403008 | 3283 | 141292 | no | yes | no | no | yes |
| 393928 | 1354 | 49003 | no | yes | no | no | yes |
| 376681 | 135 | 6505 | no | no | yes | no | yes |
| 401815 | 41 | 104579 | no | yes | no | no | yes |
| 391951 | 916 | 37341 | no | yes | no | no | yes |
| 208756 | 609 | 32363 | no | yes | no | no | yes |
| 117672 | 1967 | 72259 | no | yes | yes | no | yes |
| 272332 | 1484 | 101103 | no | yes | yes | no | yes |
| 88941 | 1725 | 63040 | no | yes | yes | no | yes |
| 401375 | 2071 | 80100 | no | yes | yes | no | yes |
