#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>
#include <atcoder/modint>
#include "blueberry/string/count-subsequences.hpp"

struct Count {
  int value;
  Count() = delete;
  Count(int x) : value(x % 1009) {}
  friend Count operator+(Count x, Count y) { return Count(x.value + y.value); }
  friend Count operator-(Count x, Count y) { return Count((x.value - y.value + 1009) % 1009); }
};
struct LargeCount {
  std::array<unsigned char, 1024> padding{};
  int value;
  LargeCount() = delete;
  LargeCount(int x) : value(x % 1009) {}
  friend LargeCount operator+(LargeCount x, LargeCount y) { return LargeCount(x.value + y.value); }
  friend LargeCount operator-(LargeCount x, LargeCount y) { return LargeCount((x.value - y.value + 1009) % 1009); }
};
struct Proxy {
  unsigned char value;
  operator unsigned char() const { return value; }
  friend bool operator<(Proxy x, Proxy y) { return x.value / 4 < y.value / 4; }
};
struct ProxySequence {
  std::vector<unsigned char> data;
  std::size_t size() const { return data.size(); }
  Proxy operator[](std::size_t i) const { return {data[i]}; }
};
template <class Sequence>
int direct(const Sequence& sequence, int modulus) {
  std::map<int, int> previous;
  int total = 1;
  for (auto x : sequence) {
    int& old = previous[static_cast<int>(x)];
    const int next = (2 * total - old + modulus) % modulus;
    old = total; total = next;
  }
  return (total + modulus - 1) % modulus;
}
template <class T>
int direct_wide(const std::vector<T>& sequence) {
  std::map<T, int> previous;
  int total = 1;
  for (auto x : sequence) {
    int& old = previous[x];
    const int next = (2 * total - old + 1009) % 1009;
    old = total; total = next;
  }
  return (total + 1008) % 1009;
}
template <class T>
bool check_integer(std::mt19937& rng) {
  for (int n : {255, 256, 257, 1024}) for (int shape = 0; shape < 5; ++shape) {
    std::vector<T> a(n);
    for (int i = 0; i < n; ++i) {
      if (shape == 0) a[i] = std::numeric_limits<T>::min() + static_cast<T>(rng() % 8);
      if (shape == 1) a[i] = std::numeric_limits<T>::max() - static_cast<T>(rng() % 8);
      if (shape == 2) a[i] = i & 1 ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
      if (shape == 3) a[i] = static_cast<T>(rng() % 32);
      if (shape == 4) a[i] = i == n / 2 + 1 ? std::numeric_limits<T>::max() : T(0);
    }
    if (blueberry::count_subsequences<Count>(a).value != direct_wide(a)) return false;
  }
  return true;
}
template <class CountType>
bool check_width(int n, int low, int width) {
  std::vector<int> a(n);
  for (int i = 0; i < n; ++i) a[i] = i & 1 ? low + width : low;
  return blueberry::count_subsequences<CountType>(a).value == direct_wide(a);
}

int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1;
  std::mt19937 rng(seed);
  assert(blueberry::count_subsequences<long long>(std::string{}) == 0);
  assert(blueberry::count_subsequences<long long>(std::string("aba")) == 6);
  assert(blueberry::count_subsequences<long long>(std::vector<int>(100, -1)) == 100);
  for (int n : {0, 1, 63, 64, 65, 255, 256, 257, 1000}) {
    std::vector<signed char> a(n);
    std::vector<unsigned char> b(n);
    std::vector<bool> bits(n);
    ProxySequence proxies;
    std::vector<int> classes(n);
    for (int i = 0; i < n; ++i) {
      a[i] = static_cast<signed char>(i % 256 - 128);
      b[i] = static_cast<unsigned char>(rng() % 256);
      bits[i] = rng() % 2;
      proxies.data.push_back(b[i]); classes[i] = b[i] / 4;
    }
    using Mint = atcoder::static_modint<1009>;
    if (blueberry::count_subsequences<Mint>(a).val() != direct(a, 1009) ||
        blueberry::count_subsequences<Mint>(b).val() != direct(b, 1009) ||
        blueberry::count_subsequences<Mint>(bits).val() != direct(bits, 1009) ||
        blueberry::count_subsequences<Mint>(proxies).val() != direct(classes, 1009) ||
        blueberry::count_subsequences<Count>(a).value != direct(a, 1009) ||
        blueberry::count_subsequences<LargeCount>(a).value != direct(a, 1009)) {
      std::cerr << "seed=" << seed << " byte/proxy input length=" << n << '\n'; return 1;
    }
  }
  if (!check_integer<std::int16_t>(rng) || !check_integer<std::uint16_t>(rng) ||
      !check_integer<std::int32_t>(rng) || !check_integer<std::uint32_t>(rng) ||
      !check_integer<std::int64_t>(rng) || !check_integer<std::uint64_t>(rng) ||
      !check_integer<char16_t>(rng) || !check_integer<char32_t>(rng) || !check_integer<wchar_t>(rng)) {
    std::cerr << "seed=" << seed << " integer boundary input\n"; return 1;
  }
  const int large_limit = static_cast<int>(262144 / sizeof(LargeCount));
  if (!check_width<Count>(256, -64, 128) || !check_width<Count>(256, -64, 129) ||
      !check_width<Count>(131072, -32768, 65535) || !check_width<Count>(131072, -32768, 65536) ||
      !check_width<LargeCount>(1024, 0, large_limit) || !check_width<LargeCount>(1024, 0, large_limit + 1)) {
    std::cerr << "seed=" << seed << " table width boundary\n"; return 1;
  }
#if defined(__SIZEOF_INT128__)
  std::vector<__int128> large(512);
  for (int i = 0; i < 512; ++i) large[i] = (static_cast<__int128>(i % 11) - 5) * (static_cast<__int128>(1) << 80);
  if (blueberry::count_subsequences<Count>(large).value != direct_wide(large)) return 1;
#endif
  for (int trial = 0; trial < 400; ++trial) {
    const int n = rng() % 13;
    std::vector<int> a(n);
    for (auto& x : a) x = static_cast<int>(rng() % 7) - 3;
    std::set<std::vector<int>> distinct;
    for (int mask = 1; mask < (1 << n); ++mask) {
      std::vector<int> s;
      for (int i = 0; i < n; ++i) if (mask >> i & 1) s.push_back(a[i]);
      distinct.insert(s);
    }
    const auto actual = blueberry::count_subsequences<long long>(a);
    using Mint = atcoder::static_modint<7>;
    const auto modular = blueberry::count_subsequences<Mint>(a).val();
    if (actual != static_cast<long long>(distinct.size()) || modular != actual % 7) {
      std::cerr << "seed=" << seed << " trial=" << trial << " input=";
      for (auto x : a) std::cerr << x << ',';
      std::cerr << " expected=" << distinct.size() << " actual=" << actual
                << " mod7=" << modular << '\n';
      return 1;
    }
  }
}
