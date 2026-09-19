#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/persistent-array.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int q;
  cin >> q;
  blueberry::PersistentArray<int> a(q);
  struct State { int version = 0, l = 0, r = 0; };
  vector<State> states(q + 1);
  for (int i = 0; i < q; ++i) {
    int type, t;
    cin >> type >> t;
    states[i + 1] = states[t + 1];
    auto& current = states[i + 1];
    if (type == 0) {
      int x;
      cin >> x;
      current.version = a.set(current.version, current.r++, x);
    } else cout << a.get(current.version, current.l++) << '\n';
  }
}
