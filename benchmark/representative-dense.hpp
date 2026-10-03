#pragma once

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <atcoder/dsu>
#include <atcoder/fenwicktree>
#include <atcoder/segtree>

#include "blueberry/data-structure/disjoint-set-union.hpp"
#include "blueberry/data-structure/fenwick-tree.hpp"
#include "blueberry/data-structure/segment-tree.hpp"

// Included after the survey harness defines U64, SurveyResult, RNG, mix,
// timed, and ensure. Input generation, hashing, destruction, and brute checks
// are outside all reported times. No implementation is selected inside a loop.
namespace representative_dense {

inline void hash_word(U64& hash, U64 value) {
  hash = mix(hash ^ mix(value + U64{0x9e3779b97f4a7c15}));
}

struct DsuInput {
  std::vector<std::pair<int, int>> merges, same_queries;
  std::vector<int> size_queries;
  U64 hash = 0;
};

inline DsuInput make_dsu_input(const std::string& shape, int n, int q) {
  RNG rng{0x46454e4345445355ULL};
  DsuInput input;
  input.merges.reserve(q);
  input.same_queries.reserve(q);
  input.size_queries.reserve(q);
  hash_word(input.hash, n);
  hash_word(input.hash, q);
  hash_word(input.hash, shape == "clustered");
  // Clustered merges stay within consecutive blocks of at most 64 vertices.
  // Both shapes include explicit self-merges and duplicate merge requests.
  auto partner = [&](int u) {
    if (shape == "random") return static_cast<int>(rng.next() % n);
    const int begin = u / 64 * 64;
    return begin + static_cast<int>(rng.next() % std::min(64, n - begin));
  };
  for (int i = 0; i < q; ++i) {
    int u = static_cast<int>(rng.next() % n);
    int v = partner(u);
    if (i % 8 == 0) v = u;
    if (i % 8 == 3) {
      u = input.merges.back().first;
      v = input.merges.back().second;
    }
    input.merges.emplace_back(u, v);
    hash_word(input.hash, u);
    hash_word(input.hash, v);
  }
  for (int i = 0; i < q; ++i) {
    const int u = static_cast<int>(rng.next() % n);
    // Half of clustered connectivity queries are local; half are global.
    const int v = shape == "clustered" && i % 2 == 0
                      ? partner(u)
                      : static_cast<int>(rng.next() % n);
    const int p = static_cast<int>(rng.next() % n);
    input.same_queries.emplace_back(u, v);
    input.size_queries.push_back(p);
    hash_word(input.hash, u);
    hash_word(input.hash, v);
    hash_word(input.hash, p);
  }
  return input;
}

struct BlueberryDsu {
  blueberry::DisjointSetUnion tree;
  explicit BlueberryDsu(int n) : tree(n) {}
  void merge(int a, int b) { tree.merge(a, b); }
  bool same(int a, int b) { return tree.same(a, b); }
  int size(int p) { return tree.component_size(p); }
};

struct AclDsu {
  atcoder::dsu tree;
  explicit AclDsu(int n) : tree(n) {}
  void merge(int a, int b) { tree.merge(a, b); }
  bool same(int a, int b) { return tree.same(a, b); }
  int size(int p) { return tree.size(p); }
};

template <class Dsu>
SurveyResult run_dsu(const DsuInput& input, int n, int q, bool check) {
  std::optional<Dsu> tree;
  const U64 build_ns = timed([&] { tree.emplace(n); });
  const U64 update_ns = timed([&] {
    for (const auto& [u, v] : input.merges) tree->merge(u, v);
  });
  U64 same_checksum = 0, size_checksum = 0;
  const U64 same_ns = timed([&] {
    for (const auto& [u, v] : input.same_queries) same_checksum += tree->same(u, v);
  });
  const U64 size_ns = timed([&] {
    for (const int p : input.size_queries) size_checksum += tree->size(p);
  });
  if (check) {
    // A label array is an independent, deliberately simple partition oracle.
    std::vector<int> labels(n);
    std::iota(labels.begin(), labels.end(), 0);
    for (const auto& [u, v] : input.merges) {
      const int from = labels[v], to = labels[u];
      for (int& label : labels) {
        if (label == from) label = to;
      }
    }
    U64 expected_same = 0, expected_size = 0;
    for (const auto& [u, v] : input.same_queries) {
      const bool expected = labels[u] == labels[v];
      ensure(tree->same(u, v) == expected, "dense DSU connectivity mismatch");
      expected_same += expected;
    }
    for (const int p : input.size_queries) {
      const int expected = static_cast<int>(std::count(labels.begin(), labels.end(), labels[p]));
      ensure(tree->size(p) == expected, "dense DSU component size mismatch");
      expected_size += expected;
    }
    ensure(same_checksum == expected_same && size_checksum == expected_size,
           "dense DSU timed checksum mismatch");
    for (int u = 0; u < n; ++u) {
      for (int v = 0; v < n; ++v) {
        ensure(tree->same(u, v) == (labels[u] == labels[v]), "dense DSU partition mismatch");
      }
    }
  }
  SurveyResult result;
  result.values = {{"build_ns", build_ns}, {"update_ns", update_ns},
                   {"same_ns", same_ns}, {"size_ns", size_ns},
                   {"query_ns", same_ns + size_ns}, {"input_hash", input.hash},
                   {"checksum", mix(same_checksum) ^ mix(size_checksum + 1)},
                   {"same_checksum", same_checksum}, {"size_checksum", size_checksum},
                   {"merge_count", static_cast<U64>(q)},
                   {"same_count", static_cast<U64>(q)}, {"size_count", static_cast<U64>(q)}};
  return result;
}

struct ArrayInput {
  std::vector<U64> initial, update_values;
  std::vector<int> update_indices, get_indices;
  std::vector<std::pair<int, int>> ranges;
  U64 hash = 0;
};

inline ArrayInput make_array_input(const std::string& shape, int n, int q) {
  RNG rng{0x44454e5345415252ULL};
  ArrayInput input;
  input.initial.resize(n);
  input.update_values.reserve(q);
  input.update_indices.reserve(q);
  input.get_indices.reserve(q);
  input.ranges.reserve(q);
  hash_word(input.hash, n);
  hash_word(input.hash, q);
  hash_word(input.hash, shape == "sequential");
  for (U64& value : input.initial) {
    value = rng.next() % 1000000;
    hash_word(input.hash, value);
  }
  for (int i = 0; i < q; ++i) {
    const int update = shape == "sequential" ? i % n : static_cast<int>(rng.next() % n);
    const int get = shape == "sequential" ? i % n : static_cast<int>(rng.next() % n);
    const U64 value = rng.next() % 1000000;
    // Sequential means cyclic update/get indices and cyclic range starts with
    // random clipped lengths. Random ranges have two independent endpoints.
    int left, right;
    if (shape == "sequential") {
      left = static_cast<int>(static_cast<U64>(i) % (static_cast<U64>(n) + 1));
      const U64 length = rng.next() % (static_cast<U64>(n) + 1);
      right = static_cast<int>(std::min(static_cast<U64>(n), static_cast<U64>(left) + length));
    } else {
      left = static_cast<int>(rng.next() % (static_cast<U64>(n) + 1));
      right = static_cast<int>(rng.next() % (static_cast<U64>(n) + 1));
      if (left > right) std::swap(left, right);
    }
    input.update_indices.push_back(update);
    input.get_indices.push_back(get);
    input.update_values.push_back(value);
    input.ranges.emplace_back(left, right);
    hash_word(input.hash, update);
    hash_word(input.hash, get);
    hash_word(input.hash, value);
    hash_word(input.hash, left);
    hash_word(input.hash, right);
  }
  return input;
}

struct BlueberryFenwick {
  blueberry::FenwickTree<U64> tree;
  explicit BlueberryFenwick(int n) : tree(n) {}
  void update(int p, U64 x) { tree.add(p, x); }
  U64 range(int l, int r) { return tree.sum(l, r); }
  U64 get(int p) { return tree.get(p); }
};

struct AclFenwick {
  atcoder::fenwick_tree<U64> tree;
  explicit AclFenwick(int n) : tree(n) {}
  void update(int p, U64 x) { tree.add(p, x); }
  U64 range(int l, int r) { return tree.sum(l, r); }
  // ACL has no public point-get operation; this is operation-equivalent.
  U64 get(int p) { return tree.sum(p, p + 1); }
};

struct Sum {
  U64 operator()(U64 a, U64 b) const { return a + b; }
};
inline U64 sum(U64 a, U64 b) { return a + b; }
inline U64 zero() { return 0; }

struct BlueberrySegmentTree {
  blueberry::SegmentTree<U64, Sum> tree;
  explicit BlueberrySegmentTree(const std::vector<U64>& values) : tree(values, Sum{}, U64{}) {}
  void update(int p, U64 x) { tree.set(p, x); }
  U64 range(int l, int r) { return tree.prod(l, r); }
  U64 get(int p) { return tree.get(p); }
};

struct AclSegmentTree {
  atcoder::segtree<U64, sum, zero> tree;
  explicit AclSegmentTree(const std::vector<U64>& values) : tree(values) {}
  void update(int p, U64 x) { tree.set(p, x); }
  U64 range(int l, int r) { return tree.prod(l, r); }
  U64 get(int p) { return tree.get(p); }
};

template <class Tree, bool Fenwick>
SurveyResult run_array(const ArrayInput& input, int n, int q, bool check) {
  std::optional<Tree> tree;
  U64 build_ns, initialization_ns = 0;
  if constexpr (Fenwick) {
    // Equal public construction contract: zero initialization, then N adds.
    // The Blueberry vector constructor is deliberately not substituted here.
    build_ns = timed([&] { tree.emplace(n); });
    initialization_ns = timed([&] {
      for (int i = 0; i < n; ++i) tree->update(i, input.initial[i]);
    });
  } else {
    build_ns = timed([&] { tree.emplace(input.initial); });
  }
  const U64 update_ns = timed([&] {
    for (int i = 0; i < q; ++i) tree->update(input.update_indices[i], input.update_values[i]);
  });
  U64 range_checksum = 0, get_checksum = 0;
  const U64 range_ns = timed([&] {
    for (const auto& [left, right] : input.ranges) range_checksum += tree->range(left, right);
  });
  const U64 get_ns = timed([&] {
    for (const int p : input.get_indices) get_checksum += tree->get(p);
  });
  if (check) {
    std::vector<U64> values = input.initial;
    for (int i = 0; i < q; ++i) {
      if constexpr (Fenwick) {
        values[input.update_indices[i]] += input.update_values[i];
      } else {
        values[input.update_indices[i]] = input.update_values[i];
      }
    }
    U64 expected_range = 0, expected_get = 0;
    for (const auto& [left, right] : input.ranges) {
      const U64 expected = std::accumulate(values.begin() + left, values.begin() + right, U64{});
      ensure(tree->range(left, right) == expected, "dense array range mismatch");
      expected_range += expected;
    }
    for (const int p : input.get_indices) {
      ensure(tree->get(p) == values[p], "dense array point mismatch");
      expected_get += values[p];
    }
    ensure(range_checksum == expected_range && get_checksum == expected_get,
           "dense array timed checksum mismatch");
    for (int p = 0; p < n; ++p) ensure(tree->get(p) == values[p], "dense array final state mismatch");
    ensure(tree->range(0, 0) == 0 && tree->range(n, n) == 0, "dense array empty range mismatch");
    ensure(tree->range(0, n) == std::accumulate(values.begin(), values.end(), U64{}),
           "dense array full range mismatch");
  }
  SurveyResult result;
  result.values = {{"build_ns", build_ns}, {"update_ns", update_ns},
                   {"range_ns", range_ns}, {"get_ns", get_ns},
                   {"query_ns", range_ns + get_ns}, {"input_hash", input.hash},
                   {"checksum", mix(range_checksum) ^ mix(get_checksum + 1)},
                   {"range_checksum", range_checksum}, {"get_checksum", get_checksum},
                   {"update_count", static_cast<U64>(q)},
                   {"range_count", static_cast<U64>(q)}, {"get_count", static_cast<U64>(q)}};
  if constexpr (Fenwick) {
    result.values["initialization_ns"] = initialization_ns;
    result.values["initialization_count"] = static_cast<U64>(n);
  }
  return result;
}

}  // namespace representative_dense

inline SurveyResult run_dense(std::string family, std::string shape, std::string implementation,
                              int n, int q, bool check) {
  ensure(n > 0 && q >= 0, "dense survey requires n > 0 and q >= 0");
  ensure(implementation == "blueberry" || implementation == "acl", "unknown dense implementation");
  using namespace representative_dense;
  if (family == "dsu") {
    ensure(shape == "random" || shape == "clustered", "unknown DSU shape");
    const auto input = make_dsu_input(shape, n, q);
    if (implementation == "blueberry") return run_dsu<BlueberryDsu>(input, n, q, check);
    return run_dsu<AclDsu>(input, n, q, check);
  }
  ensure(family == "fenwick" || family == "segment-tree", "unknown dense family");
  ensure(shape == "random" || shape == "sequential", "unknown dense array shape");
  const auto input = make_array_input(shape, n, q);
  if (family == "fenwick") {
    if (implementation == "blueberry") return run_array<BlueberryFenwick, true>(input, n, q, check);
    return run_array<AclFenwick, true>(input, n, q, check);
  }
  if (implementation == "blueberry") return run_array<BlueberrySegmentTree, false>(input, n, q, check);
  return run_array<AclSegmentTree, false>(input, n, q, check);
}
