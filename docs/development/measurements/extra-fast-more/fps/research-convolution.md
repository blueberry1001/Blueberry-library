---
title: "Library Checker fastest: `convolution_mod`"
---

# Library Checker fastest: `convolution_mod`

REST endpoint: `https://v3.api.judge.yosupo.jp/submissions`
Times are the API's maximum testcase time, converted from seconds to milliseconds.
The rows are a moving public leaderboard; compare only with the same problem, compiler, flags and machine.

| rank | submission | user | language | max time | max case | memory |
| ---: | ---: | --- | --- | ---: | ---: | ---: |
| 1 | [409465](https://judge.yosupo.jp/submission/409465) | Aiyiyi | cpp | 12.0 ms | 12.0 ms | 21602304 B |
| 2 | [409433](https://judge.yosupo.jp/submission/409433) | Aiyiyi | cpp | 13.0 ms | 13.0 ms | 21667840 B |
| 3 | [409432](https://judge.yosupo.jp/submission/409432) | Aiyiyi | cpp | 13.0 ms | 13.0 ms | 21864448 B |
| 4 | [409431](https://judge.yosupo.jp/submission/409431) | Aiyiyi | cpp | 13.0 ms | 13.0 ms | 22028288 B |

## Source heuristics

These flags are intentionally shallow indicators for deciding what to benchmark locally; they are not proof that one implementation is faster.

| submission | lines | bytes | NTT | SIMD | target pragma | ACL convolution | fast I/O |
| ---: | ---: | ---: | :---: | :---: | :---: | :---: | :---: |
| 409465 | 5818 | 269855 | yes | yes | no | no | yes |
| 409433 | 5533 | 255748 | yes | yes | no | no | yes |
| 409432 | 5533 | 255748 | yes | yes | no | no | yes |
| 409431 | 5533 | 255748 | yes | yes | no | no | yes |
