#define PROBLEM "https://judge.yosupo.jp/problem/persistent_unionfind"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/persistent-union-find.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  blueberry::PersistentUnionFind uf(n);
  vector<int> version(q + 1);
  for (int i = 0; i < q; ++i) {
    int t, k, a, b;
    cin >> t >> k >> a >> b;
    if (t == 0) version[i + 1] = uf.merge(version[k + 1], a, b);
    else {
      version[i + 1] = version[k + 1];
      cout << uf.same(version[k + 1], a, b) << '\n';
    }
  }
}
