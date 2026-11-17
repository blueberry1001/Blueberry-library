#define main benchmark_main
#include "benchmark.cpp"
#undef main
#include "candidate.hpp"

int main() {
  U checked = 0;
  auto check = [&](U n, int k) {
    const U expected = baseline(n, k);
    for (const auto f : {candidate<0>, candidate<1>, candidate<2>, candidate<3>, integer_sqrt_binary, fixed_bits, bounded_bits<false>, bounded_bits<true>, newton_small<false>, newton_small<true>, blueberry::kth_root_integer}) {
      const U actual = f(n, k);
      if (actual != expected) {
        std::fprintf(stderr, "n=%llu k=%d expected=%llu actual=%llu\n", static_cast<unsigned long long>(n), k, static_cast<unsigned long long>(expected), static_cast<unsigned long long>(actual));
        std::exit(1);
      }
    }
    ++checked;
  };
  for (U n = 0; n < 32768; ++n) for (int k = 1; k <= 70; ++k) check(n,k);
  for (U base = 2; base <= 5000; ++base) {
    U value = 1;
    for (int k = 1; k <= 63; ++k) {
      if (value > std::numeric_limits<U>::max() / base) break;
      value *= base;
      if (k < 2) continue;
      check(value-1,k); check(value,k);
      if (value != std::numeric_limits<U>::max()) check(value+1,k);
    }
  }
  for (U base : {U{65535}, U{65536}, U{1000000}, U{1048575}, U{1048576}, U{2097151}, U{2097152}, U{2147483647}, U{2147483648}, U{4294967294}, U{4294967295}}) {
    U value = 1;
    for (int k = 1; k <= 63; ++k) {
      if (value > std::numeric_limits<U>::max() / base) break;
      value *= base;
      if (k < 2) continue;
      check(value-1,k); check(value,k);
      if (value != std::numeric_limits<U>::max()) check(value+1,k);
    }
  }
  std::mt19937_64 rng(20261118);
  for (int i = 0; i < 200000; ++i) check(rng(),1+int(rng()%70));
  for (int k = 1; k <= 100; ++k) {
    check(std::numeric_limits<U>::max(),k);
    check(std::numeric_limits<U>::max()-1,k);
  }
  std::printf("%llu cases passed\n", static_cast<unsigned long long>(checked));
}
