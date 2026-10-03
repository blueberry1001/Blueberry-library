#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum"

#include <vector>

#include "blueberry/graph/offline-dynamic-component-sum.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  std::vector<long long> values(n);
  for (auto& value : values) in.read(value);
  blueberry::OfflineDynamicComponentSum<long long> graph(values);
  while (q--) {
    int type, u;
    in.read(type, u);
    if (type == 0 || type == 1) {
      int v;
      in.read(v);
      if (type == 0) graph.add_edge(u, v);
      else graph.remove_edge(u, v);
    } else if (type == 2) {
      long long delta;
      in.read(delta);
      graph.add_value(u, delta);
    } else {
      graph.query(u);
    }
  }
  for (const auto& answer : graph.solve()) out.writeln(answer);
}
