#include "blueberry/graph/heavy-light-decomposition.hpp"

#include <atcoder/fenwicktree>
#include <atcoder/lazysegtree>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<int>>;
using HLD = blueberry::HeavyLightDecomposition;

namespace {
std::uint64_t seed, checks = 0, trees = 0, pairs = 0;
const Graph* current_tree = nullptr;
std::string label;
int current_root = -1, current_u = -1, current_v = -1, current_k = -1, current_step = -1;

[[noreturn]] void fail(const char* message) {
  std::cerr << "HLD seed=" << seed << " tree=" << trees << " label=" << label
            << " root=" << current_root << " u=" << current_u
            << " v=" << current_v << " k=" << current_k
            << " step=" << current_step << " check=" << message << '\n';
  if (current_tree != nullptr) {
    std::cerr << "n=" << current_tree->size() << '\n';
    if (current_tree->size() <= 100) {
      for (int v = 0; v < static_cast<int>(current_tree->size()); ++v) {
        std::cerr << v << ':';
        for (int to : (*current_tree)[v]) std::cerr << ' ' << to;
        std::cerr << '\n';
      }
    }
  }
  std::exit(1);
}

void require(bool condition, const char* message) {
  ++checks;
  if (!condition) fail(message);
}

template <class A, class B>
void same_scalar(A actual, B expected, const char* message) {
  ++checks;
  if (actual == expected) return;
  std::cerr << "expected=" << expected << " actual=" << actual << '\n';
  fail(message);
}

void same_vertices(const std::vector<int>& actual, const std::vector<int>& expected,
                   const char* message) {
  ++checks;
  if (actual == expected) return;
  for (const auto& [name, values] :
       {std::pair{"expected", &expected}, std::pair{"actual", &actual}}) {
    std::cerr << name << " size=" << values->size() << ':';
    for (std::size_t i = 0; i < std::min<std::size_t>(values->size(), 100); ++i)
      std::cerr << ' ' << (*values)[i];
    std::cerr << '\n';
  }
  fail(message);
}

// Rooting and path answers use only tree edges, never HLD positions or heads.
struct Reference {
  std::vector<int> parent, depth, size;

  Reference(const Graph& graph, int root)
      : parent(graph.size(), -1), depth(graph.size()), size(graph.size(), 1) {
    std::vector<int> queue{root};
    parent[root] = root;
    for (std::size_t i = 0; i < queue.size(); ++i) {
      const int v = queue[i];
      for (int to : graph[v]) {
        if (parent[to] != -1) continue;
        parent[to] = v;
        depth[to] = depth[v] + 1;
        queue.push_back(to);
      }
    }
    require(queue.size() == graph.size(), "reference connected tree");
    for (std::size_t i = queue.size(); i-- > 1;)
      size[parent[queue[i]]] += size[queue[i]];
  }

  int lca(int u, int v) const {
    while (depth[u] > depth[v]) u = parent[u];
    while (depth[v] > depth[u]) v = parent[v];
    while (u != v) u = parent[u], v = parent[v];
    return u;
  }

  bool ancestor(int u, int v) const {
    while (depth[v] > depth[u]) v = parent[v];
    return u == v;
  }

  std::vector<int> path(int u, int v) const {
    const int a = lca(u, v);
    std::vector<int> answer, tail;
    for (; u != a; u = parent[u]) answer.push_back(u);
    answer.push_back(a);
    for (; v != a; v = parent[v]) tail.push_back(v);
    answer.insert(answer.end(), tail.rbegin(), tail.rend());
    return answer;
  }
};

struct Interval {
  int left, right;
  bool reversed;
  bool operator==(const Interval&) const = default;
};

std::vector<int> expand(const HLD& hld, const std::vector<Interval>& intervals) {
  std::vector<int> vertices;
  for (const auto& [left, right, reversed] : intervals) {
    require(0 <= left && left < right && right <= hld.size(),
            "callback must be a nonempty half-open interval");
    if (reversed) {
      for (int i = right; i-- > left;) vertices.push_back(hld.vertex_at(i));
    } else {
      for (int i = left; i < right; ++i) vertices.push_back(hld.vertex_at(i));
    }
  }
  return vertices;
}

void check_pair(const HLD& hld, const Reference& ref, int u, int v,
                bool every_jump = true) {
  current_u = u;
  current_v = v;
  current_k = -1;
  ++pairs;
  const auto vertices = ref.path(u, v);
  const int length = static_cast<int>(vertices.size()) - 1;
  same_scalar(hld.lca(u, v), ref.lca(u, v), "lca");
  same_scalar(hld.distance(u, v), length, "distance");
  if (every_jump) {
    for (int k = 0; k <= length; ++k) {
      current_k = k;
      same_scalar(hld.jump(u, v, k), vertices[k], "jump every position");
    }
  } else {
    for (int k : {0, std::min(1, length), length / 2, std::max(0, length - 1), length}) {
      current_k = k;
      same_scalar(hld.jump(u, v, k), vertices[k], "large jump boundary");
    }
  }
  current_k = -1;
  std::vector<int> edges;
  for (int i = 1; i <= length; ++i) {
    const int a = vertices[i - 1], b = vertices[i];
    const int child = ref.depth[a] > ref.depth[b] ? a : b;
    edges.push_back(child);
    require(hld.edge_index(a, b) == hld.in(child), "edge_index forward");
    require(hld.edge_index(b, a) == hld.in(child), "edge_index reverse");
  }
  for (bool edge : {false, true}) {
    std::vector<Interval> trace, alias, default_trace;
    hld.path_query(u, v, [&](int l, int r, bool rev) { trace.push_back({l, r, rev}); }, edge);
    auto record = [&](int l, int r, bool rev) { alias.push_back({l, r, rev}); };
    if (edge) hld.edge_query(u, v, record);
    else hld.vertex_query(u, v, record);
    require(alias == trace, "path alias preserves callback sequence and directions");
    if (!edge) {
      hld.path_query(u, v, [&](int l, int r, bool rev) { default_trace.push_back({l, r, rev}); });
      require(default_trace == trace, "default path_query is vertex mode");
    }
    same_vertices(expand(hld, trace), edge ? edges : vertices, "ordered u-to-v path");
    require(trace.size() <= static_cast<std::size_t>(2 * std::bit_width(static_cast<unsigned>(hld.size())) + 1),
            "logarithmic interval count");
    std::vector<std::pair<int, int>> expected, two, two_alias, two_default;
    for (const auto& segment : trace) expected.emplace_back(segment.left, segment.right);
    hld.path_query(u, v, [&](int l, int r) { two.emplace_back(l, r); }, edge);
    auto record_two = [&](int l, int r) { two_alias.emplace_back(l, r); };
    if (edge) hld.edge_query(u, v, record_two);
    else hld.vertex_query(u, v, record_two);
    require(two == expected && two_alias == expected,
            "two-argument callbacks preserve the ordered interval sequence");
    if (!edge) {
      hld.path_query(u, v, [&](int l, int r) { two_default.emplace_back(l, r); });
      require(two_default == expected, "two-argument default vertex mode");
    }
  }
}

// Forwarding must accept noncopyable lvalues/rvalues and prefer the three-
// argument overload when a callback can also accept two arguments.
struct Callback {
  std::vector<Interval>& trace;
  explicit Callback(std::vector<Interval>& value) : trace(value) {}
  Callback(const Callback&) = delete;
  Callback(Callback&&) = delete;
  void operator()(int, int) { fail("three-argument callback overload was not preferred"); }
  void operator()(int l, int r, bool rev) { trace.push_back({l, r, rev}); }
};

void check_small(const Graph& graph, int root, std::mt19937_64& rng) {
  current_tree = &graph;
  current_root = root;
  current_u = current_v = current_k = current_step = -1;
  ++trees;
  const Reference ref(graph, root);
  const Graph original = graph;
  const blueberry::HLD hld(graph, root);
  const int n = static_cast<int>(graph.size());
  require(graph == original, "constructor preserves adjacency lists");
  require(hld.size() == n && hld.root() == root, "size and arbitrary root");
  std::vector<int> seen(n);
  for (int v = 0; v < n; ++v) {
    current_u = current_v = v;
    require(hld.parent(v) == ref.parent[v] && hld.depth(v) == ref.depth[v], "parent/depth");
    const int position = hld.in(v);
    require(0 <= position && position < n, "in range");
    ++seen[position];
    require(hld.vertex_at(position) == v, "vertex_at/in inverse");
    require(hld.subtree_size(v) == ref.size[v], "subtree size");
    require(hld.out(v) == position + ref.size[v], "out is exclusive");
    const int head = hld.head(v);
    require(0 <= head && head < n && ref.ancestor(head, v), "head is an ancestor");
    require(hld.head(head) == head && position - hld.in(head) == ref.depth[v] - ref.depth[head],
            "chain head and contiguous chain positions");
    if (v != head)
      require(hld.head(ref.parent[v]) == head && hld.in(ref.parent[v]) + 1 == position,
              "parent on same heavy chain");
    for (int k = 0, ancestor = v; k <= ref.depth[v]; ++k, ancestor = ref.parent[ancestor]) {
      current_k = k;
      same_scalar(hld.kth_ancestor(v, k), ancestor, "kth_ancestor every valid k");
      same_scalar(hld.kth(v, k), ancestor, "kth alias");
    }
    current_k = -1;
    std::vector<Interval> single;
    hld.vertex_query(v, Callback(single));
    require(single == std::vector<Interval>{{position, position + 1, false}}, "single vertex callback");
    std::vector<std::pair<int, int>> single_two;
    hld.vertex_query(v, [&](int l, int r) { single_two.emplace_back(l, r); });
    require(single_two == std::vector<std::pair<int, int>>{{position, position + 1}},
            "single vertex two-argument callback");
    for (bool edge : {false, true}) {
      const std::pair<int, int> expected_range{position + static_cast<int>(edge), hld.out(v)};
      require(hld.subtree_range(v, edge) == expected_range, "subtree range");
      if (!edge) require(hld.subtree_range(v) == expected_range, "default subtree range");
      std::vector<Interval> trace;
      Callback callback(trace);
      hld.subtree_query(v, callback, edge);
      const std::vector<Interval> expected_trace = expected_range.first < expected_range.second
          ? std::vector<Interval>{{expected_range.first, expected_range.second, false}}
          : std::vector<Interval>{};
      require(trace == expected_trace, "subtree emits one forward interval or no callback");
      std::vector<std::pair<int, int>> two;
      hld.subtree_query(v, [&](int l, int r) { two.emplace_back(l, r); }, edge);
      require(two.size() == trace.size(), "subtree two-argument callback count");
      if (!two.empty()) require(two[0] == expected_range, "subtree two-argument interval");
      if (!edge) {
        std::vector<Interval> default_trace;
        hld.subtree_query(v, Callback(default_trace));
        require(default_trace == trace, "default subtree_query is vertex mode");
        std::vector<std::pair<int, int>> default_two;
        hld.subtree_query(v, [&](int l, int r) { default_two.emplace_back(l, r); });
        require(default_two == two, "default subtree_query two-argument mode");
      }
      std::vector<int> expected_vertices;
      for (int x = 0; x < n; ++x) {
        const bool ancestor = ref.ancestor(v, x);
        require(hld.is_ancestor(v, x) == ancestor, "is_ancestor independent reference");
        if (ancestor && (!edge || x != v)) expected_vertices.push_back(x);
      }
      auto actual = expand(hld, trace);
      std::sort(actual.begin(), actual.end());
      same_vertices(actual, expected_vertices, "subtree vertex/edge membership");
    }
  }
  require(std::all_of(seen.begin(), seen.end(), [](int count) { return count == 1; }), "Euler permutation");
  if (n <= 24) {
    for (int u = 0; u < n; ++u)
      for (int v = 0; v < n; ++v) check_pair(hld, ref, u, v);
  } else {
    for (int v = 0; v < n; ++v) {
      check_pair(hld, ref, v, v);
      check_pair(hld, ref, root, v);
      check_pair(hld, ref, v, root);
    }
    for (int query = 0; query < 120; ++query)
      check_pair(hld, ref, static_cast<int>(rng() % n), static_cast<int>(rng() % n));
  }
  std::vector<Interval> noncopyable_trace;
  Callback callback(noncopyable_trace);
  hld.path_query(0, n - 1, callback);
  same_vertices(expand(hld, noncopyable_trace), ref.path(0, n - 1), "noncopyable lvalue callback");
  noncopyable_trace.clear();
  hld.vertex_query(n - 1, 0, Callback(noncopyable_trace));
  same_vertices(expand(hld, noncopyable_trace), ref.path(n - 1, 0), "noncopyable rvalue callback");
}

Graph children_only(const Graph& graph, int root) {
  const Reference ref(graph, root);
  Graph children(graph.size());
  for (int v = 0; v < static_cast<int>(graph.size()); ++v)
    for (int to : graph[v])
      if (to != root && ref.parent[to] == v) children[v].push_back(to);
  return children;
}

void check_both(Graph graph, int root, std::mt19937_64& rng, const std::string& name) {
  for (auto& neighbors : graph) std::shuffle(neighbors.begin(), neighbors.end(), rng);
  label = name + "/undirected";
  check_small(graph, root, rng);
  Graph directed = children_only(graph, root);
  label = name + "/directed-child-lists";
  check_small(directed, root, rng);
}

Graph prufer_tree(int n, int code) {
  Graph graph(n);
  if (n == 1) return graph;
  std::vector<int> sequence(std::max(0, n - 2)), degree(n, 1);
  for (int& v : sequence) v = code % n, code /= n, ++degree[v];
  for (int v : sequence) {
    int leaf = 0;
    while (degree[leaf] != 1) ++leaf;
    graph[v].push_back(leaf);
    graph[leaf].push_back(v);
    --degree[v];
    --degree[leaf];
  }
  int a = 0, b = n - 1;
  while (degree[a] != 1) ++a;
  while (degree[b] != 1) --b;
  graph[a].push_back(b);
  graph[b].push_back(a);
  return graph;
}

struct SumLength { long long sum; int length; };
SumLength sum_op(SumLength a, SumLength b) { return {a.sum + b.sum, a.length + b.length}; }
SumLength sum_identity() { return {0, 0}; }
SumLength add_mapping(long long delta, SumLength value) {
  return {value.sum + delta * value.length, value.length};
}
long long add_composition(long long a, long long b) { return a + b; }
long long add_identity() { return 0; }

void acl_edge_queries(std::mt19937_64& rng) {
  for (int trial = 0; trial < 24; ++trial) {
    const int n = trial == 0 ? 1 : 2 + static_cast<int>(rng() % 39);
    Graph graph(n);
    for (int v = 1; v < n; ++v) {
      const int parent = static_cast<int>(rng() % v);
      graph[v].push_back(parent);
      graph[parent].push_back(v);
    }
    const int root = static_cast<int>(rng() % n);
    // Establish diagnostics before the reference constructor checks connectivity.
    label = "ACL-edge-integration-" + std::to_string(trial);
    current_tree = &graph;
    current_root = root;
    ++trees;
    if (trial % 2) graph = children_only(graph, root);
    const Reference ref(graph, root);
    const HLD hld(graph, root);
    std::vector<long long> point_value(n), range_value(n);
    std::vector<SumLength> initial(n, {0, 0});
    constexpr long long root_sentinel = 123456789;
    // The root has no incoming edge: including it in a range must be observable.
    initial[hld.in(root)] = {root_sentinel, 1};
    atcoder::fenwick_tree<long long> point(n);
    for (int v = 0; v < n; ++v) {
      if (v == root) continue;
      point_value[v] = range_value[v] = static_cast<int>(rng() % 21) - 10;
      point.add(hld.edge_index(v, ref.parent[v]), point_value[v]);
      initial[hld.in(v)] = {range_value[v], 1};
    }
    atcoder::lazy_segtree<SumLength, sum_op, sum_identity, long long,
                          add_mapping, add_composition, add_identity> range(initial);
    for (int step = 0; step < 160; ++step) {
      current_step = step;
      const int changed = static_cast<int>(rng() % n);
      const long long delta = static_cast<int>(rng() % 21) - 10;
      if (step % 2 == 0) {
        if (changed != root) {
          point_value[changed] += delta;
          point.add(hld.edge_index(ref.parent[changed], changed), delta);
        }
      } else {
        hld.edge_query(root, changed, [&](int left, int right) { range.apply(left, right, delta); });
        for (int v = changed; v != root; v = ref.parent[v]) range_value[v] += delta;
      }
      current_u = step % 4 == 0 ? root : static_cast<int>(rng() % n);
      current_v = step % 17 == 0 ? current_u : static_cast<int>(rng() % n);
      long long actual_point = 0, actual_range = 0;
      hld.edge_query(current_u, current_v, [&](int left, int right) {
        actual_point += point.sum(left, right);
      });
      hld.path_query(current_u, current_v, [&](int left, int right) {
        actual_range += range.prod(left, right).sum;
      }, true);
      long long expected_point = 0, expected_range = 0;
      const auto path = ref.path(current_u, current_v);
      for (std::size_t i = 1; i < path.size(); ++i) {
        const int a = path[i - 1], b = path[i];
        const int child = ref.depth[a] > ref.depth[b] ? a : b;
        expected_point += point_value[child];
        expected_range += range_value[child];
      }
      same_scalar(actual_point, expected_point, "ACL edge point updates/path sums");
      same_scalar(actual_range, expected_range, "ACL root-path edge additions/path sums");
      same_scalar(range.get(hld.in(root)).sum, root_sentinel,
                  "edge additions leave root dummy untouched");
    }
  }
  current_step = -1;
  current_tree = nullptr;
}

void large_chains() {
  constexpr int n = 200000;
  for (int mode = 0; mode < 3; ++mode) {
    Graph graph(n);
    const int root = mode == 0 ? 0 : mode == 1 ? n - 1 : n / 2;
    for (int v = 1; v < n; ++v) {
      if (mode != 1) graph[v - 1].push_back(v);
      if (mode != 0) graph[v].push_back(v - 1);
    }
    label = "large-chain-mode-" + std::to_string(mode);
    current_tree = &graph;
    current_root = root;
    ++trees;
    const Reference ref(graph, root);
    const HLD hld(graph, root);
    require(hld.size() == n && hld.root() == root, "large size/root");
    for (int v = 0; v < n; ++v) {
      current_u = v;
      require(hld.parent(v) == ref.parent[v] && hld.depth(v) == ref.depth[v], "large parent/depth");
      require(hld.subtree_size(v) == ref.size[v], "large subtree_size");
      require(hld.vertex_at(hld.in(v)) == v, "large Euler inverse");
      require(hld.out(v) - hld.in(v) == ref.size[v], "large subtree interval");
    }
    require(hld.kth(0, ref.depth[0]) == root && hld.kth(n - 1, ref.depth[n - 1]) == root,
            "large kth to arbitrary root");
    for (auto [u, v] : {std::pair{0, n - 1}, std::pair{n - 1, 0},
                       std::pair{root, root}, std::pair{root, n - 1}})
      check_pair(hld, ref, u, v, false);
    for (int v : {0, n - 1}) {
      if (ref.size[v] != 1) continue;
      int calls = 0;
      hld.subtree_query(v, [&](int, int, bool) { ++calls; }, true);
      require(calls == 0, "large leaf has no subtree edges");
    }
  }
  current_tree = nullptr;
}
}  // namespace

int main(int argc, char** argv) {
  const char* configured = argc > 1 ? argv[1] : std::getenv("BLUEBERRY_RANDOM_SEED");
  seed = configured ? std::strtoull(configured, nullptr, 10) : 20261004;
  std::mt19937_64 rng(seed);
  const HLD default_root(Graph(1));
  require(default_root.root() == 0 && default_root.parent(0) == 0,
          "default constructor root and self-parent");
  for (int n = 1; n <= 5; ++n) {
    int count = 1;
    for (int i = 0; i < n - 2; ++i) count *= n;
    for (int code = 0; code < count; ++code) {
      const auto graph = prufer_tree(n, code);
      for (int root = 0; root < n; ++root)
        check_both(graph, root, rng, "exhaustive-prufer-" + std::to_string(n) + "-" + std::to_string(code));
    }
  }
  for (int trial = 0; trial < 100; ++trial) {
    const int n = 6 + static_cast<int>(rng() % 75);
    Graph graph(n);
    std::vector<int> permutation(n);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), rng);
    for (int v = 1; v < n; ++v) {
      const int parent = trial % 5 == 0 ? 0 : trial % 5 == 1 ? v - 1
          : trial % 5 == 2 ? (v - 1) / 2 : trial % 5 == 3 ? (v < n / 2 ? v - 1 : n / 2 - 1)
          : static_cast<int>(rng() % v);
      graph[permutation[v]].push_back(permutation[parent]);
      graph[permutation[parent]].push_back(permutation[v]);
    }
    const int root = static_cast<int>(rng() % n);
    check_both(graph, root, rng, "random-shape-" + std::to_string(trial));
  }
  acl_edge_queries(rng);
  large_chains();
  std::cout << "HLD seed=" << seed << " trees=" << trees << " pairs=" << pairs
            << " checks=" << checks << " passed\n";
}
