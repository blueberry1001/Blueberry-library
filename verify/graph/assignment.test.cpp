#define PROBLEM "https://judge.yosupo.jp/problem/assignment"
#include <iostream>
#include "blueberry/graph/assignment.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n; cin >> n;
  vector<vector<long long>> cost(n, vector<long long>(n));
  for (auto& row : cost) for (auto& x : row) cin >> x;
  auto [total, match] = blueberry::assignment(cost);
  cout << total << '\n';
  for (int x : match) cout << x << ' ';
  cout << '\n';
}
