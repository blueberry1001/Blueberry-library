#define PROBLEM "https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod"
#include <algorithm>
#include <atcoder/modint>
#include <utility>
#include <vector>
#include "blueberry/math/combinations.hpp"
#include "blueberry/utility/fast-io.hpp"
using namespace std;
template<class Mint>
void solve(int limit, const vector<pair<int,int>>& queries, blueberry::FastOutput& out) {
  blueberry::Combinations<Mint> comb(limit);
  for (auto [n,k] : queries) out.writeln(comb.C(n,k).val());
}
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int t, p;
  if (!in.read(t,p)) return 1;
  vector<pair<int, int>> queries(t);
  int limit = 0;
  for (auto& [n, k] : queries) { if (!in.read(n,k)) return 1; limit = max(limit, n); }
  // The official contract has n < p. Precompute only up to the queried maximum.
  if (p == 998244353) solve<atcoder::modint998244353>(limit,queries,out);
  else if (p == 1000000007) solve<atcoder::modint1000000007>(limit,queries,out);
  else {
    atcoder::dynamic_modint<0>::set_mod(p);
    solve<atcoder::dynamic_modint<0>>(limit,queries,out);
  }
}
