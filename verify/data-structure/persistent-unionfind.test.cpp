#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/rollback-union-find.hpp"

using namespace std;

// Rollback Union Findでバージョンの木をDFSし、永続Union Findの問い合わせを処理する。

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
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<Query> queries(q);
  // 入力のversion=-1を根0、操作iの結果を頂点i+1として管理する。
  vector<vector<int>> children(q + 1);
  for (int i = 0; i < q; ++i) {
    int version;
    cin >> queries[i].type >> version >> queries[i].u >> queries[i].v;
    children[version + 1].push_back(i + 1);
  }

  blueberry::RollbackUnionFind dsu(n);
  vector<int> answer(q, -1);
  vector<Event> stack{{0, 0, false}};
  while (!stack.empty()) {
    const Event event = stack.back();
    stack.pop_back();
    // 子の探索を終えたら変更を巻き戻し、兄弟のバージョンに持ち込まない。
    if (event.exit) {
      dsu.rollback(event.state);
      continue;
    }

    // この操作を適用する直前の履歴位置を保存する。
    const int state = dsu.state();
    if (event.node > 0) {
      const Query& query = queries[event.node - 1];
      if (query.type == 0) {
        dsu.merge(query.u, query.v);
      } else {
        answer[event.node - 1] = dsu.same(query.u, query.v);
      }
    }
    // 終了イベントを先に積むことで、子を全て処理してからrollbackする。
    stack.push_back({event.node, state, true});
    for (auto it = children[event.node].rbegin(); it != children[event.node].rend(); ++it) {
      stack.push_back({*it, 0, false});
    }
  }

  for (int i = 0; i < q; ++i) {
    if (queries[i].type == 1) cout << answer[i] << '\n';
  }
}
