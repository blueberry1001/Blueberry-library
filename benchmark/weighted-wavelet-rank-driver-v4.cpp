#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include "weighted-wavelet-matrix.hpp"

using Clock = std::chrono::steady_clock;
struct Query { int l, r, low, high, point; long long delta; };

template<class T> T make_value(std::uint64_t x) {
  if constexpr (std::is_same_v<T, std::string>) return std::to_string(x);
  else return static_cast<T>(std::bit_cast<std::int64_t>(x));
}

template<class T> int run(int n, int q, const std::string& shape) {
  std::mt19937_64 rng(1729);
  std::vector<T> values;
  values.reserve(n);
  std::vector<long long> weights(n);
  for (int i = 0; i < n; ++i) {
    std::uint64_t x = shape == "biased-head-extreme" && i < 256 ? ((i & 1) ? (1ULL << 63) : ((1ULL << 63) - 1)) :
                      shape == "biased-tail-extreme" && i >= 256 ? ((i & 1) ? (1ULL << 63) : ((1ULL << 63) - 1)) :
                      shape == "near-min" ? (1ULL << 63) + rng() % 201 :
                      shape == "near-max" ? ((1ULL << 63) - 1) - rng() % 201 :
                      shape == "equal" ? 7 :
                      shape == "signed-small" ? static_cast<std::uint64_t>(static_cast<std::int64_t>(rng() % 201) - 100) :
                      shape == "extrema" && i % 3 == 0 ? (1ULL << 63) :
                      shape == "extrema" && i % 3 == 1 ? ((1ULL << 63) - 1) :
                      shape == "duplicate" ? rng() % 32 :
                      shape == "ascending" ? i :
                      shape == "descending" ? n - i : rng();
    values.push_back(make_value<T>(x));
    weights[i] = static_cast<long long>(rng() % 2001) - 1000;
  }
  std::vector<Query> queries;
  queries.reserve(q);
  for (int i = 0; i < q; ++i) {
    int l = rng() % n, r = rng() % n, low = rng() % n, high = rng() % n;
    if (r < l) std::swap(l, r);
    if (values[high] < values[low]) std::swap(low, high);
    queries.push_back({l, r, low, high, static_cast<int>(rng() % n), static_cast<long long>(rng() % 21) - 10});
  }
  auto start = Clock::now();
  blueberry::WeightedWaveletMatrix<T> tree(values, weights);
  auto built = Clock::now();
  std::uint64_t checksum = 0;
  for (int i = 0; i < q; ++i) {
    auto z = queries[i];
    if (i % 4 == 0) tree.add(z.point, z.delta);
    else checksum = checksum * 1000003 + static_cast<std::uint64_t>(tree.sum(z.l, z.r, values[z.low], values[z.high]));
  }
  auto done = Clock::now();
  std::cout << checksum << ' ' << std::chrono::duration<double, std::milli>(built - start).count() << ' '
            << std::chrono::duration<double, std::milli>(done - built).count() << '\n';
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 5) return 1;
  int n = std::atoi(argv[1]), q = std::atoi(argv[2]);
  if (n <= 0 || q < 0) return 1;
  return std::string(argv[4]) == "string" ? run<std::string>(n, q, argv[3]) : run<long long>(n, q, argv[3]);
}
