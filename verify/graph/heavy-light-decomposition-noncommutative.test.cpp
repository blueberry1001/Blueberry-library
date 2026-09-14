#define PROBLEM "https://judge.yosupo.jp/problem/vertex_set_path_composite"

#include <atcoder/segtree>

#include <iostream>
#include <utility>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

using namespace std;

// HLDの非可換パスクエリを検証する。区間の関数合成にはACLのsegtreeを使う。

constexpr long long kMod = 998244353;
using Affine = pair<long long, long long>;

// firstを適用してからsecondを適用する。関数合成は順番を交換できない。
Affine compose(Affine first, Affine second) {
  return {second.first * first.first % kMod,
          (second.first * first.second + second.second) % kMod};
}

// 合成の単位元は恒等関数 f(x) = x。
Affine e() { return {1, 0}; }

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<Affine> value(n);
  for (auto& [a, b] : value) cin >> a >> b;
  vector<vector<int>> tree(n);
  for (int i = 0; i + 1 < n; ++i) {
    int u, v;
    cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
  }

  const blueberry::HLD hld(tree);
  // HLD順とその逆順を用意し、パスを辿る向きに応じて区間積を選ぶ。
  vector<Affine> forward_values(n), reverse_values(n);
  for (int v = 0; v < n; ++v) {
    forward_values[hld.in(v)] = value[v];
    reverse_values[n - 1 - hld.in(v)] = value[v];
  }
  atcoder::segtree<Affine, compose, e> forward(forward_values), backward(reverse_values);

  while (q--) {
    int type;
    cin >> type;
    if (type == 0) {
      int v;
      Affine f;
      cin >> v >> f.first >> f.second;
      // 同じ頂点の関数を、正順・逆順の両方に反映する。
      forward.set(hld.in(v), f);
      backward.set(n - 1 - hld.in(v), f);
    } else {
      int u, v;
      long long x;
      cin >> u >> v >> x;
      Affine path{1, 0};
      // callbackはu→vの順。逆向きの[l,r)は逆順配列の[n-r,n-l)に対応する。
      hld.vertex_query(u, v, [&](int left, int right, bool reversed) {
        const Affine part = reversed ? backward.prod(n - right, n - left)
                                     : forward.prod(left, right);
        path = compose(path, part);
      });
      cout << (path.first * x + path.second) % kMod << '\n';
    }
  }
}
