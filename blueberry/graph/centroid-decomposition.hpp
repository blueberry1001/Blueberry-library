#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <vector>

namespace blueberry {
// Ordinary centroid ancestors, including distances and the removed branch.
class CentroidDecomposition {
 public:
  struct Entry { int centroid, distance, branch; };
 private:
  std::vector<int> parent_;
  std::vector<std::vector<Entry>> paths_;
 public:
  explicit CentroidDecomposition(const std::vector<std::vector<int>>& tree)
      : parent_(tree.size(), -1), paths_(tree.size()) {
    assert(tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int n = size();
    if (!n) return;
    std::vector<bool> removed(n, false);
    std::vector<int> parent(n, -1), count(n);
    std::function<void(int, int)> build = [&](int root, int above) {
      std::vector<int> order{root}; parent[root] = -1;
      for (std::size_t i = 0; i < order.size(); ++i) {
        int v = order[i]; count[v] = 1;
        for (int w : tree[v]) {
          assert(0 <= w && w < n);
          if (!removed[w] && w != parent[v]) { parent[w] = v; order.push_back(w); }
        }
      }
      int center = root;
      const int total = static_cast<int>(order.size());
      for (auto it = order.rbegin(); it != order.rend(); ++it) {
        int v = *it, largest = total - count[v];
        for (int w : tree[v]) if (!removed[w] && parent[w] == v) largest = std::max(largest, count[w]);
        if (largest <= total / 2) center = v;
        if (parent[v] != -1) count[parent[v]] += count[v];
      }
      parent_[center] = above; removed[center] = true;
      paths_[center].push_back({center, 0, -1});
      int branch = 0;
      for (int neighbor : tree[center]) if (!removed[neighbor]) {
        struct Visit { int v, parent, depth; };
        std::vector<Visit> stack{{neighbor, center, 1}};
        while (!stack.empty()) {
          auto [v, p, d] = stack.back(); stack.pop_back();
          paths_[v].push_back({center, d, branch});
          for (int w : tree[v]) if (!removed[w] && w != p) stack.push_back({w, v, d + 1});
        }
        ++branch;
      }
      for (int neighbor : tree[center]) if (!removed[neighbor]) build(neighbor, center);
    };
    build(0, -1);
  }
  int size() const { return static_cast<int>(parent_.size()); }
  int parent(int v) const { assert(0 <= v && v < size()); return parent_[v]; }
  const std::vector<Entry>& ancestors(int v) const {
    assert(0 <= v && v < size()); return paths_[v];
  }
};

// Callback (vertices, parent, color): root at index 0, parent[i] < i;
// colors 0/1 are real vertices, -1 virtual. Each unordered distinct pair is
// presented exactly once across different colors; its path passes index 0.
template <class F>
void centroid_decomposition_thirds(const std::vector<std::vector<int>>& tree, F callback) {
  assert(tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  struct Part { std::vector<std::vector<int>> g; std::vector<int> ids; std::vector<bool> real; };
  Part initial{tree, std::vector<int>(tree.size()), std::vector<bool>(tree.size(), true)};
  std::iota(initial.ids.begin(), initial.ids.end(), 0);
  auto solve = [&](auto&& self, Part part) -> void {
    const int n = static_cast<int>(part.g.size());
    if (std::count(part.real.begin(), part.real.end(), true) <= 1) return;
    std::vector<int> order{0}, parent(n, -1), count(n, 1);
    for (std::size_t i = 0; i < order.size(); ++i)
      for (int w : part.g[order[i]]) if (w != parent[order[i]]) {
        parent[w] = order[i]; order.push_back(w);
      }
    int center = 0;
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
      const int v = *it;
      int largest = n - count[v];
      for (int w : part.g[v]) if (parent[w] == v) largest = std::max(largest, count[w]);
      if (largest <= n / 2) center = v;
      if (parent[v] != -1) count[parent[v]] += count[v];
    }
    // Reorder once so callbacks and child parts have parent-before-child order.
    order = {center}; parent[center] = -1;
    std::vector<int> branch(n, -1), branch_size;
    for (std::size_t i = 0; i < order.size(); ++i) {
      const int v = order[i];
      for (int w : part.g[v]) if (w != parent[v]) {
        parent[w] = v;
        if (v == center) { branch[w] = static_cast<int>(branch_size.size()); branch_size.push_back(0); }
        else branch[w] = branch[v];
        ++branch_size[branch[w]]; order.push_back(w);
      }
    }
    std::vector<int> position(n), ids(n), par(n, -1), color(n, -1);
    for (int i = 0; i < n; ++i) { position[order[i]] = i; ids[i] = part.ids[order[i]]; }
    for (int i = 1; i < n; ++i) par[i] = position[parent[order[i]]];
    if (part.real[center]) {
      color[0] = 0;
      for (int i = 1; i < n; ++i) if (part.real[order[i]]) color[i] = 1;
      callback(ids, par, color);
      part.real[center] = false;
      self(self, std::move(part));
      return;
    }
    std::vector<int> side(branch_size.size(), 1);
    const int threshold = (n - 1) / 3 + ((n - 1) % 3 != 0);
    int chosen = 0;
    auto big = std::find_if(branch_size.begin(), branch_size.end(), [&](int s) { return s >= threshold; });
    if (big != branch_size.end()) side[big - branch_size.begin()] = 0;
    else for (std::size_t i = 0; i < side.size() && chosen < threshold; ++i) { side[i] = 0; chosen += branch_size[i]; }
    for (int i = 1; i < n; ++i) if (part.real[order[i]]) color[i] = side[branch[order[i]]];
    if (std::find(color.begin(), color.end(), 0) != color.end() &&
        std::find(color.begin(), color.end(), 1) != color.end()) callback(ids, par, color);
    for (int s = 0; s < 2; ++s) {
      Part child; child.ids.push_back(part.ids[center]); child.real.push_back(false); child.g.emplace_back();
      std::vector<int> map(n, -1); map[center] = 0;
      for (int i = 1; i < n; ++i) {
        const int v = order[i];
        if (side[branch[v]] != s) continue;
        const int id = static_cast<int>(child.ids.size()), p = map[parent[v]]; map[v] = id;
        child.ids.push_back(part.ids[v]); child.real.push_back(part.real[v]); child.g.emplace_back();
        child.g[p].push_back(id); child.g[id].push_back(p);
      }
      self(self, std::move(child));
    }
  };
  solve(solve, std::move(initial));
}
}  // namespace blueberry
