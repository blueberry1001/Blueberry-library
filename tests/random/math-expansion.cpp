#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include <atcoder/modint>

#include "blueberry/math/factorize.hpp"
#include "blueberry/math/linear-recurrence.hpp"
#include "blueberry/math/mod-sqrt.hpp"

using namespace std;
using u64 = uint64_t;
using mint = atcoder::modint998244353;
u64 seed;
int trial;

void check(bool ok, const char* operation) {
  if (ok) return;
  cerr << "seed=" << seed << " trial=" << trial << " operation=" << operation << '\n';
  exit(1);
}

vector<u64> trial_factors(u64 n) {
  vector<u64> result;
  for (u64 p = 2; p <= n / p; ++p)
    while (n % p == 0) result.push_back(p), n /= p;
  if (n > 1) result.push_back(n);
  return result;
}

// Gaussian consistency, independent of Berlekamp--Massey's update rule.
template <class M>
bool fits_degree(const vector<M>& s, int d) {
  const int rows = static_cast<int>(s.size()) - d;
  vector<vector<M>> a(rows, vector<M>(d + 1));
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < d; ++j) a[i][j] = s[i + d - j - 1];
    a[i][d] = s[i + d];
  }
  int r = 0;
  for (int c = 0; c < d && r < rows; ++c) {
    int p = r;
    while (p < rows && a[p][c] == M(0)) ++p;
    if (p == rows) continue;
    swap(a[p], a[r]);
    const M inv = a[r][c].inv();
    for (int j = c; j <= d; ++j) a[r][j] *= inv;
    for (int i = r + 1; i < rows; ++i) {
      const M scale = a[i][c];
      for (int j = c; j <= d; ++j) a[i][j] -= scale * a[r][j];
    }
    ++r;
  }
  for (int i = r; i < rows; ++i)
    if (a[i][d] != M(0)) return false;
  return true;
}

template <class M>
void check_bm(const vector<M>& s) {
  const auto c = blueberry::berlekamp_massey(s);
  int degree = 0;
  while (!fits_degree(s, degree)) ++degree;
  check(static_cast<int>(c.size()) == degree, "BM minimum degree");
  for (int i = degree; i < static_cast<int>(s.size()); ++i) {
    M value = 0;
    for (int j = 0; j < degree; ++j) value += c[j] * s[i - j - 1];
    check(value == s[i], "BM fits prefix");
  }
}

// Binary exponentiation of x modulo the characteristic polynomial (Kitamasa).
mint reference_kth(const vector<mint>& initial, const vector<mint>& c, u64 k) {
  const int d = static_cast<int>(c.size());
  if (!d) return 0;
  auto product = [&](const vector<mint>& a, const vector<mint>& b) {
    vector<mint> out(2 * d - 1);
    for (int i = 0; i < d; ++i)
      for (int j = 0; j < d; ++j) out[i + j] += a[i] * b[j];
    for (int i = 2 * d - 2; i >= d; --i)
      for (int j = 0; j < d; ++j) out[i - j - 1] += out[i] * c[j];
    out.resize(d);
    return out;
  };
  vector<mint> x(d), result(d);
  result[0] = 1;
  if (d == 1) x[0] = c[0];
  else x[1] = 1;
  for (; k; k >>= 1) {
    if (k & 1) result = product(result, x);
    x = product(x, x);
  }
  mint answer = 0;
  for (int i = 0; i < d; ++i) answer += initial[i] * result[i];
  return answer;
}

int main(int argc, char** argv) {
  seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  check(!blueberry::is_prime(0) && !blueberry::is_prime(1), "prime 0/1");
  check(blueberry::factorize(1).empty(), "factorize one");
  for (u64 n = 2; n < 3000; ++n) {
    trial = static_cast<int>(n);
    const auto expected = trial_factors(n);
    check(blueberry::factorize(n) == expected, "small factorization");
    check(blueberry::is_prime(n) == (expected.size() == 1), "small primality");
  }
  const vector<pair<u64, vector<u64>>> large{
      {18446744073709551557ULL, {18446744073709551557ULL}},
      {numeric_limits<u64>::max(), {3, 5, 17, 257, 641, 65537, 6700417}},
      {u64(1) << 63, vector<u64>(63, 2)},
      {18446744030759878681ULL, {4294967291ULL, 4294967291ULL}},
      {1000000016000000063ULL, {1000000007, 1000000009}}};
  for (const auto& [n, expected] : large)
    check(blueberry::factorize(n) == expected, "uint64 boundary factorization");
  for (u64 n : {341ULL, 561ULL, 3215031751ULL, 341550071728321ULL,
                3825123056546413051ULL})
    check(!blueberry::is_prime(n), "strong pseudoprime");
  for (trial = 0; trial < 100; ++trial) {
    const u64 n = rng() % 100000000 + 1;
    check(blueberry::factorize(n) == trial_factors(n), "random factorization");
  }
  for (int p = 2; p < 150; ++p) {
    if (trial_factors(p).size() != 1) continue;
    for (int a = -p; a < 2 * p; ++a) {
      trial = p * 1000 + a;
      const int value = (a % p + p) % p;
      int expected = -1;
      for (int x = 0; x < p; ++x)
        if (x * x % p == value) { expected = x; break; }
      const auto got = blueberry::mod_sqrt(a, p);
      check(got.has_value() == (expected >= 0), "sqrt existence");
      if (got) check(*got == expected, "sqrt minimum root");
    }
  }
  for (trial = 0; trial < 100; ++trial) {
    const int64_t p = trial % 2 ? 2147483647 : 998244353;
    const int64_t r = static_cast<int64_t>(rng() % p);
    const auto got = blueberry::mod_sqrt(r * r % p, p);
    check(got && *got == min(r, p - r), "large sqrt");
  }
  const auto min_root = blueberry::mod_sqrt(numeric_limits<int64_t>::min(), 7);
  check(!min_root, "negative int64 minimum"); // -2^63 == 6 (mod 7).
  check_bm(vector<mint>{});
  check_bm(vector<mint>(20));
  check_bm(vector<mint>{0, 0, 0, 1});
  check_bm(vector<mint>{1, 0, 0, 0});
  check(blueberry::linear_recurrence_kth(vector<mint>{}, vector<mint>{}, 0) == mint(0),
        "empty recurrence");
  for (trial = 0; trial < 150; ++trial) {
    vector<atcoder::static_modint<5>> s(rng() % 13);
    for (auto& x : s) x = rng() % 5;
    check_bm(s);
    const int d = 1 + rng() % 10;
    vector<mint> a(d), c(d);
    for (auto& x : a) x = rng() % 100;
    for (auto& x : c) x = rng() % 10;
    if (trial % 3 == 0) fill(c.begin() + d / 2, c.end(), mint(0));
    if (trial % 5 == 0) fill(c.begin(), c.end(), mint(0));
    vector<mint> sequence = a;
    for (int i = d; i < 70; ++i) {
      mint value = 0;
      for (int j = 0; j < d; ++j) value += c[j] * sequence[i - j - 1];
      sequence.push_back(value);
    }
    const int k = rng() % sequence.size();
    check(blueberry::linear_recurrence_kth(a, c, k) == sequence[k], "kth naive DP");
    const u64 huge = trial % 2 ? rng() : numeric_limits<u64>::max();
    check(blueberry::linear_recurrence_kth(a, c, huge) == reference_kth(a, c, huge),
          "kth uint64 Kitamasa");
    sequence.resize(2 * d + 5);
    check_bm(sequence);
  }
  // Force ACL's NTT path and zero trailing coefficients at the convolution cutoff.
  for (int d : {60, 61, 64, 65, 128}) {
    vector<mint> a(d), c(d);
    for (auto& x : a) x = rng();
    for (auto& x : c) x = rng();
    c.back() = 0;
    check(blueberry::linear_recurrence_kth(a, c, 1234567890123456789ULL) ==
              reference_kth(a, c, 1234567890123456789ULL), "NTT kth");
  }
}
