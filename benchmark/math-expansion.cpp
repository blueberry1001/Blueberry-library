// Independently written comparison candidates; no public submission source copied.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/factorize.hpp"
#include "blueberry/math/linear-recurrence.hpp"
#include "blueberry/math/mod-sqrt.hpp"

using u64 = std::uint64_t;
using mint = atcoder::modint998244353;

u64 multiply(u64 a, u64 b, u64 p) {
  return static_cast<u64>(static_cast<unsigned __int128>(a) * b % p);
}

std::vector<u64> floyd_factorize(u64 n) {
  std::vector<u64> result, pending{n};
  std::mt19937_64 rng(n);
  while (!pending.empty()) {
    n = pending.back(); pending.pop_back();
    if (n == 1) continue;
    if (blueberry::is_prime(n)) { result.push_back(n); continue; }
    if (n % 2 == 0) { result.push_back(2); pending.push_back(n / 2); continue; }
    u64 g = n;
    while (g == n || g == 1) {
      const u64 c = rng() % (n - 1) + 1;
      auto step = [&](u64 x) {
        const u64 product = multiply(x, x, n);
        return product >= n - c ? product - (n - c) : product + c;
      };
      u64 x = rng() % n, y = x;
      for (int i = 0; i < 1000000; ++i) {
        x = step(x); y = step(step(y));
        g = std::gcd(x > y ? x - y : y - x, n);
        if (g != 1) break;
      }
    }
    pending.push_back(g); pending.push_back(n / g);
  }
  std::sort(result.begin(), result.end());
  return result;
}

std::int64_t cipolla(std::int64_t a, std::int64_t p) {
  if (a == 0 || p == 2) return a;
  auto power = [p](std::int64_t x, std::int64_t e) {
    std::int64_t ans = 1;
    for (; e; e >>= 1, x = x * x % p) if (e & 1) ans = ans * x % p;
    return ans;
  };
  if (power(a, (p - 1) / 2) != 1) return -1;
  std::int64_t t = 0, w = 0;
  for (;;) {
    w = (t * t % p + p - a) % p;
    if (w == 0) return std::min(t, p - t);
    if (power(w, (p - 1) / 2) == p - 1) break;
    ++t;
  }
  using pair = std::pair<std::int64_t, std::int64_t>;
  auto product = [p, w](pair x, pair y) {
    return pair{(x.first * y.first % p + x.second * y.second % p * w) % p,
                (x.first * y.second + x.second * y.first) % p};
  };
  pair x{t, 1}, r{1, 0};
  for (std::int64_t e = (p + 1) / 2; e; e >>= 1, x = product(x, x))
    if (e & 1) r = product(r, x);
  return std::min(r.first, p - r.first);
}

mint kitamasa(const std::vector<mint>& a, const std::vector<mint>& c, u64 k) {
  const int n = static_cast<int>(a.size());
  auto product = [&](const std::vector<mint>& x, const std::vector<mint>& y) {
    std::vector<mint> r(2 * n - 1);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j) r[i + j] += x[i] * y[j];
    for (int i = 2 * n - 2; i >= n; --i)
      for (int j = 0; j < n; ++j) r[i - j - 1] += r[i] * c[j];
    r.resize(n); return r;
  };
  std::vector<mint> x(n), r(n); r[0] = 1;
  if (n == 1) x[0] = c[0]; else x[1] = 1;
  for (; k; k >>= 1, x = product(x, x)) if (k & 1) r = product(r, x);
  mint answer = 0;
  for (int i = 0; i < n; ++i) answer += r[i] * a[i];
  return answer;
}

template <class Function>
u64 timed(const std::string& name, const std::string& variant, int repeat, Function function) {
  const auto start = std::chrono::steady_clock::now();
  const u64 checksum = function();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now() - start).count();
  std::cout << name << '\t' << variant << '\t' << repeat << '\t' << ns << '\t' << checksum << '\n';
  return checksum;
}

int main() {
  std::mt19937_64 rng(20260917);
  std::vector<u64> numbers;
  while (numbers.size() < 60) {
    u64 p = 1000000 + rng() % 1000000, q = 1000000 + rng() % 1000000;
    while (!blueberry::is_prime(p)) ++p;
    while (!blueberry::is_prime(q)) ++q;
    numbers.push_back(p * q);
  }
  for (u64 n : numbers)
    if (floyd_factorize(n) != blueberry::factorize(n)) return 1;
  std::vector<std::pair<std::int64_t, std::int64_t>> squares;
  for (int i = 0; i < 4000; ++i) {
    const std::int64_t p = i % 2 ? 998244353 : 1000000007;
    squares.emplace_back(static_cast<std::int64_t>(rng() % p), p);
  }
  for (auto [a, p] : squares)
    if (cipolla(a, p) != blueberry::mod_sqrt(a, p).value_or(-1)) return 2;
  std::vector<mint> a(128), c(128), prefix(10000);
  for (auto& x : a) x = rng();
  for (auto& x : c) x = rng();
  for (auto& x : prefix) x = rng();
  constexpr u64 k = 1000000000000000000ULL;
  if (kitamasa(a, c, k) != blueberry::linear_recurrence_kth(a, c, k)) return 3;
  // repeat -1 is a warmup. Comparison variants alternate order on each repeat.
  for (int repeat = -1; repeat < 5; ++repeat) {
    for (int variant = 0; variant < 2; ++variant) {
      const bool baseline = (variant + repeat + 1) % 2;
      timed("factor-semiprime-60", baseline ? "floyd" : "brent", repeat, [&] {
        u64 checksum = 0;
        for (u64 n : numbers) {
          const auto f = baseline ? floyd_factorize(n) : blueberry::factorize(n);
          for (auto p : f) checksum += p;
        }
        return checksum;
      });
      timed("sqrt-mixed-4000", baseline ? "cipolla" : "tonelli", repeat, [&] {
        u64 checksum = 0;
        for (auto [value, p] : squares)
          checksum += baseline ? cipolla(value, p) : blueberry::mod_sqrt(value, p).value_or(-1);
        return checksum;
      });
      timed("recurrence-d128-k1e18", baseline ? "kitamasa" : "bostan-mori", repeat, [&] {
        return static_cast<u64>((baseline ? kitamasa(a, c, k) : blueberry::linear_recurrence_kth(a, c, k)).val());
      });
    }
    timed("bm-random-n10000", "quadratic", repeat, [&] {
      const auto coefficients = blueberry::berlekamp_massey(prefix);
      return static_cast<u64>(coefficients.size()) + coefficients.back().val();
    });
  }
}
