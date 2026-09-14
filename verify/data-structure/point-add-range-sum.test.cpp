#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/fenwick-tree.hpp"

using namespace std;

// FenwickTree自身の検証。一点加算と半開区間[l,r)の和を処理する。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<long long> values(n);
  for (auto& value : values) cin >> value;
  blueberry::FenwickTree<long long> fenwick(values);
  while (q--) {
    int type, x, y;
    cin >> type >> x >> y;
    if (type == 0) {
      fenwick.add(x, y);
    } else {
      cout << fenwick.sum(x, y) << '\n';
    }
  }
}
