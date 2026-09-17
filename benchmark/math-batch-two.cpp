#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include "blueberry/math/discrete-log.hpp"
#include "blueberry/math/divisor-convolution.hpp"
#include "blueberry/math/enumerate-quotients.hpp"
#include "blueberry/math/kth-root-integer.hpp"

// Independently written comparison candidates; no imported implementation.
std::uint64_t bit_root(std::uint64_t n, int k) {
  if (k == 1) return n;
  std::uint64_t r = 0;
  for (int b = 31; b >= 0; --b) {
    auto x = r | (std::uint64_t{1} << b);
    __uint128_t p = 1;
    for (int j = 0; j < k && p <= n; ++j) p *= x;
    if (p <= n) r = x;
  }
  return r;
}
std::vector<std::int64_t> jump_quotients(std::int64_t n) {
  std::vector<std::int64_t> a;
  for (std::int64_t i = 1; i <= n;) {
    auto q = n / i;
    a.push_back(q);
    auto end = n / q;
    if (end == n) break;
    i = end + 1;
  }
  std::reverse(a.begin(), a.end());
  return a;
}
std::int64_t ordered_log(std::int64_t a, std::int64_t b, std::int64_t m) {
  // Prime benchmark inputs: coprime BSGS using a deterministic ordered map.
  std::int64_t width = std::sqrt(m) + 1, power = 1;
  std::map<std::int64_t, std::int64_t> table;
  for (std::int64_t j = 0; j < width; ++j) {
    table.emplace(power, j); power = power * a % m;
  }
  auto inv = atcoder::inv_mod(power, m);
  for (std::int64_t i = 0; i <= width; ++i, b = b * inv % m) {
    auto it = table.find(b);
    if (it != table.end()) return i * width + it->second;
  }
  return -1;
}
template <class F> void measure(const char* name, F run) {
  std::vector<double> ms;
  std::uint64_t checksum = 0;
  for (int i = 0; i < 5; ++i) {
    auto start = std::chrono::steady_clock::now();
    checksum += run();
    ms.push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
  }
  std::cout << name << " runs_ms=";
  for (auto x : ms) std::cout << x << ',';
  std::sort(ms.begin(), ms.end());
  std::cout << " median_ms=" << ms[2] << " min_ms=" << ms[0] << " checksum=" << checksum << '\n';
}
int main() {
  std::mt19937_64 rng(20260918);
  std::vector<std::uint64_t> input(20000);
  for (auto& x : input) x = rng();
  auto roots = [&](auto f) { std::uint64_t s = 0; for (int i = 0; i < 20000; ++i) s += f(input[i], 2 + i % 63); return s; };
  measure("root/bounded_binary", [&] { return roots(blueberry::kth_root_integer); });
  measure("root/32_bit_trials", [&] { return roots(bit_root); });
  measure("quotients/sqrt_split", [&] { auto a = blueberry::enumerate_quotients(10000000000LL); return a.size() + a.back(); });
  measure("quotients/interval_jump", [&] { auto a = jump_quotients(10000000000LL); return a.size() + a.back(); });
  std::vector<std::uint64_t> a(1000001);
  for (auto& x : a) x = rng() % 100;
  measure("divisor/prime_sieve", [&] { auto b = a; blueberry::divisor_zeta(b); return std::accumulate(b.begin(), b.end(), std::uint64_t{}); });
  measure("divisor/harmonic", [&] { auto b = a; for (std::size_t i = a.size() - 1; i >= 1; --i) for (std::size_t j = i + i; j < a.size(); j += i) b[j] += b[i]; return std::accumulate(b.begin(), b.end(), std::uint64_t{}); });
  measure("log/hash", [&] { std::uint64_t s = 0; for (int i = 0; i < 10; ++i) s += blueberry::discrete_log(3, 1234567 + i, 1000000007).value_or(-1); return s; });
  measure("log/ordered_map", [&] { std::uint64_t s = 0; for (int i = 0; i < 10; ++i) s += ordered_log(3, 1234567 + i, 1000000007); return s; });
}
