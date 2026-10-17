#include "blueberry/math/primitive-root.hpp"
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using u64 = std::uint64_t;
using Clock = std::chrono::steady_clock;

std::vector<u64> exponents(u64 p) {
  auto result = blueberry::factorize(p - 1);
  result.erase(std::unique(result.begin(), result.end()), result.end());
  for (auto& q : result) q = (p - 1) / q;
  return result;
}

u64 search(u64 p, const std::vector<u64>& powers, int strategy) {
  if (p == 2) return 1;
  u64 state = p;
  u64 candidate = strategy == 1 ? blueberry::factorize_detail::next_random(state) % (p - 2) + 2 : 2;
  for (;;) {
    bool ok = true;
    // Strategy 3 shares squarings among all distinct prime-divisor checks.
    u64 squares[64];
    if (strategy == 3) {
      squares[0] = candidate;
      for (int i = 1; i < 64 && (powers[0] >> i); ++i)
        squares[i] = blueberry::factorize_detail::mul_mod(squares[i - 1], squares[i - 1], p);
    }
    for (auto exponent : powers) {
      u64 value = 1;
      if (strategy == 3) {
        for (int bit = 0; exponent; exponent >>= 1, ++bit)
          if (exponent & 1) value = blueberry::factorize_detail::mul_mod(value, squares[bit], p);
      } else value = blueberry::factorize_detail::pow_mod(candidate, exponent, p);
      if (value == 1) { ok = false; break; }
    }
    if (ok) return candidate;
    if (strategy == 0) ++candidate;
    else candidate = blueberry::factorize_detail::next_random(state) % (p - 2) + 2;
  }
}

double elapsed(Clock::time_point begin) {
  return std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
}

int main(int argc, char** argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int q;
  std::cin >> q;
  std::vector<u64> primes(q);
  for (auto& p : primes) std::cin >> p;
  const int repeats = argc > 1 ? std::stoi(argv[1]) : 10;
  std::vector<std::vector<u64>> prepared;
  for (u64 p : primes) prepared.push_back(exponents(p));
  u64 checksum = 0;
  auto begin = Clock::now();
  for (int r = 0; r < repeats; ++r) for (u64 p : primes) {
    auto e = exponents(p);
    checksum ^= e.size();
  }
  std::cout << "factorization_ms " << elapsed(begin) << '\n';
  for (int strategy = 0; strategy < 4; ++strategy) {
    begin = Clock::now();
    for (int r = 0; r < repeats; ++r) for (int i = 0; i < q; ++i)
      checksum ^= search(primes[i], prepared[i], strategy);
    std::cout << "search_" << strategy << "_ms " << elapsed(begin) << '\n';
  }
  begin = Clock::now();
  std::vector<u64> roots;
  for (int r = 0; r < repeats; ++r) for (u64 p : primes) {
    auto root = blueberry::primitive_root(p);
    checksum ^= root;
    if (r == 0) roots.push_back(root);
  }
  std::cout << "public_ms " << elapsed(begin) << '\n';
  std::ostringstream data;
  data << q << '\n';
  for (u64 p : primes) data << p << '\n';
  const auto text = data.str();
  begin = Clock::now();
  for (int r = 0; r < repeats; ++r) {
    std::istringstream input(text);
    std::ostringstream output;
    int count;
    input >> count;
    for (int i = 0; i < count; ++i) {
      u64 p;
      input >> p;
      output << roots[i] << '\n';
      checksum ^= p;
    }
    checksum ^= output.str().size();
  }
  std::cout << "parse_format_ms " << elapsed(begin) << '\n';
  std::cout << "queries " << q << " repeats " << repeats << " checksum " << checksum << '\n';
}
