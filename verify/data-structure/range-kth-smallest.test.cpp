#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  vector<int> a(n);
  for (int& x : a) cin >> x;
  blueberry::WaveletMatrix<int> wm(a);
  // k is zero-indexed; duplicate values each occupy one position.
  while (q--) {
    int l, r, k;
    cin >> l >> r >> k;
    cout << wm.kth_smallest(l, r, k) << '\n';
  }
}
