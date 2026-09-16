#define PROBLEM "https://judge.yosupo.jp/problem/tree_path_composite_sum"

#include <iostream>
#include <utility>
#include <vector>

#include <atcoder/modint>

#include "blueberry/graph/rerooting.hpp"

using namespace std;
using mint = atcoder::modint998244353;

struct State {
  mint sum, count;
};

// A component stores (sum, count); applying bx+c changes its sum to b*sum+c*count.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n;
  cin >> n;
  vector<mint> a(n), b(n - 1), c(n - 1);
  for (auto& value : a) {
    int x;
    cin >> x;
    value = x;
  }
  vector<pair<int, int>> edges(n - 1);
  for (int id = 0; id < n - 1; ++id) {
    int slope, intercept;
    cin >> edges[id].first >> edges[id].second >> slope >> intercept;
    b[id] = slope;
    c[id] = intercept;
  }
  const auto result = blueberry::rerooting(n, edges, State{0, 0},
      [](State x, State y) { return State{x.sum + y.sum, x.count + y.count}; },
      [&](State value, int v) { return State{value.sum + a[v], value.count + 1}; },
      [&](State value, int id, int, int) {
        return State{b[id] * value.sum + c[id] * value.count, value.count};
      });
  for (int v = 0; v < n; ++v) cout << result[v].sum.val() << (v + 1 == n ? '\n' : ' ');
}
