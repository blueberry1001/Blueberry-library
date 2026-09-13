#define PROBLEM "https://judge.yosupo.jp/problem/lca"

#include <iostream>
#include <vector>

#include "blueberry/graph/lowest-common-ancestor.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<std::vector<int>> tree(n);
  for (int v = 1; v < n; ++v) {
    int parent;
    std::cin >> parent;
    tree[parent].push_back(v);
  }
  const blueberry::LowestCommonAncestorRMQ lca(tree);
  while (q--) {
    int u, v;
    std::cin >> u >> v;
    std::cout << lca.lca(u, v) << '\n';
  }
}
