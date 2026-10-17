// Baselines are extracted by run.py from the immutable pre-change revision.
#include "baseline-fps.hpp"
#include "baseline-uf.hpp"
#include "blueberry/math/formal-power-series.hpp"
#include "blueberry/data-structure/potential-union-find.hpp"
#include <atcoder/modint>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>

using Mint = atcoder::modint998244353;
using Clock = std::chrono::steady_clock;
using Matrix = std::array<Mint, 4>;
struct Multiply {
  Matrix operator()(const Matrix& a, const Matrix& b) const {
    return {a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3]};
  }
};
struct Inverse {
  Matrix operator()(const Matrix& a) const { return {a[3], -a[1], -a[2], a[0]}; }
};
double ms(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now()-start).count();
}
template<bool Old> void fps_bench(const std::string& operation, int n, int repeats) {
  using FPS = std::conditional_t<Old, baseline_fps::FPS<Mint>, blueberry::FPS<Mint>>;
  FPS f(n), g(n);
  std::mt19937 rng(20261017);
  for (auto& x : f) x = rng();
  for (auto& x : g) x = rng();
  if (n) f[0] = 1;
  std::uint64_t checksum = 0;
  const auto start = Clock::now();
  for (int r = 0; r < repeats; ++r) {
    // Perturb input to prevent loop-invariant elimination; identical in both variants.
    if (n > 1) f[1] += 1;
    if (operation == "integral") {
      auto answer = f.integral();
      for (auto x : answer) checksum += x.val();
    } else if (operation == "eval") checksum += f.eval(Mint(r+2)).val();
    else if (operation == "log") {
      auto answer = f.log();
      for (auto x : answer) checksum += x.val();
    } else {
      FPS answer = f;
      answer *= g;
      for (auto x : answer) checksum += x.val();
    }
  }
  const double elapsed_ms = ms(start);
  std::cout << "NA," << elapsed_ms << ',' << checksum << '\n';
}
template<bool Old, bool Mat> void uf_bench(const std::string& operation, int n, int q) {
  using Value = std::conditional_t<Mat, Matrix, Mint>;
  using Op = std::conditional_t<Mat, Multiply, std::plus<Mint>>;
  using Inv = std::conditional_t<Mat, Inverse, std::negate<Mint>>;
  using UF = std::conditional_t<Old, baseline_uf::PotentialUnionFind<Value, Op, Inv>,
                                   blueberry::PotentialUnionFind<Value, Op, Inv>>;
  Value identity{};
  if constexpr (Mat) identity = {1,0,0,1};
  else identity = 0;
  std::mt19937 rng(20261017);
  std::vector<std::pair<int,int>> pairs(q);
  for (auto& [u,v] : pairs) { u = rng()%n; v = rng()%n; }
  std::vector<Value> weights(n);
  for (int i = 0; i < n; ++i) {
    if constexpr (Mat) {
      // Alternating upper/lower shears have determinant one and do not commute.
      weights[i] = i%2 ? Matrix{1, Mint(rng()), 0, 1} : Matrix{1, 0, Mint(rng()), 1};
    } else weights[i] = Mint(rng());
  }
  const auto build_start = Clock::now();
  UF uf(n, Op{}, Inv{}, identity);
  if (operation == "flat") {
    for (int i = 1; i < n; ++i) uf.merge(i, 0, weights[i]);
  } else if (operation == "balanced") {
    for (int step = 1; step < n; step *= 2)
      for (int i = 0; i+step < n; i += 2*step) uf.merge(i, i+step, weights[i+step]);
  }
  const double build_ms = ms(build_start);
  std::uint64_t checksum = 0;
  const auto start = Clock::now();
  for (int i = 0; i < q; ++i) {
    const auto [u,v] = pairs[i];
    if (operation == "mixed" && i%3 == 0) checksum += uf.merge(u,v,weights[u]);
    else if (operation == "mixed" && i%3 == 1) checksum += uf.same(u,v);
    else {
      auto answer = uf.diff(u,v);
      if (answer) {
        if constexpr (Mat) checksum += (*answer)[0].val() + (*answer)[1].val();
        else checksum += answer->val();
      }
    }
  }
  const double elapsed_ms = ms(start);
  std::cout << build_ms << ',' << elapsed_ms << ',' << checksum << '\n';
}
int main(int argc, char** argv) {
  if (argc != 6) return 2;
  const bool old = std::string(argv[1]) == "before";
  const std::string kind = argv[2], operation = argv[3];
  const int n = std::atoi(argv[4]), q = std::atoi(argv[5]);
  if (kind == "fps") {
    if (old) fps_bench<true>(operation,n,q); else fps_bench<false>(operation,n,q);
  } else if (kind == "matrix") {
    if (old) uf_bench<true,true>(operation,n,q); else uf_bench<false,true>(operation,n,q);
  } else {
    if (old) uf_bench<true,false>(operation,n,q); else uf_bench<false,false>(operation,n,q);
  }
}
