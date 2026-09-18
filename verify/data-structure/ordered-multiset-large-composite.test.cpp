#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite_large_array"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/ordered-multiset.hpp"

using Mint = atcoder::modint998244353;
struct Function { long long key; Mint a, b; };
Function op(Function f, Function g) {
  if (f.key == -1) return g;
  if (g.key == -1) return f;
  return {f.key, g.a * f.a, g.a * f.b + g.b};
}
Function e() { return {-1, 1, 0}; }
struct Compare {
  bool operator()(const Function& f, const Function& g) const { return f.key < g.key; }
};
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  long long n;
  int q;
  std::cin >> n >> q;
  blueberry::OrderedMultiset<Function, op, e, Compare> bag;
  while (q--) {
    int type;
    std::cin >> type;
    if (type == 0) {
      long long p;
      int a, b;
      std::cin >> p >> a >> b;
      bag.erase({p, 0, 0});
      bag.insert({p, a, b});
    } else {
      long long l, r;
      int x;
      std::cin >> l >> r >> x;
      // Untouched positions are identity maps and need no stored node.
      const auto f = bag.prod(bag.rank({l, 0, 0}), bag.rank({r, 0, 0}));
      std::cout << (f.a * x + f.b).val() << '\n';
    }
  }
}
