// Research harness, not a public API. All prototypes are independently written.
// See docs/development/fastest-performance-plan.md for scope and limitations.
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "blueberry/data-structure/sparse-table.hpp"
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/data-structure/sqrt-tree.hpp"
#include "blueberry/data-structure/linear-rmq.hpp"

int minimum(int a, int b) { return std::min(a, b); }
int identity() { return std::numeric_limits<int>::max(); }
struct Min { int operator()(int a, int b) const { return minimum(a, b); } };
struct Sparse : blueberry::SparseTable<int, Min> {
  explicit Sparse(const std::vector<int>& a) : SparseTable(a, Min{}) {}
};
struct Disjoint : blueberry::DisjointSparseTable<int, Min> {
  explicit Disjoint(const std::vector<int>& a) : DisjointSparseTable(a, Min{}) {}
};

// Fixed 16-element blocks: short scans, boundary prefix/suffix, macro sparse table.
// Exploits min's idempotence; does not replace a general monoid Sqrt Tree.
struct Block16 {
  std::vector<int> a, pref, suff;
  Sparse macro;
  static std::vector<int> totals(const std::vector<int>& a) {
    std::vector<int> out((a.size() + 15) / 16, identity());
    for (int i = 0; i < static_cast<int>(a.size()); ++i)
      out[i / 16] = minimum(out[i / 16], a[i]);
    return out;
  }
  explicit Block16(const std::vector<int>& v) : a(v), pref(v), suff(v), macro(totals(v)) {
    for (int i = 0; i < static_cast<int>(a.size()); i += 16) {
      const int end = std::min(i + 16, static_cast<int>(a.size()));
      for (int j = i + 1; j < end; ++j) pref[j] = minimum(pref[j - 1], a[j]);
      for (int j = end - 2; j >= i; --j) suff[j] = minimum(a[j], suff[j + 1]);
    }
  }
  int prod(int l, int r) const {
    const int x = l / 16, y = (r - 1) / 16;
    if (x == y) {
      int ans = a[l];
      for (int i = l + 1; i < r; ++i) ans = minimum(ans, a[i]);
      return ans;
    }
    int ans = minimum(suff[l], pref[r - 1]);
    if (x + 1 < y) ans = minimum(ans, macro.prod(x + 1, y));
    return ans;
  }
};

// Alternative fixed 32-element monotonic masks. Fixed width is NOT an
// asymptotically linear-space claim: macro table is O((N/32) log(N/32)).
struct Mask32 {
  std::vector<int> a;
  std::vector<std::uint32_t> masks;
  Sparse macro;
  static std::vector<int> totals(const std::vector<int>& a) {
    std::vector<int> out((a.size() + 31) / 32, identity());
    for (int i = 0; i < static_cast<int>(a.size()); ++i)
      out[i / 32] = minimum(out[i / 32], a[i]);
    return out;
  }
  explicit Mask32(const std::vector<int>& v) : a(v), masks(v.size()), macro(totals(v)) {
    std::uint32_t mask = 0;
    for (int i = 0; i < static_cast<int>(a.size()); ++i) {
      if (i % 32 == 0) mask = 0;
      while (mask && a[i / 32 * 32 + std::bit_width(mask) - 1] > a[i])
        mask &= ~(std::uint32_t{1} << (std::bit_width(mask) - 1));
      mask |= std::uint32_t{1} << (i % 32);
      masks[i] = mask;
    }
  }
  int small(int l, int r) const {
    return a[l / 32 * 32 + std::countr_zero(masks[r - 1] & (~std::uint32_t{0} << (l % 32)))];
  }
  int prod(int l, int r) const {
    const int x = l / 32, y = (r - 1) / 32;
    if (x == y) return small(l, r);
    int ans = minimum(small(l, (x + 1) * 32), small(y * 32, r));
    if (x + 1 < y) ans = minimum(ans, macro.prod(x + 1, y));
    return ans;
  }
};

struct Streams {
  int read() { int x; if (!(std::cin >> x)) std::exit(2); return x; }
  void write(int x) { std::cout << x << '\n'; }
  void flush() { std::cout.flush(); }
};
// Portable scalar fread/fwrite; no mmap, SIMD, sentinel read or unsigned-only trick.
struct Buffered {
  std::array<char, 1 << 16> input{}, output{};
  std::size_t pos = 0, end = 0, used = 0;
  int next() {
    if (pos == end) {
      end = std::fread(input.data(), 1, input.size(), stdin); pos = 0;
      if (!end) return EOF;
    }
    return static_cast<unsigned char>(input[pos++]);
  }
  int read() {
    int c;
    do { c = next(); if (c == EOF) std::exit(2); } while (c <= ' ');
    bool neg = c == '-'; if (neg) c = next();
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + c - '0'; c = next(); }
    return neg ? -x : x;
  }
  void put(char c) { if (used == output.size()) flush(); output[used++] = c; }
  void write(int x) {
    char digits[12]; int n = 0;
    // Official staticrmq answers are nonnegative and fit int.
    do { digits[n++] = static_cast<char>('0' + x % 10); x /= 10; } while (x);
    while (n) put(digits[--n]);
    put('\n');
  }
  void flush() { if (used) std::fwrite(output.data(), 1, used, stdout); used = 0; std::fflush(stdout); }
};

using Clock = std::chrono::steady_clock;
double ms(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}
template<class Tree, class IO> void run(bool streaming) {
  IO io;
  const auto start = Clock::now();
  const int n = io.read(), q = io.read();
  std::vector<int> a(n);
  for (int& x : a) x = io.read();
  std::vector<std::array<int, 2>> queries;
  std::vector<int> answers;
  if (!streaming) {
    queries.resize(q); answers.resize(q);
    for (auto& query : queries) { query[0] = io.read(); query[1] = io.read(); }
  }
  const auto parsed = Clock::now();
  Tree tree(a);
  const auto built = Clock::now();
  std::uint64_t checksum = 0;
  for (int i = 0; i < q; ++i) {
    const int l = streaming ? io.read() : queries[i][0];
    const int r = streaming ? io.read() : queries[i][1];
    const int ans = tree.prod(l, r);
    checksum += static_cast<unsigned>(ans);
    if (streaming) io.write(ans); else answers[i] = ans;
  }
  const auto queried = Clock::now();
  if (!streaming) for (int x : answers) io.write(x);
  io.flush();
  const auto written = Clock::now();
  std::fprintf(stderr, "{\"n\":%d,\"q\":%d,\"parse_ms\":%.6f,\"build_ms\":%.6f,\"query_ms\":%.6f,\"write_ms\":%.6f,\"inside_ms\":%.6f,\"checksum\":%llu}\n",
    n, q, ms(start, parsed), ms(parsed, built), ms(built, queried), ms(queried, written), ms(start, written), static_cast<unsigned long long>(checksum));
}
template<class IO> void select(const std::string& algorithm, bool streaming) {
  if (algorithm == "sparse") run<Sparse, IO>(streaming);
  else if (algorithm == "disjoint") run<Disjoint, IO>(streaming);
  else if (algorithm == "sqrt") run<blueberry::SqrtTree<int, minimum, identity>, IO>(streaming);
  else if (algorithm == "linear") run<blueberry::LinearRMQ<int>, IO>(streaming);
  else if (algorithm == "block16") run<Block16, IO>(streaming);
  else if (algorithm == "mask32") run<Mask32, IO>(streaming);
  else std::exit(2);
}
int main(int argc, char** argv) {
  if (argc != 4) return 2;
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  const bool streaming = std::string(argv[3]) == "stream";
  if (std::string(argv[2]) == "iostream") select<Streams>(argv[1], streaming);
  else if (std::string(argv[2]) == "buffered") select<Buffered>(argv[1], streaming);
  else return 2;
}
