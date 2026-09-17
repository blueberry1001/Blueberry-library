#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include "blueberry/math/fraction.hpp"

using boost::multiprecision::cpp_int;
using blueberry::Fraction;
using i64 = std::int64_t;
std::uint64_t seed;

// Independent unbounded integer oracle; never narrows intermediates.
struct Rational {
  cpp_int n, d;
  Rational(cpp_int a, cpp_int b) : n(a), d(b) {
    cpp_int x = a < 0 ? -a : a, y = b < 0 ? -b : b;
    while (y != 0) {
      cpp_int r = x % y;
      x = y;
      y = r;
    }
    if (d < 0) x = -x;
    n /= x;
    d /= x;
  }
  bool fits() const {
    return n >= std::numeric_limits<i64>::min() &&
           n <= std::numeric_limits<i64>::max() &&
           d <= std::numeric_limits<i64>::max();
  }
};

void check(const Fraction& actual, const Rational& expected, const char* operation,
           const Fraction& a = {}, const Fraction& b = {}) {
  if (cpp_int(actual.num()) == expected.n && cpp_int(actual.den()) == expected.d) return;
  std::cerr << "seed=" << seed << " op=" << operation << " a=" << a << " b=" << b
            << " expected=" << expected.n << '/' << expected.d << " actual=" << actual << '\n';
  std::exit(1);
}

void test_pair(Fraction a, Fraction b) {
  const Rational x(a.num(), a.den()), y(b.num(), b.den());
  const cpp_int left = x.n * y.d, right = y.n * x.d;
  if ((a == b) != (left == right) || (a != b) != (left != right) ||
      (a < b) != (left < right) || (a <= b) != (left <= right) ||
      (a > b) != (left > right) || (a >= b) != (left >= right)) {
    std::cerr << "seed=" << seed << " comparison a=" << a << " b=" << b
              << " expected cross=" << left << "," << right << '\n';
    std::exit(1);
  }
  check(+a, x, "unary+", a);
  const Rational negative(-x.n, x.d);
  if (negative.fits()) check(-a, negative, "unary-", a);
  const Rational sum(left + right, x.d * y.d);
  const Rational difference(left - right, x.d * y.d);
  const Rational product(x.n * y.n, x.d * y.d);
  if (sum.fits()) {
    check(a + b, sum, "+", a, b);
    auto c = a;
    check(c += b, sum, "+=", a, b);
  }
  if (difference.fits()) {
    check(a - b, difference, "-", a, b);
    auto c = a;
    check(c -= b, difference, "-=", a, b);
  }
  if (product.fits()) {
    check(a * b, product, "*", a, b);
    auto c = a;
    check(c *= b, product, "*=", a, b);
  }
  if (b.num()) {
    const Rational quotient(x.n * y.d, x.d * y.n);
    if (quotient.fits()) {
      check(a / b, quotient, "/", a, b);
      auto c = a;
      check(c /= b, quotient, "/=", a, b);
    }
  }
  auto c = a;
  check(c -= c, Rational(0, 1), "self-=", a);
  c = a;
  if (a.num()) check(c /= c, Rational(1, 1), "self/=", a);
  auto copied = a;
  auto moved = std::move(copied);
  check(moved, x, "move", a);
  check(copied, x, "moved-from", a);
  c = std::move(moved);
  check(c, x, "move=", a);
  std::ostringstream out;
  out << a;
  std::string expected = x.n.convert_to<std::string>();
  if (x.d != 1) expected += '/' + x.d.convert_to<std::string>();
  if (out.str() != expected) {
    std::cerr << "seed=" << seed << " stream expected=" << expected << " actual=" << out.str() << '\n';
    std::exit(1);
  }
}

int main(int argc, char** argv) {
  const char* env = std::getenv("BLUEBERRY_RANDOM_SEED");
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : env ? std::strtoull(env, nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  const i64 lo = std::numeric_limits<i64>::min(), hi = std::numeric_limits<i64>::max();
  std::vector<i64> endpoints{lo, lo + 1, lo + 2, -1000000000, -2, -1, 0, 1, 2, 1000000000, hi - 1, hi};
  std::vector<Fraction> boundary;
  for (i64 n : endpoints) for (i64 d : endpoints) if (d) {
    Rational expected(n, d);
    if (expected.fits()) {
      Fraction f(n, d);
      check(f, expected, "ctor");
      boundary.push_back(f);
    }
  }
  for (const auto& a : boundary) for (const auto& b : boundary) test_pair(a, b);
  check(Fraction(), Rational(0, 1), "default");
  check(Fraction(lo), Rational(lo, 1), "integer");
  for (int trial = 0; trial < 8000; ++trial) {
    i64 n = std::bit_cast<i64>(rng()), d = std::bit_cast<i64>(rng());
    i64 p = std::bit_cast<i64>(rng()), q = std::bit_cast<i64>(rng());
    if (trial % 3 == 0) {
      n %= 10000; d %= 10000; p %= 10000; q %= 10000;
    } else if (trial % 3 == 1) {
      // Equal denominators and reciprocals exercise large cancellation.
      d = q = 1 + static_cast<i64>(rng() % 1000000000);
      p = n == lo ? n + 1 : -n;
    }
    if (!d || !q) continue;
    Rational x(n, d), y(p, q);
    if (!x.fits() || !y.fits()) continue;
    const Fraction a(n, d), b(p, q);
    check(a, x, "random ctor");
    check(b, y, "random ctor");
    test_pair(a, b);
    if (n && Rational(d, n).fits()) test_pair(a, Fraction(d, n));
  }
  std::cout << "fraction seed=" << seed << " passed\n";
}
