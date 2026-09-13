# Random tests

Place reusable randomized comparison programs under this directory as `*.cpp`.
`make random-test` compiles every program with the same strict flags as the
compile checks, then runs it repeatedly.

`runner-smoke.cpp` is an infrastructure check for seed delivery; algorithm
tests can be added alongside it without changing the runner or CI workflow.

Each program receives the seed in both `argv[1]` and the
`BLUEBERRY_RANDOM_SEED` environment variable.  It should return zero on success
and print enough context to reproduce a mismatch before returning non-zero.

```cpp
#include <cstdlib>
#include <random>

int main(int argc, char** argv) {
  const auto seed = std::strtoull(argv[1], nullptr, 10);
  std::mt19937_64 random(seed);
  // Compare the library implementation with a simple reference implementation.
  return 0;
}
```

Run one failing seed again with:

```console
RANDOM_SEED=123 RANDOM_RUNS=1 make random-test
```
