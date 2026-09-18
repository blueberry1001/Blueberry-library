#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include "blueberry/graph/clique-enumeration.hpp"
#include "blueberry/graph/maximum-independent-set.hpp"
#include "blueberry/graph/rooted-tree-isomorphism.hpp"
using namespace std;
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? static_cast<unsigned>(strtoul(argv[1], nullptr, 10)) : 20260918;
  mt19937 rng(seed);
  cerr << "seed=" << seed << '\n';
  assert(blueberry::rooted_tree_isomorphism({}).empty());
  assert(blueberry::maximum_independent_set(0, {}).empty());
  blueberry::enumerate_cliques(0, {}, [](const auto&) { assert(false); });
  assert(blueberry::maximum_independent_set(40, {}).size() == 40);
  for (int trial = 0; trial < 350; ++trial) {
    int n = 1 + rng() % 13;
    vector<pair<int, int>> edges;
    vector<unsigned> adj(n);
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
      if (rng() % 3 == 0) { edges.emplace_back(u, v); adj[u] |= 1U << v; adj[v] |= 1U << u; }
    set<unsigned> expected, actual;
    int maximum = 0;
    for (unsigned s = 1; s < (1U << n); ++s) {
      bool clique = true, independent = true;
      for (int v = 0; v < n; ++v) if ((s >> v) & 1) {
        independent &= !(s & adj[v]);
        clique &= (s & ~(adj[v] | (1U << v))) == 0;
      }
      if (clique) expected.insert(s);
      if (independent) maximum = max(maximum, popcount(s));
    }
    blueberry::enumerate_cliques(n, edges, [&](const auto& c) {
      unsigned s = 0; for (int v : c) s |= 1U << v;
      assert(popcount(s) == static_cast<int>(c.size()));
      assert(actual.insert(s).second);
    });
    auto answer = blueberry::maximum_independent_set(n, edges);
    unsigned mask = 0; for (int v : answer) mask |= 1U << v;
    for (int v : answer) assert(!(mask & adj[v]));
    if (expected != actual || maximum != static_cast<int>(answer.size())) {
      cerr << "trial=" << trial << " n=" << n << " expected MIS=" << maximum << " actual=" << answer.size() << '\n';
      for (auto [u,v] : edges) cerr << u << ' ' << v << '\n';
      return 1;
    }
    vector<int> p(n, -1), perm(n); iota(perm.begin(), perm.end(), 0);
    shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; ++i) p[perm[i]] = perm[rng() % i];
    vector<vector<int>> children(n);
    for (int v = 0; v < n; ++v) if (p[v] >= 0) children[p[v]].push_back(v);
    auto canonical = [&](auto&& self, int v) -> string {
      vector<string> parts; for (int u : children[v]) parts.push_back(self(self, u));
      sort(parts.begin(), parts.end()); string result = "(";
      for (auto& part : parts) result += part;
      return result + ")";
    };
    auto ids = blueberry::rooted_tree_isomorphism(p);
    for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v)
      assert((ids[u] == ids[v]) == (canonical(canonical, u) == canonical(canonical, v)));
  }
  vector<int> path(200000); iota(path.begin(), path.end(), -1);
  auto ids = blueberry::rooted_tree_isomorphism(path);
  assert(set<int>(ids.begin(), ids.end()).size() == path.size());
}
