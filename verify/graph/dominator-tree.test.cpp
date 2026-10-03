#define PROBLEM "https://judge.yosupo.jp/problem/dominatortree"
#include <iostream>
#include <vector>
#include "blueberry/graph/dominator-tree.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, m, root;
  std::cin >> n >> m >> root;
  std::vector<std::vector<int>> graph(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    std::cin >> u >> v;
    graph[u].push_back(v);
  }
  // Root points to itself; unreachable vertices are reported as -1.
  const auto idom = blueberry::dominator_tree(graph, root);
  for (int v = 0; v < n; ++v) std::cout << idom[v] << (v + 1 == n ? '\n' : ' ');
}
