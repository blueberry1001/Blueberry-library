# Library Checker fastest: `bell_number`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [364958](https://judge.yosupo.jp/submission/364958) | urectanc | rust | 64.0 ms | 64.0 ms | 21299200 B |
| 2 | [406022](https://judge.yosupo.jp/submission/406022) | Rohan_Kapri | cpp | 97.0 ms | 97.0 ms | 54239232 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 364958 | 1371 | 59682 | yes | yes | no | no | yes |
| 406022 | 2146 | 87579 | yes | yes | yes | no | no |
