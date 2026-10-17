# Library Checker fastest: `bernoulli_number`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [405758](https://judge.yosupo.jp/submission/405758) | Rohan_Kapri | rust | 38.0 ms | 38.0 ms | 16920576 B |
| 2 | [383856](https://judge.yosupo.jp/submission/383856) | urectanc | rust | 38.0 ms | 38.0 ms | 16830464 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 405758 | 1829 | 78317 | yes | yes | no | no | yes |
| 383856 | 1829 | 78317 | yes | yes | no | no | yes |
