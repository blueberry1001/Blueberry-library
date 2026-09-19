# Library Checker fastest: `static_range_lis_query`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [331206](https://judge.yosupo.jp/submission/331206) | fhdqwq | cpp | 164.0 ms | 164.0 ms | 7319552 B |
| 2 | [393940](https://judge.yosupo.jp/submission/393940) | chaihf | cpp | 201.0 ms | 201.0 ms | 27103232 B |
| 3 | [382253](https://judge.yosupo.jp/submission/382253) | t98slider | cpp20 | 209.0 ms | 209.0 ms | 4161536 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 331206 | 371 | 6402 | no | no | no | no | yes |
| 393940 | 1637 | 57048 | no | yes | no | no | yes |
| 382253 | 189 | 7311 | no | no | no | no | yes |
