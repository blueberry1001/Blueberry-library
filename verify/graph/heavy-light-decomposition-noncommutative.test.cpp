#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"

#include <iostream>
#include <utility>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

constexpr long long kMod = 998244353;
using Affine = std::pair<long long, long long>;

Affine compose(const Affine& first, const Affine& second) {
  return {second.first * first.first % kMod,
          (second.first * first.second + second.second) % kMod};
}

class SegmentTree {
 public:
  explicit SegmentTree(const std::vector<Affine>& values) : n_(1) {
    while (n_ < static_cast<int>(values.size())) n_ *= 2;
    data_.assign(2 * n_, {1, 0});
    for (int i = 0; i < static_cast<int>(values.size()); ++i) data_[n_ + i] = values[i];
    for (int i = n_ - 1; i > 0; --i) data_[i] = compose(data_[2 * i], data_[2 * i + 1]);
  }

  void set(int position, Affine value) {
    int index = position + n_;
    data_[index] = value;
    while (index >>= 1) data_[index] = compose(data_[2 * index], data_[2 * index + 1]);
  }

  Affine prod(int left, int right) const {
    Affine lhs{1, 0}, rhs{1, 0};
    for (left += n_, right += n_; left < right; left /= 2, right /= 2) {
      if (left & 1) lhs = compose(lhs, data_[left++]);
      if (right & 1) rhs = compose(data_[--right], rhs);
    }
    return compose(lhs, rhs);
  }

 private:
  int n_;
  std::vector<Affine> data_;
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<Affine> value(n);
  for (auto& [a, b] : value) std::cin >> a >> b;
  std::vector<std::vector<int>> tree(n);
  for (int i = 0; i + 1 < n; ++i) {
    int u, v;
    std::cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
  }

  const blueberry::HLD hld(tree);
  std::vector<Affine> forward_values(n), reverse_values(n);
  for (int v = 0; v < n; ++v) {
    forward_values[hld.in(v)] = value[v];
    reverse_values[n - 1 - hld.in(v)] = value[v];
  }
  SegmentTree forward(forward_values), reverse(reverse_values);

  while (q--) {
    int type;
    std::cin >> type;
    if (type == 0) {
      int v;
      Affine f;
      std::cin >> v >> f.first >> f.second;
      forward.set(hld.in(v), f);
      reverse.set(n - 1 - hld.in(v), f);
    } else {
      int u, v;
      long long x;
      std::cin >> u >> v >> x;
      Affine path{1, 0};
      hld.vertex_query(u, v, [&](int left, int right, bool reversed) {
        const Affine part = reversed ? reverse.prod(n - right, n - left)
                                     : forward.prod(left, right);
        path = compose(path, part);
      });
      std::cout << (path.first * x + path.second) % kMod << '\n';
    }
  }
}
