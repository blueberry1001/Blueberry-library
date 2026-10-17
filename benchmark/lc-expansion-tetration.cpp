// Independent pre-optimization baselines; no external implementation is copied.
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "blueberry/math/tetration-mod.hpp"
#include "blueberry/utility/fast-io.hpp"

using u32 = std::uint32_t;
using u64 = std::uint64_t;

u32 trial_phi(u32 n) {
  u32 result = n;
  for (u64 p = 2; p * p <= n; ++p) {
    if (n % p) continue;
    result -= result / static_cast<u32>(p);
    while (n % p == 0) n /= static_cast<u32>(p);
  }
  if (n > 1) result -= result / n;
  return result;
}
u64 capped(u64 a, u64 h, u64 cap) {
  if (!cap) return 0;
  if (!h) return 1;
  if (!a) return h % 2 == 0;
  if (a == 1 || cap == 1) return 1;
  u64 need = 0, value = 1;
  while (value < cap) {
    ++need;
    if (value > (cap - 1) / a) { value = cap; break; }
    value *= a;
  }
  const auto e = capped(a, h - 1, need);
  if (e == need) return cap;
  value = 1;
  for (u64 i = 0; i < e; ++i) value *= a;
  return value;
}
u32 baseline(u64 a, u64 h, u32 m, bool use_factorize) {
  if (m == 1) return 0;
  if (!h || a == 1) return 1;
  if (!a) return static_cast<u32>(h % 2 == 0);
  if (h == 1) return static_cast<u32>(a % m);
  const auto phi = use_factorize ? blueberry::tetration_mod_detail::totient(m) : trial_phi(m);
  u64 e = baseline(a, h - 1, phi, use_factorize);
  if (capped(a, h - 1, phi) >= phi) e += phi;
  return blueberry::tetration_mod_detail::pow_mod(a, e, m);
}
u32 gcd_baseline(u64 a, u64 h, u32 m, bool use_factorize) {
  if (m == 1) return 0;
  if (!h || a == 1) return 1;
  if (!a) return static_cast<u32>(h % 2 == 0);
  if (h == 1) return static_cast<u32>(a % m);
  auto e = blueberry::tetration_mod_detail::capped_tower(a, h - 1);
  if (e < 32) return blueberry::tetration_mod_detail::pow_mod(a, e, m);
  u32 n = m, threshold = 0;
  for (auto g = std::gcd(n, static_cast<u32>(a % m)); g != 1; g = std::gcd(n, g))
    n /= g, ++threshold;
  if (n == 1) return 0;
  const auto phi = use_factorize ? blueberry::tetration_mod_detail::totient(n) : trial_phi(n);
  u64 exponent = gcd_baseline(a, h - 1, phi, use_factorize);
  if (exponent < threshold) exponent += ((threshold - exponent + phi - 1) / phi) * phi;
  return blueberry::tetration_mod_detail::pow_mod(a, exponent, m);
}
int main(int argc, char** argv) {
  using Clock = std::chrono::steady_clock;
  struct Query { u64 a, h; u32 m, answer; };
  const int mode = argc > 1 ? std::atoi(argv[1]) : 4;
  const auto start = Clock::now();
  blueberry::FastInput in;
  int count = 0;
  if (!in.read(count)) return 0;
  std::vector<Query> queries(static_cast<std::size_t>(count));
  for (auto& q : queries) in.read(q.a, q.h, q.m);
  const auto parsed = Clock::now();
  for (auto& q : queries) {
    if (mode < 2) q.answer = baseline(q.a, q.h, q.m, mode == 1);
    else if (mode < 4) q.answer = gcd_baseline(q.a, q.h, q.m, mode == 2);
    else q.answer = blueberry::tetration_mod(q.a, q.h, q.m);
  }
  const auto operated = Clock::now();
  blueberry::FastOutput out;
  for (const auto& q : queries) out.writeln(q.answer);
  out.flush();
  const auto formatted = Clock::now();
  auto seconds = [](auto begin, auto end) { return std::chrono::duration<double>(end - begin).count(); };
  std::fprintf(stderr, "{\"parse\":%.9f,\"core\":%.9f,\"format\":%.9f,\"queries_bytes\":%zu}\n",
               seconds(start, parsed), seconds(parsed, operated), seconds(operated, formatted),
               queries.capacity() * sizeof(Query));
}
