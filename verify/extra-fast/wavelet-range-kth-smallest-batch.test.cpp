#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
#include "blueberry/utility/fast-io.hpp"
#include <array>
#include <vector>
#include "blueberry/extra-fast/wavelet-matrix.hpp"

using namespace std;

int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  vector<int> a(n);
  for (int& x : a) in.read(x);
  blueberry::extra_fast::WaveletMatrix<int> wm(a);
  // Independent queries are grouped by level; output order remains unchanged.
  vector<array<int, 3>> queries(q);
  for (auto& query : queries) in.read(query[0], query[1], query[2]);
  for (int answer : wm.kth_smallest_batch(queries)) out.writeln(answer);
  return 0;
}
