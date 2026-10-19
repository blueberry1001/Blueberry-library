// Isolated construction candidate, independently implemented from the prefix
// scan idea seen in LC 257591. Not a public library or a copied submission.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#define PREFIX_HAS_AVX2 1
#else
#define PREFIX_HAS_AVX2 0
#endif

using U64 = std::uint64_t;
using Clock = std::chrono::steady_clock;
struct Layout {
  std::vector<U64> data;
  std::array<std::size_t, 8> offsets{};
  int levels = 0;
  explicit Layout(const std::vector<U64>& values) {
    std::size_t n = values.size(), total = 0;
    do {
      offsets[levels++] = total;
      n = std::max(std::size_t(1), (n + 15) / 16);
      total += 16 * n;
    } while (n > 1);
    data.assign(total, 0);
    std::copy(values.begin(), values.end(), data.begin());
  }
};

void scalar_prefix(Layout& tree) {
  for (int level = 0; level < tree.levels; ++level) {
    const auto begin = tree.offsets[level];
    const auto end = level + 1 < tree.levels ? tree.offsets[level + 1] : tree.data.size();
    for (auto base = begin; base < end; base += 16) {
      for (int j = 1; j < 16; ++j) tree.data[base + j] += tree.data[base + j - 1];
      if (level + 1 < tree.levels)
        tree.data[tree.offsets[level + 1] + (base - begin) / 16] = tree.data[base + 15];
    }
  }
}

#if PREFIX_HAS_AVX2
__attribute__((target("avx2"))) void simd_prefix(Layout& tree) {
  for (int level = 0; level < tree.levels; ++level) {
    const auto begin = tree.offsets[level];
    const auto end = level + 1 < tree.levels ? tree.offsets[level + 1] : tree.data.size();
    for (auto base = begin; base < end; base += 16) {
      __m256i carry = _mm256_setzero_si256();
      for (int j = 0; j < 16; j += 4) {
        auto* pointer = reinterpret_cast<__m256i*>(tree.data.data() + base + j);
        __m256i values = _mm256_loadu_si256(pointer);
        // Prefix within each 128-bit half, then propagate the lower half's sum.
        values = _mm256_add_epi64(values, _mm256_slli_si256(values, 8));
        const auto low_total = _mm256_permute4x64_epi64(values, _MM_SHUFFLE(1, 1, 1, 1));
        values = _mm256_add_epi64(values,
            _mm256_blend_epi32(_mm256_setzero_si256(), low_total, 0xf0));
        values = _mm256_add_epi64(values, carry);
        _mm256_storeu_si256(pointer, values);
        carry = _mm256_permute4x64_epi64(values, _MM_SHUFFLE(3, 3, 3, 3));
      }
      if (level + 1 < tree.levels)
        tree.data[tree.offsets[level + 1] + (base - begin) / 16] = tree.data[base + 15];
    }
  }
}
#endif

bool simd_available() {
#if PREFIX_HAS_AVX2
  return __builtin_cpu_supports("avx2");
#else
  return false;
#endif
}

void build(Layout& tree, bool simd) {
#if PREFIX_HAS_AVX2
  if (simd) { simd_prefix(tree); return; }
#else
  (void) simd;
#endif
  scalar_prefix(tree);
}

int main(int argc, char** argv) {
  if (argc != 3) return 2;
  const int n = std::stoi(argv[1]);
  const bool simd = std::string(argv[2]) == "simd";
  if (n < 0 || (simd && !simd_available())) return 3;
  std::mt19937_64 rng(20261019);
  // Verify every internal prefix, including overflow and padded zero cells.
  for (int size : {0, 1, 3, 4, 7, 15, 16, 17, 255, 256, 257, 4095, 4096, 4097}) {
    std::vector<U64> input(size);
    for (auto& x : input) x = rng();
    Layout expected(input), actual(input);
    scalar_prefix(expected); build(actual, simd);
    if (expected.data != actual.data) throw std::runtime_error("prefix mismatch");
  }
  std::vector<U64> input(n);
  for (auto& x : input) x = rng();
  const auto start = Clock::now();
  Layout tree(input);
  const auto allocated = Clock::now();
  build(tree, simd);
  const auto built = Clock::now();
  U64 expected = 0;
  for (auto x : input) expected += x;
  if (tree.data.back() != expected) throw std::runtime_error("total mismatch");
  auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
  std::cout << "{\"allocation_copy_ms\":" << ms(start, allocated)
            << ",\"prefix_ms\":" << ms(allocated, built)
            << ",\"build_ms\":" << ms(start, built)
            << ",\"checksum\":\"" << tree.data.back() << "\"}\n";
}
