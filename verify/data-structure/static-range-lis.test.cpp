#define PROBLEM "https://judge.yosupo.jp/problem/static_range_lis_query"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/static-range-lis.hpp"

int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  std::vector<int> a(n);
  for (int& x : a) in.read(x);
  blueberry::StaticRangeLIS<int> ds(a);
  while (q--) {
    int l, r;
    in.read(l, r);
    out.writeln(ds.lis(l, r));
  }
}
