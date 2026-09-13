#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

#include "blueberry/data-structure/disjoint-set-union.hpp"
#include "blueberry/data-structure/fenwick-tree.hpp"
#include "blueberry/data-structure/rollback-union-find.hpp"
#include "blueberry/data-structure/segment-tree.hpp"
#include "blueberry/data-structure/sparse-table.hpp"

namespace {

std::mt19937 rng(20260913);

void test_sparse_table() {
  const auto minimum = [](int a, int b) { return std::min(a, b); };
  for (int n = 1; n <= 100; ++n) {
    std::vector<int> values(n);
    for (int& value : values) value = static_cast<int>(rng() % 2001) - 1000;
    const blueberry::SparseTable<int, decltype(minimum)> table(values, minimum);
    assert(table.size() == n);
    for (int iteration = 0; iteration < 300; ++iteration) {
      int left = static_cast<int>(rng() % n);
      int right = static_cast<int>(rng() % n);
      if (left > right) std::swap(left, right);
      ++right;
      assert(table.product(left, right) ==
             *std::min_element(values.begin() + left, values.begin() + right));
    }
  }
}

void test_fenwick_tree() {
  for (int n = 0; n <= 100; ++n) {
    std::vector<long long> values(n);
    for (auto& value : values) value = rng() % 10;
    blueberry::FenwickTree<long long> bit(values);
    assert(bit.size() == n);
    for (int iteration = 0; iteration < 1000; ++iteration) {
      if (n > 0 && rng() % 3 == 0) {
        const int index = static_cast<int>(rng() % n);
        const long long delta = rng() % 10;
        values[index] += delta;
        bit.add(index, delta);
      } else if (n > 0 && rng() % 2 == 0) {
        const int index = static_cast<int>(rng() % n);
        assert(bit.get(index) == values[index]);
      } else {
        int left = n == 0 ? 0 : static_cast<int>(rng() % (n + 1));
        int right = n == 0 ? 0 : static_cast<int>(rng() % (n + 1));
        if (left > right) std::swap(left, right);
        assert(bit.sum(left, right) ==
               std::accumulate(values.begin() + left, values.begin() + right, 0LL));

        const long long total = std::accumulate(values.begin(), values.end(), 0LL);
        const long long target = static_cast<long long>(rng() % (total + 2));
        int expected = 0;
        long long prefix = 0;
        while (expected < n && prefix < target) prefix += values[expected++];
        assert(bit.lower_bound(target) == expected);
      }
    }
  }
}

void test_segment_tree() {
  const auto concatenate = [](const std::vector<int>& left, const std::vector<int>& right) {
    std::vector<int> result;
    result.reserve(left.size() + right.size());
    result.insert(result.end(), left.begin(), left.end());
    result.insert(result.end(), right.begin(), right.end());
    return result;
  };
  for (int n = 0; n <= 40; ++n) {
    std::vector<std::vector<int>> values(n);
    for (int i = 0; i < n; ++i) values[i] = {i};
    blueberry::SegmentTree<std::vector<int>, decltype(concatenate)> tree(
        values, concatenate, {});
    assert(tree.size() == n);
    for (int iteration = 0; iteration < 300; ++iteration) {
      int left = n == 0 ? 0 : static_cast<int>(rng() % (n + 1));
      int right = n == 0 ? 0 : static_cast<int>(rng() % (n + 1));
      if (left > right) std::swap(left, right);
      std::vector<int> expected;
      for (int i = left; i < right; ++i) {
        expected.insert(expected.end(), values[i].begin(), values[i].end());
      }
      assert(tree.product(left, right) == expected);
      if (n > 0 && rng() % 3 == 0) {
        const int index = static_cast<int>(rng() % n);
        values[index] = {static_cast<int>(rng())};
        tree.set(index, values[index]);
        assert(tree.get(index) == values[index]);
      }
    }
    assert(tree.all_product() == tree.product(0, n));
  }
}

void test_disjoint_set_union() {
  for (int n = 0; n <= 80; ++n) {
    blueberry::DisjointSetUnion dsu(n);
    std::vector<int> component(n);
    std::iota(component.begin(), component.end(), 0);
    for (int iteration = 0; n > 0 && iteration < 1000; ++iteration) {
      const int u = static_cast<int>(rng() % n);
      const int v = static_cast<int>(rng() % n);
      if (rng() % 2 == 0) {
        const bool expected = component[u] != component[v];
        const int from = component[v];
        const int to = component[u];
        assert(dsu.merge(u, v) == expected);
        if (expected) {
          for (int& value : component) {
            if (value == from) value = to;
          }
        }
      } else {
        assert(dsu.same(u, v) == (component[u] == component[v]));
        assert(dsu.component_size(u) ==
               static_cast<int>(std::count(component.begin(), component.end(), component[u])));
      }
    }
    auto groups = dsu.groups();
    for (auto& group : groups) std::sort(group.begin(), group.end());
    std::sort(groups.begin(), groups.end());
    std::vector<std::vector<int>> expected(n);
    for (int v = 0; v < n; ++v) expected[component[v]].push_back(v);
    expected.erase(std::remove_if(expected.begin(), expected.end(),
                                  [](const auto& group) { return group.empty(); }),
                   expected.end());
    std::sort(expected.begin(), expected.end());
    assert(groups == expected);
  }
}

void test_rollback_union_find() {
  constexpr int n = 40;
  blueberry::RollbackUnionFind dsu(n);
  std::vector<std::vector<int>> states(1, std::vector<int>(n));
  std::iota(states[0].begin(), states[0].end(), 0);
  for (int iteration = 0; iteration < 5000; ++iteration) {
    if (dsu.state() > 0 && rng() % 4 == 0) {
      const int target = static_cast<int>(rng() % (dsu.state() + 1));
      dsu.rollback(target);
      states.resize(target + 1);
    } else {
      const int u = static_cast<int>(rng() % n);
      const int v = static_cast<int>(rng() % n);
      auto next = states.back();
      const int from = next[v];
      const int to = next[u];
      const bool expected = from != to;
      assert(dsu.merge(u, v) == expected);
      if (expected) {
        for (int& value : next) {
          if (value == from) value = to;
        }
      }
      states.push_back(std::move(next));
    }
    const auto& current = states.back();
    int components = 0;
    for (int v = 0; v < n; ++v) components += current[v] == v;
    assert(dsu.components() == components);
    for (int sample = 0; sample < 10; ++sample) {
      const int u = static_cast<int>(rng() % n);
      const int v = static_cast<int>(rng() % n);
      assert(dsu.same(u, v) == (current[u] == current[v]));
      assert(dsu.component_size(u) ==
             static_cast<int>(std::count(current.begin(), current.end(), current[u])));
    }
  }
  dsu.snapshot();
  const int saved = dsu.state();
  dsu.merge(0, 1);
  dsu.rollback();
  assert(dsu.state() == saved);
}

}  // namespace

int main() {
  test_sparse_table();
  test_fenwick_tree();
  test_segment_tree();
  test_disjoint_set_union();
  test_rollback_union_find();
}
