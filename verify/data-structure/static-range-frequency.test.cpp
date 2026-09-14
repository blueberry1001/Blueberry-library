#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
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
  // Query exact frequency, including values absent from the input.
  while (q--) {
    int l, r, x;
    cin >> l >> r >> x;
    cout << wm.count(l, r, x) << '\n';
  }
}
