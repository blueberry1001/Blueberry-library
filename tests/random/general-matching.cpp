#include <algorithm>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/graph/general-matching.hpp"
#include "blueberry/graph/hopcroft-karp.hpp"

using namespace std;
using Edge = pair<int, int>;

unsigned long long seed;
int cases = 0;

[[noreturn]] void fail(int n, const vector<Edge>& edges, int expected,
                      const vector<int>& actual, const string& reason) {
  cerr << "seed=" << seed << " case=" << cases << " reason=" << reason
       << " expected=" << expected << "\ninput:\n" << n << ' ' << edges.size() << '\n';
  for (auto [u, v] : edges) cerr << u << ' ' << v << '\n';
  cerr << "actual mate:";
  for (int v : actual) cerr << ' ' << v;
  cerr << '\n';
  exit(1);
}

// Enumerate leaving the smallest remaining vertex unmatched or matching it to a neighbor.
int brute(int n, const vector<Edge>& edges) {
  vector<unsigned> adj(n);
  for (auto [u, v] : edges) if (u != v) {
    adj[u] |= 1U << v;
    adj[v] |= 1U << u;
  }
  vector<int> memo(1U << n, -1);
  memo[0] = 0;
  auto solve = [&](auto&& self, unsigned mask) -> int {
    int& best = memo[mask];
    if (best != -1) return best;
    const int u = countr_zero(mask);
    const unsigned rest = mask & (mask - 1);
    best = self(self, rest);
    for (unsigned neighbors = adj[u] & rest; neighbors; neighbors &= neighbors - 1) {
      const int v = countr_zero(neighbors);
      best = max(best, 1 + self(self, rest & ~(1U << v)));
    }
    return best;
  };
  return solve(solve, (1U << n) - 1);
}

void check(int n, const vector<Edge>& edges, int expected = -1) {
  ++cases;
  if (expected < 0) expected = brute(n, edges);
  const auto original = edges;
  const auto mate = blueberry::general_matching(n, edges);
  if (edges != original) fail(n, edges, expected, mate, "modified input");
  if (mate.size() != static_cast<size_t>(n)) fail(n, edges, expected, mate, "wrong result size");
  vector<vector<bool>> adj(n, vector<bool>(n));
  for (auto [u, v] : edges) if (u != v) adj[u][v] = adj[v][u] = true;
  int count = 0;
  for (int u = 0; u < n; ++u) {
    const int v = mate[u];
    if (v == -1) continue;
    if (v < 0 || v >= n || v == u || mate[v] != u || !adj[u][v])
      fail(n, edges, expected, mate, "invalid matching edge");
    count += u < v;
  }
  if (count != expected) fail(n, edges, expected, mate, "nonmaximum matching");
}

int main(int argc, char** argv) {
  const char* configured_seed = argc > 1 ? argv[1] : getenv("BLUEBERRY_RANDOM_SEED");
  seed = configured_seed ? strtoull(configured_seed, nullptr, 10) : 20261003;
  mt19937_64 rng(seed);
  auto random_int = [&](int limit) { return static_cast<int>(rng() % static_cast<unsigned>(limit)); };
  cerr << "seed=" << seed << '\n';

  // All 33,868 labelled simple graphs on zero through six vertices.
  for (int n = 0; n <= 6; ++n) {
    vector<Edge> possible;
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) possible.emplace_back(u, v);
    for (unsigned mask = 0; mask < (1U << possible.size()); ++mask) {
      vector<Edge> edges;
      for (size_t i = 0; i < possible.size(); ++i) if ((mask >> i) & 1U) edges.push_back(possible[i]);
      check(n, edges);
    }
  }

  // The augmenting path must pass through a triangle after greedy initialization.
  check(6, {{0, 1}, {1, 2}, {2, 0}, {1, 3}, {3, 4}, {4, 5}}, 3);
  check(8, {{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 4}, {3, 4},
            {3, 5}, {4, 6}, {5, 6}, {6, 7}}, 4);
  check(1, {{0, 0}, {0, 0}}, 0);
  check(2, {{0, 0}, {1, 1}, {0, 1}, {1, 0}, {0, 1}}, 1);
  check(1500, {}, 0);

  for (int n : {1, 2, 3, 50, 99, 500, 501}) {
    vector<Edge> path, star, complete, odd_components, nested;
    for (int v = 1; v < n; ++v) {
      path.emplace_back(v - 1, v);
      star.emplace_back(0, v);
      for (int u = 0; u < v; ++u) complete.emplace_back(u, v);
    }
    check(n, path, n / 2);
    check(n, star, n > 1 ? 1 : 0);
    check(n, complete, n / 2);
    if (n > 2) path.emplace_back(n - 1, 0);
    check(n, path, n / 2);
    for (int v = 0; v + 2 < n; v += 3) {
      odd_components.emplace_back(v, v + 1);
      odd_components.emplace_back(v + 1, v + 2);
      odd_components.emplace_back(v + 2, v);
    }
    check(n, odd_components, n / 3);
    // Successive contractions absorb the previously contracted triangle.
    for (int v = 0; v + 2 < n; v += 2) {
      nested.emplace_back(v, v + 1);
      nested.emplace_back(v + 1, v + 2);
      nested.emplace_back(v + 2, v);
    }
    check(n, nested, (n - 1) / 2);
    if (n >= 4 && n % 2 == 0) {
      // Two exposed endpoints force an augmenting path through nested blossoms.
      nested.emplace_back(0, n - 1);
      check(n, nested, n / 2);
    }
  }

  for (int trial = 0; trial < 700; ++trial) {
    const int n = random_int(17), density = random_int(101);
    vector<Edge> edges;
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
      if (random_int(100) < density) edges.emplace_back(u, v);
    shuffle(edges.begin(), edges.end(), rng);
    const int expected = brute(n, edges);
    check(n, edges, expected);
    vector<Edge> multigraph = edges;
    for (auto [u, v] : edges) if (random_int(2)) {
      multigraph.emplace_back(v, u);
      multigraph.emplace_back(u, v);
    }
    for (int u = 0; u < n; ++u) multigraph.emplace_back(u, u);
    shuffle(multigraph.begin(), multigraph.end(), rng);
    check(n, multigraph, expected);
    vector<int> perm(n);
    iota(perm.begin(), perm.end(), 0);
    shuffle(perm.begin(), perm.end(), rng);
    for (auto& [u, v] : multigraph) {
      u = perm[u];
      v = perm[v];
      if (random_int(2)) swap(u, v);
    }
    check(n, multigraph, expected);
  }

  for (int trial = 0; trial < 50; ++trial) {
    const int half = 1 + random_int(250), n = 2 * half;
    vector<Edge> planted;
    vector<int> perm(n);
    iota(perm.begin(), perm.end(), 0);
    shuffle(perm.begin(), perm.end(), rng);
    for (int v = 0; v < n; v += 2) planted.emplace_back(perm[v], perm[v + 1]);
    for (int i = 0; i < 5 * n; ++i) planted.emplace_back(random_int(n), random_int(n));
    shuffle(planted.begin(), planted.end(), rng);
    check(n, planted, half);

    const int left = 1 + random_int(80), right = 1 + random_int(80);
    vector<Edge> bipartite, general;
    for (int i = 0; i < left + right; ++i) {
      const int u = random_int(left), v = random_int(right);
      bipartite.emplace_back(u, v);
      general.emplace_back(u, left + v);
    }
    check(left + right, general, blueberry::HopcroftKarp(left, right, bipartite).size());
  }
  cerr << "general_matching: " << cases << " cases passed\n";
}
