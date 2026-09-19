#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include <algorithm>
#include "blueberry/utility/fast-io.hpp"
#include <limits>
#include <vector>
#include "blueberry/data-structure/sqrt-tree.hpp"
int op(int a, int b) { return std::min(a,b); }
int e() { return std::numeric_limits<int>::max(); }
int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q; in.read(n, q);
  std::vector<int> a(n); for (auto& x : a) in.read(x);
  blueberry::SqrtTree<int, op, e> tree(a);
  // Static RMQ covers an idempotent monoid alongside the sum driver.
  while (q--) { int l, r; in.read(l, r); out.writeln(tree.prod(l, r)); }
}
