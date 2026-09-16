#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include "blueberry/graph/biconnected-components.hpp"
#include "blueberry/graph/eulerian-trail.hpp"
#include "blueberry/graph/hopcroft-karp.hpp"

using namespace std;
using Edge = pair<int, int>;
unsigned seed;
vector<Edge> input;
void check(bool ok, const char* message, int expected = 1, int actual = 0) {
  if (ok) return;
  cerr << "seed=" << seed << " " << message << " expected=" << expected << " actual=" << actual << "\nedges:";
  for (auto [u, v] : input) cerr << ' ' << u << ',' << v;
  cerr << '\n';
  exit(1);
}

void matching_test(int l, int r) {
  blueberry::HopcroftKarp hk(l, r, input);
  vector<int> dp(1 << r, -1);
  dp[0] = 0;
  for (int u = 0; u < l; ++u) {
    auto next = dp;
    for (int mask = 0; mask < (1 << r); ++mask) if (dp[mask] >= 0) {
      for (auto [a, b] : input) if (a == u && !(mask >> b & 1))
        next[mask | (1 << b)] = max(next[mask | (1 << b)], dp[mask] + 1);
    }
    dp = std::move(next);
  }
  const int optimum = *max_element(dp.begin(), dp.end());
  check(hk.size() == optimum, "matching size", optimum, hk.size());
  auto pairs = hk.pairs();
  check(static_cast<int>(pairs.size()) == optimum, "pairs size", optimum, pairs.size());
  vector<int> used_l(l), used_r(r);
  for (auto [u, v] : pairs) {
    check(0 <= u && u < l && 0 <= v && v < r, "pair bounds");
    check(!used_l[u]++ && !used_r[v]++, "duplicate pair endpoint");
    check(find(input.begin(), input.end(), Edge{u, v}) != input.end(), "nonexistent matching edge");
    check(hk.left_match(u) == v && hk.right_match(v) == u, "mate symmetry");
  }
  for (int u = 0; u < l; ++u) check((hk.left_match(u) != -1) == !!used_l[u], "unmatched left");
  for (int v = 0; v < r; ++v) check((hk.right_match(v) != -1) == !!used_r[v], "unmatched right");
  auto [cl, cr] = hk.min_vertex_cover();
  check(static_cast<int>(cl.size() + cr.size()) == optimum, "minimum cover size");
  vector<bool> covered_l(l), covered_r(r);
  for (int u : cl) { check(0 <= u && u < l && !covered_l[u], "cover left"); covered_l[u] = true; }
  for (int v : cr) { check(0 <= v && v < r && !covered_r[v], "cover right"); covered_r[v] = true; }
  for (auto [u, v] : input) check(covered_l[u] || covered_r[v], "uncovered edge");
}

bool connected(int mask, const vector<vector<int>>& graph) {
  if (!mask) return true;
  int seen = 0;
  vector<int> todo{__builtin_ctz(static_cast<unsigned>(mask))};
  while (!todo.empty()) {
    int v = todo.back(); todo.pop_back();
    if (seen >> v & 1) continue;
    seen |= 1 << v;
    for (int to : graph[v]) if ((mask >> to & 1) && !(seen >> to & 1)) todo.push_back(to);
  }
  return seen == mask;
}

void blocks_test(int n) {
  vector<vector<int>> g(n);
  for (auto [u, v] : input) { g[u].push_back(v); g[v].push_back(u); }
  vector<int> candidates, expected, actual;
  for (int mask = 1; mask < (1 << n); ++mask) {
    bool ok = connected(mask, g);
    for (int v = 0; v < n; ++v) if (mask >> v & 1) ok &= connected(mask ^ (1 << v), g);
    if (ok) candidates.push_back(mask);
  }
  for (int mask : candidates) {
    bool maximal = true;
    for (int other : candidates) if (mask != other && (mask & other) == mask) maximal = false;
    if (maximal) expected.push_back(mask);
  }
  blueberry::BiconnectedComponents bc(n, input);
  for (const auto& group : bc.groups()) {
    int mask = 0;
    check(!group.empty(), "empty block");
    for (int v : group) {
      check(0 <= v && v < n && !(mask >> v & 1), "block duplicate/bounds");
      mask |= 1 << v;
    }
    actual.push_back(mask);
  }
  sort(expected.begin(), expected.end()); sort(actual.begin(), actual.end());
  check(actual == expected, "maximal vertex blocks");
  auto forest = bc.block_cut_tree();
  check(forest.size() == n + bc.groups().size(), "forest size");
  vector<int> parent(forest.size()); iota(parent.begin(), parent.end(), 0);
  auto root = [&](int v) { while (v != parent[v]) v = parent[v]; return v; };
  for (size_t b = 0; b < bc.groups().size(); ++b) {
    auto vertices = forest[n + b], group = bc.groups()[b];
    sort(vertices.begin(), vertices.end()); sort(group.begin(), group.end());
    check(vertices == group, "forest block membership");
    for (int v : vertices) {
      check(count(forest[v].begin(), forest[v].end(), n + b) == 1, "forest symmetry");
      int a = root(v), c = root(n + b);
      check(a != c, "incidence forest cycle"); parent[a] = c;
    }
  }
  for (auto [u, v] : input) check(root(u) == root(v), "forest connectivity");
}

template<bool Directed>
void trail_test(int n, int start) {
  const int m = input.size();
  auto dfs = [&](auto&& self, int v, int mask) -> bool {
    if (mask == (1 << m) - 1) return true;
    for (int id = 0; id < m; ++id) if (!(mask >> id & 1)) {
      auto [a, b] = input[id];
      if (a == v && self(self, b, mask | (1 << id))) return true;
      if (!Directed && b == v && self(self, a, mask | (1 << id))) return true;
    }
    return false;
  };
  bool expected = n == 0;
  for (int v = 0; v < n; ++v) if (start == -1 || start == v) expected |= dfs(dfs, v, 0);
  blueberry::EulerianTrail<Directed> trail(n, input, start);
  check(trail.exists() == expected, "trail existence", expected, trail.exists());
  if (!expected) { check(trail.vertices().empty() && trail.edges().empty(), "failed trail empty"); return; }
  const auto& vs = trail.vertices(); const auto& es = trail.edges();
  check(es.size() == input.size() && vs.size() == static_cast<size_t>(n ? m + 1 : 0), "trail lengths");
  if (start >= 0) check(vs.front() == start, "explicit start");
  vector<bool> seen(m);
  for (int i = 0; i < m; ++i) {
    int id = es[i]; check(0 <= id && id < m && !seen[id], "trail edge repeated/bounds"); seen[id] = true;
    auto [a, b] = input[id];
    check((a == vs[i] && b == vs[i + 1]) || (!Directed && b == vs[i] && a == vs[i + 1]), "trail orientation");
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? stoul(argv[1]) : 1;
  mt19937 rng(seed);
  for (int iter = 0; iter < 250; ++iter) {
    int l = rng() % 6, r = rng() % 6;
    input.clear();
    if (l && r) for (unsigned i = rng() % 25; i--; ) input.emplace_back(rng() % l, rng() % r);
    matching_test(l, r);
    int n = rng() % 8;
    input.clear();
    if (n) for (unsigned i = rng() % 15; i--; ) { int a = rng() % n, b = rng() % n; if (a != b) input.emplace_back(a, b); }
    blocks_test(n);
    n = rng() % 5;
    input.clear();
    if (n) for (unsigned i = rng() % 9; i--; ) input.emplace_back(rng() % n, rng() % n);
    for (int start = -1; start < n; ++start) { trail_test<false>(n, start); trail_test<true>(n, start); }
  }
  if (seed == 1) {
    const int n = 200000;
    input.clear();
    for (int i = 0; i + 1 < n; ++i) { input.emplace_back(i, i + 1); input.emplace_back(i, i); }
    input.emplace_back(n - 1, n - 1);
    blueberry::HopcroftKarp matching(n, n, input);
    check(matching.size() == n, "long alternating path", n, matching.size());
    input.clear(); for (int i = 1; i < n; ++i) input.emplace_back(i - 1, i);
    blueberry::BiconnectedComponents blocks(n, input);
    check(blocks.groups().size() == n - 1, "deep block path");
    blueberry::EulerianTrail<true> d(n, input);
    blueberry::EulerianTrail<false> u(n, input);
    check(d.exists() && u.exists() && d.edges().size() == input.size(), "deep trail");
  }
  cout << "PASS graph-expansion seed=" << seed << '\n';
}
