#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <atcoder/modint>
#include "blueberry/graph/assignment.hpp"
#include "blueberry/graph/complement-components.hpp"
#include "blueberry/graph/count-spanning-trees.hpp"
#include "blueberry/graph/minimum-spanning-forest.hpp"
using namespace std;
using mint = atcoder::modint998244353;
int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? stoul(argv[1]) : 12345;
  mt19937 rng(seed);
  int trial = -1;
  ostringstream context;
  auto check = [&](bool ok, const char* what) {
    if (!ok) { cerr << "seed=" << seed << " trial=" << trial << " failed=" << what << '\n' << context.str(); abort(); }
  };
  check(blueberry::assignment(vector<vector<long long>>{}).first == 0, "empty assignment");
  check(blueberry::complement_components({}).empty(), "empty complement");
  check(blueberry::count_spanning_trees<mint>(0, {}).val() == 1, "empty spanning tree");
  check(blueberry::minimum_spanning_forest<long long>(0, {}).second.empty(), "empty forest");
  for (trial = 0; trial < 300; ++trial) {
    int n = 1 + rng() % 6, m = n + rng() % (8 - n);
    vector<vector<long long>> cost(n, vector<long long>(m));
    for (auto& row : cost) for (auto& x : row) x = static_cast<int>(rng() % 31) - 15;
    context.str(""); context << "matrix " << n << ' ' << m << '\n';
    for (const auto& row : cost) { for (auto x : row) context << x << ' '; context << '\n'; }
    vector<int> p(m); iota(p.begin(), p.end(), 0);
    long long expected = numeric_limits<long long>::max();
    do { long long sum = 0; for (int i = 0; i < n; ++i) sum += cost[i][p[i]]; expected = min(expected, sum); }
    while (next_permutation(p.begin(), p.end()));
    auto [actual, match] = blueberry::assignment(cost);
    context << "expected=" << expected << " actual=" << actual << '\n';
    check(actual == expected, "assignment total");
    long long sum = 0; vector<bool> used(m);
    for (int i = 0; i < n; ++i) { check(0 <= match[i] && match[i] < m && !used[match[i]], "assignment matching"); used[match[i]] = true; sum += cost[i][match[i]]; }
    check(sum == actual, "assignment reconstructed cost");
  }
  for (trial = 0; trial < 700; ++trial) {
    int n = 1 + rng() % 7, m = rng() % 12, root = rng() % n;
    vector<pair<int, int>> edges;
    vector<tuple<int, int, long long>> weighted;
    vector<vector<int>> graph(n), adjacent(n, vector<int>(n));
    atcoder::dsu original(n), complement(n);
    context.str(""); context << "graph " << n << ' ' << m << " root=" << root << '\n';
    for (int i = 0; i < m; ++i) {
      int u = rng() % n, v = rng() % n;
      edges.emplace_back(u, v); weighted.emplace_back(u, v, static_cast<int>(rng() % 31) - 15);
      context << u << ' ' << v << ' ' << get<2>(weighted.back()) << '\n';
      graph[u].push_back(v); graph[v].push_back(u); adjacent[u][v] = adjacent[v][u] = 1; original.merge(u, v);
    }
    for (int u = 0; u < n; ++u) for (int v = 0; v < u; ++v) if (!adjacent[u][v]) complement.merge(u, v);
    vector<int> component(n, -1); auto groups = blueberry::complement_components(graph);
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) for (int v : groups[i]) { check(component[v] == -1, "complement duplicate"); component[v] = i; }
    for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) check(component[u] >= 0 && ((component[u] == component[v]) == complement.same(u, v)), "complement partition");
    int count = 0, directed = 0;
    long long best = numeric_limits<long long>::max();
    const int needed = n - static_cast<int>(original.groups().size());
    for (unsigned mask = 0; mask < (1u << m); ++mask) {
      const int chosen = popcount(mask);
      if (chosen != n - 1 && chosen != needed) continue;
      atcoder::dsu dsu(n); bool acyclic = true; long long sum = 0;
      vector<vector<int>> out(n); vector<int> indegree(n);
      for (int i = 0; i < m; ++i) if (mask >> i & 1) {
        auto [u, v] = edges[i]; if (dsu.same(u, v)) acyclic = false; dsu.merge(u, v);
        sum += get<2>(weighted[i]); out[u].push_back(v); ++indegree[v];
      }
      if (!acyclic) continue;
      if (chosen == needed) best = min(best, sum);
      if (chosen == n - 1) {
        ++count;
        bool ok = indegree[root] == 0;
        for (int v = 0; v < n; ++v) if (v != root) ok &= indegree[v] == 1;
        directed += ok;
      }
    }
    const int got_count = blueberry::count_spanning_trees<mint>(n, edges).val();
    const int got_directed = blueberry::count_directed_spanning_trees<mint>(n, edges, root).val();
    context << "undirected expected=" << count << " actual=" << got_count << " directed expected=" << directed << " actual=" << got_directed << '\n';
    check(got_count == count, "undirected tree count");
    check(got_directed == directed, "directed tree count");
    auto [actual, selected] = blueberry::minimum_spanning_forest(n, weighted);
    context << "forest expected=" << best << " actual=" << actual << '\n';
    check(actual == best && static_cast<int>(selected.size()) == needed, "minimum forest cost/size");
    atcoder::dsu reconstructed(n); long long sum = 0;
    for (int id : selected) { auto [u, v, w] = weighted[id]; check(!reconstructed.same(u, v), "forest cycle"); reconstructed.merge(u, v); sum += w; }
    check(sum == actual, "forest reconstructed cost");
  }
  cout << "graph batch three seed=" << seed << " passed\n";
}
