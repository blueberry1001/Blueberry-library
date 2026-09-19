#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

using namespace std;

int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  vector<int> a(n);
  for (int& x : a) in.read(x);
  blueberry::WaveletMatrix<int> wm(a);
  // k is zero-indexed; duplicate values each occupy one position.
  while (q--) {
    int l, r, k;
    in.read(l, r, k);
    out.writeln(wm.kth_smallest(l, r, k));
  }
}
