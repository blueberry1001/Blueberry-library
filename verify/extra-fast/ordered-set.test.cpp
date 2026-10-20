#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include <cstdint>

#include "blueberry/extra-fast/ordered-map.hpp"
#include "blueberry/utility/fast-io.hpp"

// Online insertion/deletion, rank, kth and inclusive predecessor/successor.
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  blueberry::extra_fast::OrderedSet<> set;
  for (int i = 0; i < n; ++i) {
    std::uint32_t x;
    in.read(x);
    set.insert(x);
  }
  while (q--) {
    int t;
    std::uint32_t x;
    in.read(t, x);
    if (t == 0)
      set.insert(x);
    else if (t == 1)
      set.erase(x);
    else if (t == 3)
      out.writeln(set.rank(x + 1)); // LC guarantees x <= 1e9, so x+1 fits.
    else {
      auto answer = t == 2   ? set.kth(static_cast<int>(x) - 1)
                    : t == 4 ? set.floor(x)
                             : set.lower_bound(x);
      out.writeln(answer ? static_cast<std::int64_t>(*answer) : -1);
    }
  }
}
