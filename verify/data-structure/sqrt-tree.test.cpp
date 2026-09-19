#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/sqrt-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q; in.read(n, q);
  std::vector<long long> a(n); for (auto& x : a) in.read(x);
  blueberry::SqrtTree<long long, op, e> tree(a);
  // Non-idempotent aggregation across square-root block boundaries.
  while (q--) { int l, r; in.read(l, r); out.writeln(tree.prod(l, r)); }
}
