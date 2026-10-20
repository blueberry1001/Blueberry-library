---
title: "Library Checker fastest: `inv_of_formal_power_series`"
---

# Library Checker fastest: `inv_of_formal_power_series`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409370](https://judge.yosupo.jp/submission/409370) | Aiyiyi | cpp | 11.0 ms | 11.0 ms | 12091392 B |
| 2 | [409242](https://judge.yosupo.jp/submission/409242) | Aiyiyi | cpp | 13.0 ms | 13.0 ms | 15581184 B |
| 3 | [409369](https://judge.yosupo.jp/submission/409369) | Aiyiyi | cpp | 18.0 ms | 18.0 ms | 11927552 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409370 | 5674 | 261512 | yes | yes | no | no | yes |
| 409242 | 3715 | 171054 | yes | yes | no | no | yes |
| 409369 | 5674 | 261512 | yes | yes | no | no | yes |
