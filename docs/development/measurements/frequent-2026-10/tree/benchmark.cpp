#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include LCA_HEADER

using Clock = std::chrono::steady_clock;
double ms(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}

template <class LCA>
void measure(const std::vector<std::vector<int>>& tree, int root,
             const std::vector<std::pair<int, int>>& queries, const std::string& kind) {
  const auto start = Clock::now();
  const LCA lca(tree, root);
  const auto built = Clock::now();
  std::uint64_t checksum = 0;
  for (const auto [u, v] : queries) {
    int result;
    if constexpr (requires { lca.kth_ancestor(u, v); }) {
      if (kind == "kth") result = lca.kth_ancestor(u, v);
      else result = lca.lca(u, v);
    } else result = lca.lca(u, v);
    checksum = checksum * 0x9e3779b97f4a7c15ULL + result;
  }
  const auto queried = Clock::now();
  std::cout << "{\"build_ms\":" << ms(start, built)
            << ",\"query_ms\":" << ms(built, queried)
            << ",\"total_ms\":" << ms(start, queried)
            << ",\"checksum\":" << checksum << "}\n";
}

int main(int argc, char** argv) {
  if (argc != 7) return 1;
  const std::string kind = argv[1], shape = argv[2];
  const int n = std::atoi(argv[3]), q = std::atoi(argv[4]);
  std::mt19937 rng(static_cast<unsigned>(std::strtoul(argv[5], nullptr, 10)));
  const bool directed = std::atoi(argv[6]);
  std::vector<int> label(n), parent(n), depth(n);
  std::iota(label.begin(), label.end(), 0);
  if (shape == "permuted") std::shuffle(label.begin(), label.end(), rng);
  const int root = label[0];
  parent[root] = root;
  std::vector<std::vector<int>> tree(n);
  for (int i = 1; i < n; ++i) {
    const int p = shape == "path" ? i - 1 : shape == "star" ? 0
                  : shape == "balanced" ? (i - 1) / 2
                  : shape == "broom" ? (i < n / 2 ? i - 1 : n / 2 - 1)
                  : static_cast<int>(rng() % i);
    const int v = label[i], u = label[p];
    tree[u].push_back(v);
    if (!directed) tree[v].push_back(u);
    parent[v] = u;
    depth[v] = depth[u] + 1;
  }
  std::vector<std::pair<int, int>> queries;
  queries.reserve(q);
  for (int i = 0; i < q; ++i) {
    const int u = static_cast<int>(rng() % n);
    const int v = kind == "kth" ? static_cast<int>(rng() % (depth[u] + 1))
                                 : static_cast<int>(rng() % n);
    queries.emplace_back(u, v);
  }
  if (kind == "rmq") measure<blueberry::LowestCommonAncestorRMQ>(tree, root, queries, kind);
  else measure<blueberry::LowestCommonAncestor>(tree, root, queries, kind);
}
