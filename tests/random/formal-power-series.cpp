#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;
using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

unsigned long long seed;

void check(const char* operation, const fps& input, const fps& expected, const fps& actual) {
  if (expected == actual) return;
  cerr << "seed=" << seed << " operation=" << operation;
  for (const auto& [label, values] : {pair{" input=", input}, pair{" expected=", expected},
                                    pair{" actual=", actual}}) {
    cerr << label << '[';
    for (mint value : values) cerr << value.val() << ',';
    cerr << ']';
  }
  cerr << '\n';
  exit(1);
}

fps multiply(const fps& a, const fps& b, int n) {
  fps c(n);
  for (int i = 0; i < min<int>(a.size(), n); ++i)
    for (int j = 0; j < static_cast<int>(b.size()) && i + j < n; ++j)
      c[i + j] += a[i] * b[j];
  return c;
}

fps inverse(const fps& a, int n) {
  fps b(n);
  if (n == 0) return b;
  b[0] = a[0].inv();
  for (int i = 1; i < n; ++i) {
    for (int j = 1; j <= i && j < static_cast<int>(a.size()); ++j)
      b[i] -= a[j] * b[i - j];
    b[i] *= b[0];
  }
  return b;
}

pair<fps, fps> divide(fps a, fps b) {
  b.shrink();
  const int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
  fps q(max(0, n - m + 1));
  for (int i = n - m; i >= 0; --i) {
    q[i] = a[i + m - 1] / b.back();
    for (int j = 0; j < m; ++j) a[i + j] -= q[i] * b[j];
  }
  a.shrink();
  return {q, a};
}

int main(int argc, char** argv) {
  seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);

  // Regressions: scalar aliases, sparse ordering/duplicates, truncated zeros,
  // and reduction of the multiplicative identity modulo a constant.
  fps alias{2, 3};
  alias *= alias[0];
  check("scalar alias", {2, 3}, {4, 6}, alias);
  fps sparse_input{2, 4, 8, 16};
  fps::Sparse sparse{{2, 5}, {0, 3}, {2, -5}, {0, -1}, {1, 2}};
  check("sparse division", sparse_input, {1, 1, 3, 5}, sparse_input / sparse);
  check("sqrt truncated zero", {0, 1}, {0}, fps{0, 1}.sqrt(1));
  check("sqrt ignored nonresidue", {0, 0, 3}, {0, 0}, fps{0, 0, 3}.sqrt(2));
  check("sqrt_with truncated zero", {0, 1}, {0},
        fps{0, 1}.sqrt_with([](mint a) { return a; }, 1));
  check("sqrt zero precision", {1}, {}, fps{1}.sqrt(0));
  check("sqrt_with zero precision", {1}, {}, fps{1}.sqrt_with([](mint a) { return a; }, 0));
  check("sqrt zero skips callback", {}, {0, 0}, fps{}.sqrt_with([](mint) -> mint {
    abort();
  }, 2));
  check("mod_pow constant", {1, 2}, {}, fps{1, 2}.mod_pow(0, fps{2}));
  check("sqrt odd valuation", {0, 1}, {}, fps{0, 1}.sqrt());
  check("sqrt nonresidue", {3}, {}, fps{3}.sqrt());
  check("empty exp", {}, {1, 0, 0}, fps{}.exp(3));
  check("zero power", {}, {1, 0, 0}, fps{}.pow(0, 3));
  check("padded divisor", {1, 2, 1}, {1, 1}, fps{1, 2, 1} / fps{1, 1, 0});
  check("empty dividend", {}, {}, fps{} / fps{1});
  check("empty sparse multiplication", {1, 2}, {}, fps{1, 2} * fps::Sparse{});
  check("empty sparse division", {}, {}, fps{} / fps::Sparse{{0, 1}});

  // Include both sides of the quadratic/NTT inverse threshold and power-of-two
  // transform boundaries; compare against independent quadratic recurrences.
  for (int n : {0, 1, 2, 3, 7, 16, 31, 32, 63, 64, 65, 127, 128, 129}) {
    for (int trial = 0; trial < 4; ++trial) {
      fps a(max(1, n)), b(max(1, n / 2 + 1));
      for (mint& x : a) x = rng() % 100;
      for (mint& x : b) x = rng() % 100;
      a[0] = 1 + rng() % 100;
      b.back() = 1 + rng() % 100;
      check("inverse", a, inverse(a, n), a.inv(n));
      check("dense multiply", a, multiply(a, b, a.size() + b.size() - 1), a * b);
      const auto expected_division = divide(a, b);
      const auto actual_division = a.div_mod(b);
      check("polynomial quotient", a, expected_division.first, actual_division.first);
      check("polynomial remainder", a, expected_division.second, actual_division.second);

      fps::Sparse terms{{0, 2}, {2, 3}, {0, 4}, {1, 5}, {2, -1}};
      shuffle(terms.begin(), terms.end(), rng);
      check("sparse multiply", a, multiply(a, {6, 5, 2}, a.size()), a * terms);
      check("sparse inverse", a, multiply(a, inverse({6, 5, 2}, a.size()), a.size()), a / terms);

      fps e(n);
      if (n) e[0] = 1;
      a[0] = 0;
      for (int i = 1; i < n; ++i) {
        for (int j = 1; j <= i; ++j) e[i] += mint(j) * a[j] * e[i - j];
        e[i] /= mint(i);
      }
      check("exp", a, e, a.exp(n));
      if (n) check("log", e, a.pre(n), e.log(n));

      if (trial % 2 == 0) a[0] = 1 + rng() % 100;
      const int exponent = rng() % 6;
      fps power(n);
      if (n) power[0] = 1;
      for (int k = 0; k < exponent; ++k) power = multiply(power, a, n);
      check("pow", a, power, a.pow(exponent, n));

      fps root_source = a.pre(n);
      if (n && trial % 2 == 0) root_source[0] = 1;
      const fps square = multiply(root_source, root_source, n);
      const fps root = square.sqrt(n);
      check("sqrt length", square, fps(n), fps(root.size()));
      check("sqrt square", square, square, multiply(root, root, n));
      const fps callback_root = square.sqrt_with([](mint x) {
        // The source coefficients lie in [0,100], so an independent bounded
        // search supplies the callback rather than sharing library internals.
        for (int candidate = 0; candidate <= 100; ++candidate)
          if (mint(candidate) * mint(candidate) == x) return mint(candidate);
        abort();
      }, n);
      check("sqrt_with square", square, square, multiply(callback_root, callback_root, n));

      fps residue{1};
      residue = divide(residue, b).second;
      for (int k = 0; k < exponent; ++k)
        residue = divide(multiply(residue, a, residue.size() + a.size()), b).second;
      check("mod_pow", a, residue, a.mod_pow(exponent, b));
    }
  }
}
