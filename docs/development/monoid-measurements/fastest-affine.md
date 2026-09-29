# Library Checker fastest: `range_affine_range_sum`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [402323](https://judge.yosupo.jp/submission/402323) | nandhagk | cpp | 82.0 ms | 82.0 ms | 13762560 B |
| 2 | [402233](https://judge.yosupo.jp/submission/402233) | nandhagk | cpp | 90.0 ms | 90.0 ms | 13770752 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 402323 | 3436 | 158996 | no | yes | no | no | no |
| 402233 | 1346 | 61384 | yes | yes | no | no | no |
