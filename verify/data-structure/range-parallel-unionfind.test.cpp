#define PROBLEM "https://judge.yosupo.jp/problem/range_parallel_unionfind"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/range-parallel-union-find.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  using Mint = atcoder::modint998244353;
  int n, q;
  cin >> n >> q;
  vector<Mint> weight(n);
  for (auto& x : weight) { int v; cin >> v; x = v; }
  blueberry::RangeParallelUnionFind uf(n);
  Mint answer = 0;
  while (q--) {
    int k, a, b;
    cin >> k >> a >> b;
    uf.range_merge(a, b, k, [&](int kept, int removed) {
      answer += weight[kept] * weight[removed];
      weight[kept] += weight[removed];
    });
    cout << answer.val() << '\n';
  }
}
