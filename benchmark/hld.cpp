// Run with hld.py; both headers are immutable snapshots made by the runner.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define blueberry hld_baseline
#include HLD_BASELINE_HEADER
#undef blueberry
#define blueberry hld_candidate
#include HLD_CANDIDATE_HEADER
#undef blueberry

using Clock = std::chrono::steady_clock;
using Word = std::uint64_t;
using Tree = std::vector<std::vector<int>>;
using Queries = std::vector<std::pair<int, int>>;
volatile Word sink = 0;

Word mix(Word hash, Word value) { return hash * 1000000007ULL + value + 1; }

struct Result {
  double milliseconds;
  Word checksum;
  Word calls;
};

template <class HLD>
Word layout_checksum(const HLD& hld) {
  Word value = 0;
  for (int v = 0; v < hld.size(); ++v) {
    value = mix(value, hld.parent(v));
    value = mix(value, hld.depth(v));
    value = mix(value, hld.head(v));
    value = mix(value, hld.in(v));
    value = mix(value, hld.out(v));
    value = mix(value, hld.vertex_at(v));
  }
  return value;
}

template <class HLD>
Result build(const Tree& tree) {
  const auto begin = Clock::now();
  HLD hld(tree);
  const auto end = Clock::now();
  const auto checksum = layout_checksum(hld);
  sink = checksum;
  // Inspection and destruction are outside the constructor timing.
  return {std::chrono::duration<double, std::milli>(end - begin).count(), checksum, 1};
}

enum Operation { lca, path2_vertex, path2_edge, path3_vertex, path3_edge,
                 subtree_range, subtree_query };
constexpr const char* names[] = {"lca", "path2_vertex", "path2_edge", "path3_vertex",
                                "path3_edge", "subtree_range", "subtree_query"};

template <Operation operation, class HLD>
Result query(const HLD& hld, const Queries& queries) {
  Word checksum = 0, calls = 0;
  const auto begin = Clock::now();
  for (auto [u, v] : queries) {
    if constexpr (operation == lca) {
      checksum = mix(checksum, hld.lca(u, v));
      ++calls;
    } else if constexpr (operation == path2_vertex || operation == path2_edge) {
      hld.path_query(u, v, [&](int left, int right) {
        checksum = mix(checksum, (Word(left) << 32) | Word(right));
        ++calls;
      }, operation == path2_edge);
    } else if constexpr (operation == path3_vertex || operation == path3_edge) {
      hld.path_query(u, v, [&](int left, int right, bool reversed) {
        checksum = mix(checksum, (Word(left) << 32) | Word(right));
        checksum = mix(checksum, reversed);
        ++calls;
      }, operation == path3_edge);
    } else if constexpr (operation == subtree_range) {
      auto [left, right] = hld.subtree_range(u, v & 1);
      checksum = mix(checksum, (Word(left) << 32) | Word(right));
      ++calls;
    } else {
      hld.subtree_query(u, [&](int left, int right, bool reversed) {
        checksum = mix(checksum, (Word(left) << 32) | Word(right));
        checksum = mix(checksum, reversed);
        ++calls;
      }, v & 1);
    }
    // Keep empty edge paths observable and distinguish query boundaries.
    checksum = mix(checksum, u);
  }
  const auto end = Clock::now();
  sink = checksum;
  return {std::chrono::duration<double, std::milli>(end - begin).count(), checksum, calls};
}

template <class HLD>
Result dispatch(int operation, const HLD& hld, const Queries& queries) {
  switch (operation) {
    case lca: return query<lca>(hld, queries);
    case path2_vertex: return query<path2_vertex>(hld, queries);
    case path2_edge: return query<path2_edge>(hld, queries);
    case path3_vertex: return query<path3_vertex>(hld, queries);
    case path3_edge: return query<path3_edge>(hld, queries);
    case subtree_range: return query<subtree_range>(hld, queries);
    default: return query<subtree_query>(hld, queries);
  }
}

int main(int argc, char** argv) {
  if (argc != 6) throw std::runtime_error("usage: benchmark N Q repeats seed baseline_only");
  const int n = std::stoi(argv[1]), q = std::stoi(argv[2]), repeats = std::stoi(argv[3]);
  const Word seed = std::stoull(argv[4]);
  const bool baseline_only = std::stoi(argv[5]);
  if (n < 2 || q < 1 || repeats < 1) throw std::runtime_error("invalid dimensions");
  std::cout << "shape,n,q,seed,variant,operation,repeat,ms,checksum,calls\n"
            << std::fixed << std::setprecision(6);
  for (const std::string shape : {"chain", "star", "balanced", "random_parent"}) {
    std::mt19937_64 rng(seed);
    Tree tree(n);
    std::vector<int> parents(n);
    for (int v = 1; v < n; ++v) {
      const int parent = shape == "chain" ? v - 1 : shape == "star" ? 0 :
                         shape == "balanced" ? (v - 1) / 2 : int(rng() % v);
      parents[v] = parent;
      tree[parent].push_back(v);
      tree[v].push_back(parent);
    }
    // Shuffle locally, including heavy-child ties, but use identical inputs.
    for (auto& neighbors : tree) std::shuffle(neighbors.begin(), neighbors.end(), rng);
    Queries queries(q);
    for (int i = 0; i < q; ++i) {
      const int u = rng() % n;
      int v = rng() % n;
      if (i % 10 == 0) v = u;
      if (i % 10 == 1) {
        v = u;
        for (int steps = 1 + rng() % 32; steps > 0; --steps) v = parents[v];
      }
      queries[i] = i % 20 == 1 ? std::make_pair(v, u) : std::make_pair(u, v);
    }
    hld_baseline::HLD before(tree);
    hld_candidate::HLD after(tree);
    auto emit = [&](const Result& value, int variant, int operation, int repeat) {
      if (repeat >= 0)
        std::cout << shape << ',' << n << ',' << q << ',' << seed << ','
                  << (variant ? "candidate" : "baseline") << ','
                  << (operation < 0 ? "build" : names[operation]) << ',' << repeat << ','
                  << value.milliseconds << ',' << value.checksum << ',' << value.calls << '\n';
    };
    for (int operation = -1; operation <= subtree_query; ++operation) {
      for (int repeat = -1; repeat < repeats; ++repeat) {
        Result results[2]{};
        for (int turn = 0; turn < (baseline_only ? 1 : 2); ++turn) {
          const int variant = baseline_only ? 0 : turn ^ ((repeat + 1) & 1);
          if (variant == 0) results[variant] = operation < 0
              ? build<hld_baseline::HLD>(tree) : dispatch(operation, before, queries);
          else results[variant] = operation < 0
              ? build<hld_candidate::HLD>(tree) : dispatch(operation, after, queries);
          emit(results[variant], variant, operation, repeat);
        }
        if (!baseline_only && (results[0].checksum != results[1].checksum ||
                               results[0].calls != results[1].calls))
          throw std::runtime_error("before/after checksum or callback-count mismatch");
      }
    }
  }
}
