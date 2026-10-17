# Library Checker fastest: `primitive_root`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [138249](https://judge.yosupo.jp/submission/138249) | mizar | rust | 1.0 ms | 1.0 ms | 724992 B |
| 2 | [410843](https://judge.yosupo.jp/submission/410843) | a1073741824 | cpp | 2.0 ms | 2.0 ms | 851968 B |
| 3 | [398661](https://judge.yosupo.jp/submission/398661) | Today03 | cpp | 2.0 ms | 2.0 ms | 888832 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 138249 | 154 | 90604 | no | yes | no | no | yes |
| 410843 | 1805 | 63595 | no | yes | no | no | yes |
| 398661 | 1755 | 48385 | no | yes | no | no | yes |
