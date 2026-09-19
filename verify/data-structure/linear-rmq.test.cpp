#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/data-structure/linear-rmq.hpp"
int main() {
  blueberry::FastInput in; blueberry::FastOutput out;
  int n,q; in.read(n,q);
  std::vector<int> a(n); for(auto& x:a) in.read(x);
  blueberry::LinearRMQ<int> rmq(a);
  while(q--){int l,r;in.read(l,r);out.writeln(rmq.prod(l,r));}
}
