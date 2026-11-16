#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"
#include <atcoder/modint>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

struct Group {
  long long value = 0;
  Group() = default;
  explicit Group(long long x) : value(x) {}
  Group& operator+=(const Group& rhs) { value += rhs.value; return *this; }
  friend Group operator+(Group a, const Group& b) { return a += b; }
  friend Group operator-(Group a, const Group& b) { a.value -= b.value; return a; }
};

template<class T>
void check_large(T a) {
  blueberry::DynamicFenwickTree2D<T, int> tree(4, 4);
  for (int y : {1, 0, 2}) {
    T value = y == 0 ? a : -a;
    tree.add(1, y, value); tree.add(2, y, -value);
  }
  assert(tree.sum(1, 1, 3, 3) == T{});
  assert(tree.sum(0, 0, 4, 4) == T{});
}

template<class T>
void check_small(std::uint64_t seed) {
  constexpr int n=12,m=9;
  blueberry::DynamicFenwickTree2D<T,int> tree(n,m);
  long long a[n][m] = {};
  std::mt19937_64 rng(seed);
  for (int step = 0; step < 400; ++step) {
    int x = rng()%n, y = rng()%m, delta = int(rng()%11)-5;
    tree.add(x,y,static_cast<T>(delta)); a[x][y] += delta;
  }
  for (int l=0;l<=n;++l) for (int r=l;r<=n;++r)
    for (int d=0;d<=m;++d) for (int u=d;u<=m;++u) {
      long long result=0;
      for (int x=l;x<r;++x) for (int y=d;y<u;++y) result+=a[x][y];
      assert(tree.sum(l,d,r,u) == static_cast<T>(result));
    }
}
int main(int argc, char** argv) {
  auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  check_small<int>(seed); check_small<long long>(seed); check_small<unsigned>(seed);
  check_small<std::uint8_t>(seed); check_small<std::uint16_t>(seed);
  check_small<atcoder::modint998244353>(seed);
  check_large<std::int8_t>(100); check_large<std::int16_t>(20000);
  check_large<int>(1500000000); check_large<long long>(6000000000000000000LL);
  check_large<double>(1e308);
  blueberry::DynamicFenwickTree2D<Group,int> tree(4,4);
  tree.add(1,2,Group(-7)); tree.add(2,1,Group(10));
  assert(tree.sum(1,1,3,3).value == 3);
  blueberry::DynamicFenwickTree2D<long long> minimum(1,1);
  minimum.add(0,0,std::numeric_limits<long long>::min());
  assert(minimum.get(0,0)==std::numeric_limits<long long>::min());
  constexpr long long hi=std::numeric_limits<long long>::max();
  blueberry::DynamicFenwickTree2D<long long> big(hi,hi);
  big.add(hi-1,hi-1,5); big.add(hi-10,hi-8,-3); big.add(0,0,10);
  assert(big.sum(hi-10,hi-8,hi,hi)==2);
  assert(big.get(hi-1,hi-1)==5);
  assert(big.sum(0,0,hi,hi)==12);
}
