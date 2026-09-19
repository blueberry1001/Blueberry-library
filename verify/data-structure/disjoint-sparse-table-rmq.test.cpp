#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
// @title Disjoint Sparse Table - Static RMQ（区間最小値）
#include <algorithm>
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"

int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  std::vector<int> a(n);
  for (int& x : a) in.read(x);
  // Static RMQ asks for the minimum, unlike the separate range-sum verifier.
  blueberry::DisjointSparseTable table(a, [](int x, int y) { return std::min(x, y); });
  while (q--) {
    int l, r;
    in.read(l, r);
    out.writeln(table.prod(l, r));
  }
}
