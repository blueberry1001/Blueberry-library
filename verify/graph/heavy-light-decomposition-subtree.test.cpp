#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"

#include <iostream>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

class Fenwick {
 public:
  explicit Fenwick(int n) : bit_(n + 1, 0) {}

  void add(int index, long long value) {
    for (++index; index < static_cast<int>(bit_.size()); index += index & -index) {
      bit_[index] += value;
    }
  }

  long long sum(int right) const {
    long long result = 0;
    for (; right > 0; right -= right & -right) result += bit_[right];
    return result;
  }

  long long sum(int left, int right) const { return sum(right) - sum(left); }

 private:
  std::vector<long long> bit_;
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<long long> value(n);
  for (long long& x : value) std::cin >> x;
  std::vector<std::vector<int>> tree(n);
  for (int v = 1; v < n; ++v) {
    int parent;
    std::cin >> parent;
    tree[parent].push_back(v);
  }

  const blueberry::HeavyLightDecomposition hld(tree);
  Fenwick fenwick(n);
  for (int v = 0; v < n; ++v) fenwick.add(hld.in(v), value[v]);
  while (q--) {
    int type;
    std::cin >> type;
    if (type == 0) {
      int vertex;
      long long delta;
      std::cin >> vertex >> delta;
      fenwick.add(hld.in(vertex), delta);
    } else {
      int vertex;
      std::cin >> vertex;
      long long answer = 0;
      hld.subtree_query(vertex, [&](int left, int right) {
        answer += fenwick.sum(left, right);
      });
      std::cout << answer << '\n';
    }
  }
}
