#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

using Clock = std::chrono::steady_clock;
std::uint64_t input_hash = 0, checksum = 0;
void mix(std::uint64_t& hash, std::uint64_t x) { hash = (hash ^ x) * 0x9e3779b185ebca87ULL; }
template<class T> void mix_value(std::uint64_t& hash, T value) {
  using U = std::make_unsigned_t<T>;
  mix(hash, static_cast<std::uint64_t>(static_cast<U>(value)));
  if constexpr (std::numeric_limits<U>::digits > 64)
    mix(hash, static_cast<std::uint64_t>(static_cast<U>(value) >> 64));
}
template<class T> void measure(const std::string& distribution, int n, int repetitions) {
  using U = std::make_unsigned_t<T>;
  std::mt19937_64 rng(20260921);
  std::vector<T> a(n);
  for (int i = 0; i < n; ++i) {
    U value = static_cast<U>(rng());
    if constexpr (std::numeric_limits<U>::digits > 64) value |= static_cast<U>(rng()) << 64;
    a[i] = static_cast<T>(value);
    if (distribution == "duplicates") a[i] = static_cast<T>(static_cast<int>(rng() % 17) - 8);
  }
  if (distribution == "full") { a[0] = std::numeric_limits<T>::min(); a[1] = std::numeric_limits<T>::max(); }
  for (T value : a) mix_value(input_hash, value);
  double build_ms = 0;
  for (int repetition = 0; repetition < repetitions; ++repetition) {
    const auto start = Clock::now();
    blueberry::WaveletMatrix<T> matrix(a);
    const auto end = Clock::now();
    build_ms += std::chrono::duration<double, std::milli>(end - start).count();
    // Full-width checksum, outside the measured region; destructor is excluded too.
    mix_value(checksum, matrix.get(static_cast<int>((17LL * repetition) % n)));
  }
  std::cout << "{\"build_ms\":" << build_ms << ",\"build_us_each\":" << build_ms * 1000 / repetitions
            << ",\"input_hash\":\"" << input_hash << "\",\"checksum\":\"" << checksum << "\"}\n";
}
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const int width = std::atoi(argv[1]), n = std::atoi(argv[3]), repetitions = std::atoi(argv[4]);
  if (width == 64) measure<long long>(argv[2], n, repetitions);
  else if (width == 128) measure<__int128_t>(argv[2], n, repetitions);
  else return 2;
}
