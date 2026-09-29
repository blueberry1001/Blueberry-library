# Library Checker fastest: `point_set_range_composite`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [402223](https://judge.yosupo.jp/submission/402223) | nandhagk | cpp | 49.0 ms | 49.0 ms | 18616320 B |
| 2 | [402225](https://judge.yosupo.jp/submission/402225) | nandhagk | cpp | 50.0 ms | 50.0 ms | 18628608 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 402223 | 765 | 34495 | no | yes | no | no | no |
| 402225 | 1172 | 54997 | no | yes | no | no | no |
