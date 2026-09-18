#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include <iostream>
#include "blueberry/data-structure/persistent-queue.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int q; cin >> q;
  blueberry::PersistentQueue<int> queue;
  while (q--) {
    int type, t; cin >> type >> t; ++t;
    if (type == 0) { int x; cin >> x; queue.push(t, x); }
    else { cout << queue.front(t) << '\n'; queue.pop(t); }
  }
}
