---
title: Prime Sieve
documentation_of: //blueberry/math/prime-sieve.hpp
---

Eratosthenesの篩により、`limit` 以下の素数を列挙する。

## 計算量

- 構築: $O(N\log\log N)$
- `is_prime`: $O(1)$
- メモリ: $O(N)$ bit

`primes()` は昇順の素数列へのconst参照を返す。
