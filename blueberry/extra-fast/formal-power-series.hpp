#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#if (defined(__x86_64__) || defined(__i386__)) && \
    (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#define BLUEBERRY_EXTRA_FAST_FPS_AVX2 1
#else
#define BLUEBERRY_EXTRA_FAST_FPS_AVX2 0
#endif

namespace blueberry::extra_fast {

// 998244353-only FPS. Independent radix-2 DIF/DIT Montgomery NTT; the SIMD
// Montgomery/transform-reuse ideas were studied in LC 409662 and 409370.
// Inputs and outputs are canonical uint32_t coefficients, in ascending order.
class FPS998244353 {
  using U = std::uint32_t;
  using V = std::vector<U>;
  static constexpr U p = 998244353, p2 = p * 2;
  static constexpr U neg_inv = 998244351, r = 301989884, r2 = 932051910;
  bool simd_;

  static U reduce(std::uint64_t x) {
    return static_cast<U>((x + std::uint64_t(U(x) * neg_inv) * p) >> 32);
  }
  static U canonical(U x) { return x >= p ? x - p : x; }
  static U add(U a, U b) { return a + b >= p2 ? a + b - p2 : a + b; }
  static U sub(U a, U b) { return a >= b ? a - b : a + p2 - b; }
  static U mul(U a, U b) { return reduce(std::uint64_t(a) * b); }
  static U power(U a, U e) {
    U value = r;
    for (; e; e >>= 1, a = mul(a, a)) if (e & 1) value = mul(value, a);
    return value;
  }
  static V encode(const V& a, int n = -1) {
    const std::size_t size = n < 0 ? a.size() : std::min<std::size_t>(a.size(), n);
    V b(size);
    for (std::size_t i = 0; i < size; ++i) {
      assert(a[i] < p);
      b[i] = mul(a[i], r2);
    }
    return b;
  }
  static V decode(V a) {
    for (U& x : a) x = canonical(reduce(x));
    return a;
  }
  static int degree_of(const V& f, int n) {
    assert(f.size() <= (1U << 23));
    if (n < 0) n = static_cast<int>(f.size());
    assert(n <= (1 << 23));
    return n;
  }

  struct Transform {
    V roots, inverse_roots;
    bool simd;
    Transform(int n, bool simd) : roots(n), inverse_roots(n), simd(simd) {
      assert(n <= (1 << 23));
      for (int h = 1; h < n; h <<= 1) {
        U w = power(mul(3, r2), (p - 1) / (h * 2));
        U iw = power(w, p - 2);
        roots[h] = inverse_roots[h] = r;
        for (int j = 1; j < h; ++j) {
          roots[h + j] = mul(roots[h + j - 1], w);
          inverse_roots[h + j] = mul(inverse_roots[h + j - 1], iw);
        }
      }
    }
#if BLUEBERRY_EXTRA_FAST_FPS_AVX2
    __attribute__((target("avx2"))) static __m256i multiply32(__m256i a, __m256i b) {
#if defined(BLUEBERRY_EXTRA_FAST_USE_ASM) || defined(BLUEBERRY_EXTRA_FAST_INLINE_ASM)
      __m256i c;
      asm("vpmuludq {%2, %1, %0|%0, %1, %2}" : "=x"(c) : "x"(a), "x"(b));
      return c;
#else
      return _mm256_mul_epu32(a, b);
#endif
    }
    __attribute__((target("avx2"))) static __m256i multiply(__m256i a, __m256i b) {
      const auto mod = _mm256_set1_epi32(p), inv = _mm256_set1_epi32(neg_inv);
      auto lo = multiply32(a, b);
      auto hi = multiply32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
      lo = _mm256_srli_epi64(_mm256_add_epi64(lo, multiply32(multiply32(lo, inv), mod)), 32);
      hi = _mm256_add_epi64(hi, multiply32(multiply32(hi, inv), mod));
      return _mm256_blend_epi32(lo, hi, 0xaa);
    }
    __attribute__((target("avx2"))) static void small(U* a, const U* w,
                                                    int n, int h, bool inverse) {
      alignas(32) U order[8], factors[8], mask[8];
      for (int i = 0; i < 8; ++i) {
        order[i] = i ^ h;
        factors[i] = w[i % h];
        mask[i] = i & h ? ~U(0) : 0;
      }
      const auto permutation = _mm256_load_si256(reinterpret_cast<const __m256i*>(order));
      const auto twiddles = _mm256_load_si256(reinterpret_cast<const __m256i*>(factors));
      const auto upper = _mm256_load_si256(reinterpret_cast<const __m256i*>(mask));
      const auto mod2 = _mm256_set1_epi32(p2);
      for (int i = 0; i < n; i += 8) {
        auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + i));
        if (inverse && h > 1) x = _mm256_blendv_epi8(x, multiply(x, twiddles), upper);
        auto y = _mm256_permutevar8x32_epi32(x, permutation);
        auto sum = _mm256_add_epi32(x, y);
        auto difference = _mm256_sub_epi32(_mm256_add_epi32(y, mod2), x);
        sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, mod2));
        difference = _mm256_min_epu32(difference, _mm256_sub_epi32(difference, mod2));
        if (!inverse && h > 1) difference = multiply(difference, twiddles);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(a + i),
                            _mm256_blendv_epi8(sum, difference, upper));
      }
    }
    __attribute__((target("avx2"))) static void block(U* a, U* b, const U* w,
                                                    int h, bool inverse) {
      const auto mod2 = _mm256_set1_epi32(p2);
      for (int j = 0; j < h; j += 8) {
        auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(a + j));
        auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(b + j));
        auto z = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(w + j));
        if (inverse) y = multiply(y, z);
        auto sum = _mm256_add_epi32(x, y);
        auto difference = _mm256_sub_epi32(_mm256_add_epi32(x, mod2), y);
        sum = _mm256_min_epu32(sum, _mm256_sub_epi32(sum, mod2));
        difference = _mm256_min_epu32(difference, _mm256_sub_epi32(difference, mod2));
        if (!inverse) difference = multiply(difference, z);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(a + j), sum);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(b + j), difference);
      }
    }
#endif
    // Forward DIF and inverse DIT share the same bit-reversed frequency order.
    void apply(V& a, bool inverse = false) const {
      const int n = static_cast<int>(a.size());
      for (int hh = 1; hh < n; hh <<= 1) {
        const int h = inverse ? hh : n / (hh * 2);
        const U* w = (inverse ? inverse_roots : roots).data() + h;
#if BLUEBERRY_EXTRA_FAST_FPS_AVX2
        if (simd && h < 8 && n >= 8) { small(a.data(), w, n, h, inverse); continue; }
#endif
        for (int k = 0; k < n; k += h * 2) {
          U* x = a.data() + k;
          U* y = x + h;
#if BLUEBERRY_EXTRA_FAST_FPS_AVX2
          if (simd && h >= 8) { block(x, y, w, h, inverse); continue; }
#endif
          for (int j = 0; j < h; ++j) {
            U u = x[j], v = inverse && h > 1 ? mul(y[j], w[j]) : y[j];
            x[j] = add(u, v);
            y[j] = sub(u, v);
            if (!inverse && h > 1) y[j] = mul(y[j], w[j]);
          }
        }
      }
      if (inverse) {
        const U scale = power(mul(n, r2), p - 2);
        for (U& x : a) x = mul(x, scale);
      }
    }
    V convolution(V a, V b) const {
      if (a.empty() || b.empty()) return {};
      const int m = static_cast<int>(a.size() + b.size() - 1);
      if (std::min(a.size(), b.size()) <= 32) {
        V c(m);
        for (std::size_t i = 0; i < a.size(); ++i)
          for (std::size_t j = 0; j < b.size(); ++j) c[i + j] = add(c[i + j], mul(a[i], b[j]));
        return c;
      }
      const int n = static_cast<int>(std::bit_ceil(unsigned(m)));
      a.resize(n); b.resize(n);
      apply(a); apply(b);
      for (int i = 0; i < n; ++i) a[i] = mul(a[i], b[i]);
      apply(a, true);
      a.resize(m);
      return a;
    }
    V inverse(const V& f, int n) const {
      V g(n);
      if (!n) return g;
      g[0] = power(f[0], p - 2);
      const int base = std::min(n, 32);
      for (int i = 1; i < base; ++i) {
        U value = 0;
        for (int j = 1; j <= i && j < static_cast<int>(f.size()); ++j)
          value = add(value, mul(f[j], g[i - j]));
        g[i] = mul(sub(0, value), g[0]);
      }
      for (int m = 32; m < n; m <<= 1) {
        V a(m * 2), b(m * 2);
        std::copy_n(f.begin(), std::min<int>(f.size(), m * 2), a.begin());
        std::copy_n(g.begin(), m, b.begin());
        apply(a); apply(b);
        for (int i = 0; i < m * 2; ++i) a[i] = mul(a[i], b[i]);
        apply(a, true);
        std::fill_n(a.begin(), m, 0);
        apply(a);
        for (int i = 0; i < m * 2; ++i) a[i] = mul(a[i], b[i]);
        apply(a, true);
        for (int i = m; i < std::min(m * 2, n); ++i) g[i] = sub(0, a[i]);
      }
      return g;
    }
  };

 public:
  explicit FPS998244353(bool use_simd = true) : simd_(use_simd && simd_available()) {}
  bool uses_simd() const { return simd_; }
  static bool simd_available() {
#if BLUEBERRY_EXTRA_FAST_FPS_AVX2 && !defined(BLUEBERRY_EXTRA_FAST_FORCE_SCALAR)
    return __builtin_cpu_supports("avx2");
#else
    return false;
#endif
  }

  std::vector<std::uint32_t> convolution(const V& a, const V& b) const {
    if (a.empty() || b.empty()) return {};
    assert(a.size() + b.size() - 1 <= (1U << 23));
    Transform t(std::min(a.size(), b.size()) <= 32 ? 1 :
                    static_cast<int>(std::bit_ceil(unsigned(a.size() + b.size() - 1))), simd_);
    return decode(t.convolution(encode(a), encode(b)));
  }

  std::vector<std::uint32_t> inv(const V& f, int n = -1) const {
    n = degree_of(f, n);
    if (!n) return {};
    assert(!f.empty() && f[0] != 0);
    V a = encode(f, n);
    if (a.size() == 1) {
      V g(n);
      g[0] = power(a[0], p - 2);
      return decode(std::move(g));
    }
    Transform t(static_cast<int>(std::bit_ceil(unsigned(n))), simd_);
    return decode(t.inverse(a, n));
  }

  std::vector<std::uint32_t> log(const V& f, int n = -1) const {
    n = degree_of(f, n);
    if (!n) return {};
    assert(!f.empty() && f[0] == 1 && n <= (1 << 22));
    if (f.size() == 1 || n == 1) return V(n);
    Transform t(static_cast<int>(std::bit_ceil(unsigned(std::max(1, n * 2 - 2)))), simd_);
    V a = encode(f, n), derivative(std::min<int>(a.size(), n) - 1);
    for (int i = 1; i <= static_cast<int>(derivative.size()); ++i)
      derivative[i - 1] = mul(a[i], mul(i, r2));
    V product = t.convolution(std::move(derivative), t.inverse(a, n));
    V result(n), inverses(n, r);
    for (int i = 2; i < n; ++i) inverses[i] = mul(sub(0, inverses[p % i]), mul(p / i, r2));
    for (int i = 1; i < n; ++i) result[i] = mul(i <= static_cast<int>(product.size()) ? product[i - 1] : 0, inverses[i]);
    return decode(std::move(result));
  }

  std::vector<std::uint32_t> exp(const V& f, int n = -1) const {
    n = degree_of(f, n);
    if (!n) return {};
    assert(f.empty() || f[0] == 0);
    if (n == 1) return {1};
    if (f.size() <= 1) { V g(n); g[0] = 1; return g; }
    Transform t(static_cast<int>(std::bit_ceil(unsigned(n))), simd_);
    V a = encode(f, n), g{r, a.size() > 1 ? a[1] : 0};
    V h{r}, ht{r, r}, inverses(std::bit_ceil(unsigned(n)), r);
    for (int i = 2; i < static_cast<int>(inverses.size()); ++i)
      inverses[i] = mul(sub(0, inverses[p % i]), mul(p / i, r2));
    for (int m = 2; m < n; m <<= 1) {
      V gt = g; gt.resize(m * 2); t.apply(gt);
      V correction(m);
      for (int i = 0; i < m; ++i) correction[i] = mul(gt[i], ht[i]);
      t.apply(correction, true);
      std::fill_n(correction.begin(), m / 2, 0);
      t.apply(correction);
      for (int i = 0; i < m; ++i) correction[i] = mul(sub(0, correction[i]), ht[i]);
      t.apply(correction, true);
      h.insert(h.end(), correction.begin() + m / 2, correction.end());
      ht = h; ht.resize(m * 2); t.apply(ht);
      V error(m);
      for (int i = 1; i < m && i < static_cast<int>(a.size()); ++i)
        error[i - 1] = mul(a[i], mul(i, r2));
      t.apply(error);
      for (int i = 0; i < m; ++i) error[i] = mul(error[i], gt[i]);
      t.apply(error, true);
      for (int i = 1; i < m; ++i) error[i - 1] = sub(error[i - 1], mul(g[i], mul(i, r2)));
      error.resize(m * 2);
      for (int i = 0; i < m - 1; ++i) { error[m + i] = error[i]; error[i] = 0; }
      t.apply(error);
      for (int i = 0; i < m * 2; ++i) error[i] = mul(error[i], ht[i]);
      t.apply(error, true);
      for (int i = m * 2 - 1; i >= 1; --i) error[i] = mul(error[i - 1], inverses[i]);
      for (int i = m; i < m * 2 && i < static_cast<int>(a.size()); ++i) error[i] = add(error[i], a[i]);
      std::fill_n(error.begin(), m, 0);
      t.apply(error);
      for (int i = 0; i < m * 2; ++i) error[i] = mul(error[i], gt[i]);
      t.apply(error, true);
      g.insert(g.end(), error.begin() + m, error.end());
    }
    g.resize(n);
    return decode(std::move(g));
  }
};

}  // namespace blueberry::extra_fast

#undef BLUEBERRY_EXTRA_FAST_FPS_AVX2
