#define PROBLEM "https://judge.yosupo.jp/problem/area_of_union_of_rectangles"
#include <array>
#include <iostream>
#include <vector>
#include "blueberry/data-structure/rectangle-union.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n; cin >> n;
  vector<array<long long, 4>> rectangles(n);
  for (auto& r : rectangles) for (auto& x : r) cin >> x;
  cout << blueberry::rectangle_union_area(rectangles) << '\n';
}
