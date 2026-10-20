# Library Checker fastest: `many_aplusb`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409103](https://judge.yosupo.jp/submission/409103) | Aiyiyi | cpp | 18.0 ms | 18.0 ms | 21012480 B |
| 2 | [402646](https://judge.yosupo.jp/submission/402646) | nandhagk | cpp | 23.0 ms | 23.0 ms | 21012480 B |
| 3 | [407732](https://judge.yosupo.jp/submission/407732) | (anonymous) | cpp | 24.0 ms | 24.0 ms | 20992000 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409103 | 916 | 42714 | no | yes | no | no | yes |
| 402646 | 934 | 40371 | no | yes | no | no | yes |
| 407732 | 3271 | 140812 | no | yes | no | no | yes |
