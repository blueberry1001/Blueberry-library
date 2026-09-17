#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <boost/rational.hpp>
#include "blueberry/math/fraction.hpp"
#ifdef BLUEBERRY_FRACTION_BEFORE_HEADER
#include BLUEBERRY_FRACTION_BEFORE_HEADER
#endif

using i64 = std::int64_t;
using u64 = std::uint64_t;
struct Input { i64 a, b, c, d; };
template <class Rational> i64 numerator(const Rational& x) {
  if constexpr (requires { x.num(); }) return x.num();
  else return x.numerator();
}
template <class Rational> i64 denominator(const Rational& x) {
  if constexpr (requires { x.den(); }) return x.den();
  else return x.denominator();
}

template <class Rational>
u64 kernel(const std::vector<Input>& input, const std::string& operation) {
  u64 answer = 0;
  for (const auto& row : input) {
    Rational a(row.a, row.b), b(row.c, row.d);
    if (operation == "construct-compare") {
      answer += u64(numerator(a)) * 0x9e3779b97f4a7c15ULL + denominator(a) + (a < b);
    } else {
      const Rational results[] = {a + b, a - b, a * b, a / b};
      for (const auto& result : results)
        answer += u64(numerator(result)) * 0x9e3779b97f4a7c15ULL + denominator(result);
    }
  }
  return answer;
}

int main(int argc, char** argv) {
  if (argc != 3) return 2;
  const std::string variant = argv[1], operation = argv[2];
  if ((variant != "blueberry" && variant != "boost"
#ifdef BLUEBERRY_FRACTION_BEFORE_HEADER
       && variant != "before"
#endif
       ) ||
      (operation != "construct-compare" && operation != "arithmetic")) return 2;
  std::mt19937_64 rng(20260917);
  std::vector<Input> input(300000);
  for (auto& row : input) {
    row = {static_cast<i64>(rng() % 2000001) - 1000000,
           1 + static_cast<i64>(rng() % 1000000),
           1 + static_cast<i64>(rng() % 1000000),
           1 + static_cast<i64>(rng() % 1000000)};
  }
  // Input generation is excluded; construction and all rational operations included.
  const auto start = std::chrono::steady_clock::now();
  u64 checksum;
  if (variant == "blueberry") checksum = kernel<blueberry::Fraction>(input, operation);
#ifdef BLUEBERRY_FRACTION_BEFORE_HEADER
  else if (variant == "before") checksum = kernel<blueberry_before::Fraction>(input, operation);
#endif
  else checksum = kernel<boost::rational<i64>>(input, operation);
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now() - start).count();
  std::cout << ns << '\t' << checksum << '\n';
}
