#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"

#include <cstdint>
#include <utility>
#include <vector>

#include "blueberry/extra-fast/static-rmq.hpp"
#include "blueberry/utility/fast-io.hpp"

using namespace std;

// 32bit min専用のブロックRMQを、オンラインの任意順クエリで検証する。
int main() {
  blueberry::FastInput input;
  blueberry::FastOutput output;
  int n, q;
  if (!input.read(n, q)) return 0;
  vector<int32_t> values(n);
  for (auto& value : values) input.read(value);
  blueberry::extra_fast::StaticRangeMin<int32_t> rmq(std::move(values));
  while (q--) {
    int l, r;
    input.read(l, r);
    output.writeln(rmq.prod(l, r));
  }
}
