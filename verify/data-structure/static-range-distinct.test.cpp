#define PROBLEM "https://judge.yosupo.jp/problem/static_range_count_distinct"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/static-range-distinct.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, q; cin >> n >> q;
  vector<int> a(n); for (int& x : a) cin >> x;
  blueberry::StaticRangeDistinct<int> distinct(a);
  while (q--) { int l, r; cin >> l >> r; cout << distinct.count(l, r) << '\n'; }
}
