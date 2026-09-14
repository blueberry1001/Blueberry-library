#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"

#include <algorithm>
#include <iostream>
#include <vector>

#include "blueberry/data-structure/sparse-table.hpp"

using namespace std;

// 更新のない配列を前計算し、半開区間[l,r)の最小値をO(1)で求める。

struct Minimum {
  int operator()(int left, int right) const { return min(left, right); }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<int> values(n);
  for (int& value : values) cin >> value;
  blueberry::SparseTable<int, Minimum> sparse_table(values, Minimum{});
  while (q--) {
    int left, right;
    cin >> left >> right;
    cout << sparse_table.prod(left, right) << '\n';
  }
}
