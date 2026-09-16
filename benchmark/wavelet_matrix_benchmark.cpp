// Independent integer-prefix candidate versus packed rank. See ADDITIONS.md.
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

class PrefixMatrix {
  std::vector<int> values_, middle_;
  std::vector<std::vector<int>> prefix_;
 public:
  explicit PrefixMatrix(const std::vector<int>& a) : values_(a) {
    std::sort(values_.begin(), values_.end());
    values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
    int height = std::bit_width(static_cast<unsigned>(values_.size() - 1));
    const int n = static_cast<int>(a.size());
    std::vector<int> codes(n), buffer(n);
    for (int i = 0; i < n; ++i) codes[i] = static_cast<int>(std::lower_bound(values_.begin(), values_.end(), a[i]) - values_.begin());
    for (int bit = height - 1; bit >= 0; --bit) {
      auto& pref = prefix_.emplace_back(n + 1, 0);
      for (int i = 0; i < n; ++i) pref[i + 1] = pref[i] + ((codes[i] >> bit) & 1);
      middle_.push_back(n - pref[n]);
      int left = 0, right = middle_.back();
      for (int code : codes) {
        if ((code >> bit) & 1) buffer[right++] = code;
        else buffer[left++] = code;
      }
      codes.swap(buffer);
    }
  }
  int kth_smallest(int l, int r, int k) const {
    int code = 0;
    for (std::size_t level = 0; level < prefix_.size(); ++level) {
      const auto& p = prefix_[level];
      const int left = l - p[l], right = r - p[r];
      code *= 2;
      if (k < right - left) { l = left; r = right; }
      else { k -= right - left; ++code; l = middle_[level] + p[l]; r = middle_[level] + p[r]; }
    }
    return values_[code];
  }
};

struct Query { int l, r, k; };
struct Measurement { double build_ms, query_ms; std::uint64_t checksum; };

template <class Matrix>
Measurement measure(const std::vector<int>& a, const std::vector<Query>& queries) {
  const auto start = std::chrono::steady_clock::now();
  Matrix wm(a);
  const auto built = std::chrono::steady_clock::now();
  std::uint64_t checksum = 0;
  for (auto [l, r, k] : queries) checksum += wm.kth_smallest(l, r, k);
  const auto finish = std::chrono::steady_clock::now();
  return {std::chrono::duration<double, std::milli>(built - start).count(),
          std::chrono::duration<double, std::milli>(finish - built).count(), checksum};
}

int main() {
  constexpr int n = 200000, q = 200000, repeats = 5;
  std::cout << "seed=20260914 N=" << n << " Q=" << q << " repeats=" << repeats << "\n";
  std::cout << "distribution,run,candidate,build_ms,query_ms,checksum\n";
  for (int sigma : {16, n}) {
    std::mt19937 rng(20260914);
    std::vector<int> a(n);
    for (auto& x : a) x = static_cast<int>(rng() % sigma);
    std::vector<Query> queries;
    for (int i = 0; i < q; ++i) {
      int l = static_cast<int>(rng() % n), r = static_cast<int>(rng() % n);
      if (l > r) std::swap(l, r);
      ++r;
      queries.push_back({l, r, static_cast<int>(rng() % (r - l))});
    }
    for (int run = 0; run < repeats; ++run) {
      Measurement packed, prefix;
      if (run % 2) {
        prefix = measure<PrefixMatrix>(a, queries);
        packed = measure<blueberry::WaveletMatrix<int>>(a, queries);
      } else {
        packed = measure<blueberry::WaveletMatrix<int>>(a, queries);
        prefix = measure<PrefixMatrix>(a, queries);
      }
      if (packed.checksum != prefix.checksum) return EXIT_FAILURE;
      std::cout << sigma << ',' << run << ",packed," << packed.build_ms << ',' << packed.query_ms << ',' << packed.checksum << '\n';
      std::cout << sigma << ',' << run << ",prefix," << prefix.build_ms << ',' << prefix.query_ms << ',' << prefix.checksum << '\n';
    }
  }
}
