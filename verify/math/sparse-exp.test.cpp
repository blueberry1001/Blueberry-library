#define PROBLEM "https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse"
#include <atcoder/modint>
#include "blueberry/math/sparse-exp.hpp"
#include "blueberry/utility/fast-io.hpp"

// Verify the sparse recurrence and every output coefficient with official cases.
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, k;
  if (!in.read(n, k)) return 0;
  using Mint = atcoder::modint998244353;
  std::vector<std::pair<int, Mint>> terms;
  terms.reserve(k);
  for (int i = 0; i < k; ++i) {
    int degree, coefficient;
    in.read(degree, coefficient);
    terms.emplace_back(degree, coefficient);
  }
  auto result = blueberry::sparse_exp(terms, n);
  for (int i = 0; i < n; ++i) {
    if (i) out.write(' ');
    out.write(result[i].val());
  }
  out.write('\n');
}
