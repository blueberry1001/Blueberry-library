#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
// @title Disjoint Sparse Table - Static Range Sum（区間和）
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q; in.read(n, q);
  std::vector<long long> a(n); for (auto& x : a) in.read(x);
  blueberry::DisjointSparseTable table(a, [](long long x, long long y) { return x + y; });
  // Non-idempotent sum exercises disjoint aggregation.
  while (q--) { int l, r; in.read(l, r); out.writeln(l == r ? 0 : table.prod(l, r)); }
}
