#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include "blueberry/graph/lowest-common-ancestor.hpp"

// Check arbitrary labels/roots, directed children, and table-height boundaries.
void expect(int expected, int actual, const char* api, unsigned long long seed,
            const std::vector<std::vector<int>>& tree, int root, int u, int v) {
  if (expected == actual) return;
  std::cerr << "seed=" << seed << " api=" << api << " n=" << tree.size()
            << " root=" << root << " u=" << u << " v=" << v
            << " expected=" << expected << " actual=" << actual << '\n';
  for (int x = 0; x < static_cast<int>(tree.size()); ++x) {
    std::cerr << x << ':';
    for (const int to : tree[x]) std::cerr << ' ' << to;
    std::cerr << '\n';
  }
  std::exit(1);
}

int main(int argc, char** argv) {
  const char* env_seed = std::getenv("BLUEBERRY_RANDOM_SEED");
  const auto seed = std::strtoull(argc > 1 ? argv[1] : env_seed ? env_seed : "20261017", nullptr, 10);
  std::mt19937_64 rng(seed);
  for (const int n : {1, 2, 3, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33, 65, 129}) {
    for (int shape = 0; shape < 5; ++shape) {
      std::vector<int> label(n), parent(n), depth(n);
      std::iota(label.begin(), label.end(), 0);
      std::shuffle(label.begin(), label.end(), rng);
      const int root = label[0];
      parent[root] = root;
      std::vector<std::vector<int>> directed(n), undirected(n);
      for (int i = 1; i < n; ++i) {
        const int p = shape == 0 ? i - 1 : shape == 1 ? 0 : shape == 2 ? (i - 1) / 2
                                  : shape == 3 ? std::max(0, i - 3)
                                               : static_cast<int>(rng() % i);
        const int v = label[i], u = label[p];
        parent[v] = u;
        depth[v] = depth[u] + 1;
        directed[u].push_back(v);
        undirected[u].push_back(v);
        undirected[v].push_back(u);
      }
      for (auto* tree : {&directed, &undirected}) {
        for (auto& edges : *tree) std::shuffle(edges.begin(), edges.end(), rng);
        const blueberry::LowestCommonAncestor doubling(*tree, root);
        const blueberry::LowestCommonAncestorRMQ rmq(*tree, root);
        auto check = [&](int expected, int actual, const char* api, int u = -1, int v = -1) {
          expect(expected, actual, api, seed, *tree, root, u, v);
        };
        check(n, doubling.size(), "doubling.size");
        check(n, rmq.size(), "rmq.size");
        for (int u = 0; u < n; ++u) {
          check(depth[u], doubling.depth(u), "doubling.depth", u);
          check(depth[u], rmq.depth(u), "rmq.depth", u);
          int ancestor = u;
          for (int k = 0; k <= depth[u]; ++k) {
            check(ancestor, doubling.kth_ancestor(u, k), "kth_ancestor", u, k);
            check(ancestor, doubling.kth(u, k), "kth", u, k);
            ancestor = parent[ancestor];
          }
          for (int v = 0; v < n; ++v) {
            int a = u, b = v;
            while (depth[a] > depth[b]) a = parent[a];
            while (depth[b] > depth[a]) b = parent[b];
            while (a != b) a = parent[a], b = parent[b];
            check(a, doubling.lca(u, v), "doubling.lca", u, v);
            check(a, rmq.lca(u, v), "rmq.lca", u, v);
            const int distance = depth[u] + depth[v] - 2 * depth[a];
            check(distance, doubling.distance(u, v), "doubling.distance", u, v);
            check(distance, rmq.distance(u, v), "rmq.distance", u, v);
          }
        }
      }
    }
  }
  // Iterative construction must also support deep trees without recursion.
  const int n = 200000;
  std::vector<std::vector<int>> path(n);
  for (int v = 1; v < n; ++v) path[v - 1].push_back(v);
  const blueberry::LowestCommonAncestor doubling(path);
  const blueberry::LowestCommonAncestorRMQ rmq(path);
  for (int v = 0; v < n; v += 997) {
    expect(v, doubling.lca(v, n - 1), "deep doubling.lca", seed, path, 0, v, n - 1);
    expect(v, rmq.lca(v, n - 1), "deep rmq.lca", seed, path, 0, v, n - 1);
    expect(v, doubling.kth(n - 1, n - 1 - v), "deep kth", seed, path, 0, n - 1, n - 1 - v);
  }
}
