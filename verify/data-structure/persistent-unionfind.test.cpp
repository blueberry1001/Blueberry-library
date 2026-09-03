#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/rollback-union-find.hpp"

struct Query {
  int type;
  int u;
  int v;
};

struct Event {
  int node;
  int state;
  bool exit;
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<Query> queries(q);
  std::vector<std::vector<int>> children(q + 1);
  for (int i = 0; i < q; ++i) {
    int version;
    std::cin >> queries[i].type >> version >> queries[i].u >> queries[i].v;
    children[version + 1].push_back(i + 1);
  }

  blueberry::RollbackUnionFind dsu(n);
  std::vector<int> answer(q, -1);
  std::vector<Event> stack{{0, 0, false}};
  while (!stack.empty()) {
    const Event event = stack.back();
    stack.pop_back();
    if (event.exit) {
      dsu.rollback(event.state);
      continue;
    }

    const int state = dsu.state();
    if (event.node > 0) {
      const Query& query = queries[event.node - 1];
      if (query.type == 0) {
        dsu.merge(query.u, query.v);
      } else {
        answer[event.node - 1] = dsu.same(query.u, query.v);
      }
    }
    stack.push_back({event.node, state, true});
    for (auto it = children[event.node].rbegin(); it != children[event.node].rend(); ++it) {
      stack.push_back({*it, 0, false});
    }
  }

  for (int i = 0; i < q; ++i) {
    if (queries[i].type == 1) std::cout << answer[i] << '\n';
  }
}
