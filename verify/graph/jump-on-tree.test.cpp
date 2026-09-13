#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"

#include <iostream>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<std::vector<int>> tree(n);
  for (int i = 0; i + 1 < n; ++i) {
    int u, v;
    std::cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
  }
  const blueberry::HLD hld(tree);
  while (q--) {
    int u, v, k;
    std::cin >> u >> v >> k;
    std::cout << (k <= hld.distance(u, v) ? hld.jump(u, v, k) : -1) << '\n';
  }
}
