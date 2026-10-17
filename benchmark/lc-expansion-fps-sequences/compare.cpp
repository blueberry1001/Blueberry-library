#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/bernoulli.hpp"
#include "blueberry/math/bell.hpp"
#include "blueberry/math/partition.hpp"

using Mint = atcoder::modint998244353;
using FPS = blueberry::FPS<Mint>;
using Clock = std::chrono::steady_clock;

std::vector<Mint> run(const std::string& mode, int n) {
  if (mode == "bernoulli") return blueberry::bernoulli<Mint>(n);
  if (mode == "bell") return blueberry::bell<Mint>(n);
  if (mode == "partition") return blueberry::partition<Mint>(n);
  if (mode == "bernoulli_repeated_inverse") {
    FPS a(n + 1);
    a[0] = 1;
    for (int i = 1; i <= n; ++i) a[i] = a[i - 1] / Mint(i + 1);
    auto b = a.inv(n + 1);
    Mint fact = 1;
    for (int i = 1; i <= n; ++i) b[i] *= (fact *= Mint(i));
    return b;
  }
  if (mode == "bell_two_exp") {
    FPS x(n + 1);
    if (n) x[1] = 1;
    auto a = x.exp(n + 1);
    a[0] = 0;
    auto b = a.exp(n + 1);
    Mint fact = 1;
    for (int i = 1; i <= n; ++i) b[i] *= (fact *= Mint(i));
    return b;
  }
  if (mode == "partition_pentagonal_dp") {
    std::vector<Mint> a(n + 1);
    a[0] = 1;
    for (int i = 1; i <= n; ++i) {
      for (std::int64_t k = 1;; ++k) {
        const auto j = k * (3 * k - 1) / 2;
        if (j > i) break;
        const Mint value = a[i - j] + (j + k <= i ? a[i - j - k] : Mint(0));
        if (k & 1) a[i] += value;
        else a[i] -= value;
      }
    }
    return a;
  }
  if (mode == "partition_sigma_exp") {
    FPS a(n + 1);
    std::vector<Mint> inv(n + 1);
    if (n) inv[1] = 1;
    for (int i = 2; i <= n; ++i) inv[i] = -inv[Mint::mod() % i] * Mint(Mint::mod() / i);
    for (int i = 1; i <= n; ++i) {
      for (int j = i; j <= n; j += i) a[j] += inv[j / i];
    }
    return a.exp(n + 1);
  }
  std::abort();
}

int main(int argc, char** argv) {
  if (argc != 3) return 2;
  const std::string mode = argv[1];
  const int n = std::stoi(argv[2]);
  const auto begin = Clock::now();
  std::vector<Mint> a;
  if (mode.starts_with("io_")) {
    a.resize(n + 1);
    for (int i = 0; i <= n; ++i) a[i] = Mint(i) * Mint(1234567);
  } else a = run(mode, n);
  const auto computed = Clock::now();
  std::uint64_t checksum = 0;
  for (int i = 0; i <= n; ++i) checksum += std::uint64_t(i + 1) * a[i].val();
  const auto queried = Clock::now();
  std::string output;
  if (mode == "io_stream") {
    std::ostringstream stream;
    for (int i = 0; i <= n; ++i) stream << (i ? " " : "") << a[i].val();
    output = stream.str();
  } else {
    output.resize(static_cast<std::size_t>(n + 1) * 11);
    char* next = output.data();
    for (int i = 0; i <= n; ++i) {
      if (i) *next++ = ' ';
      next = std::to_chars(next, output.data() + output.size(), a[i].val()).ptr;
    }
    output.resize(static_cast<std::size_t>(next - output.data()));
  }
  const auto formatted = Clock::now();
  std::uint64_t parsed_sum = 0;
  if (mode == "io_stream") {
    std::istringstream stream(output);
    unsigned value;
    while (stream >> value) parsed_sum += value;
  } else {
    const char* next = output.data();
    const char* end = next + output.size();
    while (next != end) {
      unsigned value;
      next = std::from_chars(next, end, value).ptr;
      parsed_sum += value;
      if (next != end) ++next;
    }
  }
  const auto parsed = Clock::now();
  const auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
  std::cout << mode << ',' << n << ',' << ms(begin, computed) << ',' << ms(computed, queried)
            << ',' << ms(queried, formatted) << ',' << ms(formatted, parsed)
            << ',' << checksum << ',' << parsed_sum << '\n';
}
