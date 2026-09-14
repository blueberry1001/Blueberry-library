#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/segment-tree.hpp"

using namespace std;

// SegmentTree自身の検証。区間内の一次関数を左から順に合成する。

namespace {

constexpr long long kMod = 998244353;

struct Affine {
  long long a;
  long long b;
};

// op(left,right)はright(left(x))。単位元は恒等関数{1,0}。
struct Compose {
  Affine operator()(const Affine& left, const Affine& right) const {
    return {right.a * left.a % kMod, (right.a * left.b + right.b) % kMod};
  }
};

}  // namespace

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<Affine> functions(n);
  for (auto& [a, b] : functions) cin >> a >> b;
  blueberry::SegmentTree<Affine, Compose> segment_tree(functions, Compose{}, {1, 0});
  while (q--) {
    int type;
    cin >> type;
    if (type == 0) {
      int position;
      Affine function;
      cin >> position >> function.a >> function.b;
      segment_tree.set(position, function);
    } else {
      int left, right;
      long long x;
      cin >> left >> right >> x;
      const auto function = segment_tree.prod(left, right);
      cout << (function.a * x + function.b) % kMod << '\n';
    }
  }
}
