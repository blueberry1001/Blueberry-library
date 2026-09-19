#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
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
  // Query exact frequency, including values absent from the input.
  while (q--) {
    int l, r, x;
    in.read(l, r, x);
    out.writeln(wm.count(l, r, x));
  }
}
