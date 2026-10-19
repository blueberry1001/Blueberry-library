#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include "blueberry/math/sparse-exp.hpp"
#include "blueberry/math/sparse-exp.hpp"

using namespace std;

unsigned long long seed;

// Expand sum_k f^k / k! with naive polynomial products.  This does not use
// either the sparse differential recurrence or the dense FPS implementation.
template <class Mint>
vector<Mint> brute_exp(const vector<Mint>& input) {
  const int n = static_cast<int>(input.size());
  vector<Mint> result(n), power(n);
  if (n == 0) return result;
  result[0] = power[0] = 1;
  Mint inverse_factorial = 1;
  for (int k = 1; k < n; ++k) {
    vector<Mint> next(n);
    for (int i = 0; i < n; ++i)
      for (int j = 1; i + j < n; ++j) next[i + j] += power[i] * input[j];
    power = std::move(next);
    inverse_factorial /= Mint(k);
    for (int i = 0; i < n; ++i) result[i] += power[i] * inverse_factorial;
  }
  return result;
}

template <class Mint>
void check(const char* operation, const vector<pair<int, Mint>>& terms, int n,
           const vector<Mint>& expected, const vector<Mint>& actual) {
  if (expected == actual) return;
  cerr << "seed=" << seed << " mod=" << Mint::mod() << " operation=" << operation
       << " n=" << n << " terms=[";
  for (const auto& [degree, value] : terms) cerr << '(' << degree << ',' << value.val() << "),";
  cerr << ']';
  for (const auto& [label, values] : {pair{" expected=", expected}, pair{" actual=", actual}}) {
    cerr << label << '[';
    for (Mint value : values) cerr << value.val() << ',';
    cerr << ']';
  }
  cerr << '\n';
  exit(1);
}

template <class Mint>
void verify(const vector<pair<int, Mint>>& terms, int n, bool brute = true) {
  vector<Mint> dense(n);
  for (const auto& [degree, value] : terms)
    if (degree < n) dense[degree] += value;
  const auto saved = terms;
  const auto actual = blueberry::sparse_exp<Mint>(terms, n);
  // Vector equality also checks exact precision, including trailing zeros.
  if (brute) check("power-series expansion", terms, n, brute_exp(dense), actual);
  const auto dense_result = blueberry::FPS<Mint>(dense.begin(), dense.end()).exp(n);
  check("dense FPS exp", terms, n, vector<Mint>(dense_result.begin(), dense_result.end()), actual);
  if (terms != saved) {
    cerr << "seed=" << seed << " mod=" << Mint::mod() << " input was modified\n";
    exit(1);
  }
}

template <class Mint>
void run(mt19937_64& rng, int max_n) {
  // Empty input, explicit zeros, cancelling constants and ignored huge degrees.
  for (int n = 0; n <= max_n; ++n) {
    verify<Mint>({}, n);
    verify<Mint>({{0, 0}, {1, 0}, {n + 5, 0}}, n);
    verify<Mint>({{numeric_limits<int>::max(), 3}, {0, 5}, {n, 7},
                  {0, -5}, {n, -7}, {1, 2}, {1, -2}}, n);
  }
  // Each possible surviving monomial checks gaps and factorial denominators.
  for (int degree = 1; degree < max_n; ++degree)
    verify<Mint>({{degree, -3}}, max_n);

  for (int test = 0; test < 180; ++test) {
    const int n = static_cast<int>(rng() % (max_n + 1));
    vector<pair<int, Mint>> terms;
    const Mint constant(rng() % Mint::mod());
    terms.emplace_back(0, constant);
    terms.emplace_back(0, -constant);
    const int count = static_cast<int>(rng() % 50);
    for (int i = 0; i < count; ++i) {
      const int degree = 1 + static_cast<int>(rng() % (max_n + 10));
      const Mint value(rng() % Mint::mod());
      terms.emplace_back(degree, value);
      if (rng() % 3 == 0) terms.emplace_back(degree, -value);
      if (rng() % 4 == 0) terms.emplace_back(degree, Mint(rng() % Mint::mod()));
    }
    shuffle(terms.begin(), terms.end(), rng);
    verify(terms, n);
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  using mint = atcoder::modint998244353;
  run<mint>(rng, 40);
  run<atcoder::static_modint<17>>(rng, 16);
  run<atcoder::static_modint<2>>(rng, 1);

  // Non-power-of-two precisions around larger inverse-table boundaries.
  for (int n : {63, 64, 65, 127, 128, 129, 255, 256, 257}) {
    vector<pair<int, mint>> terms{{0, 1}, {0, -1}, {1, 2}, {3, 5}, {3, -2},
                                  {n - 1, 7}, {n, 11}, {n + 20, 13}};
    shuffle(terms.begin(), terms.end(), rng);
    verify(terms, n, false);
  }
}
