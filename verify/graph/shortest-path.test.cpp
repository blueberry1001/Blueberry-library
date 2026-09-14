#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"

#include <iostream>

#include "blueberry/graph/dijkstra.hpp"

using namespace std;

// 非負重みの有向グラフで最短距離を計算し、復元した経路の各辺を出力する。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, m, source, target;
  cin >> n >> m >> source >> target;
  blueberry::WeightedGraph<long long> graph(n);
  for (int i = 0; i < m; ++i) {
    int from, to;
    long long cost;
    cin >> from >> to >> cost;
    graph[from].push_back({to, cost});
  }

  constexpr long long kInfinity = 1LL << 62;
  const auto result = blueberry::dijkstra(graph, source, kInfinity);
  const auto path = result.path_to(target);
  // 到達不能ならpath_toは空。到達できる場合は始点・終点を含む頂点列を返す。
  if (path.empty()) {
    cout << -1 << '\n';
    return 0;
  }

  cout << result.distance[target] << ' ' << path.size() - 1 << '\n';
  for (int i = 1; i < static_cast<int>(path.size()); ++i) {
    cout << path[i - 1] << ' ' << path[i] << '\n';
  }
}
