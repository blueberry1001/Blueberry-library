// Independent recurrence variants; no third-party implementation is embedded.
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <atcoder/modint>
#include "blueberry/math/sparse-exp.hpp"

using Mint = atcoder::modint998244353;
using Clock = std::chrono::steady_clock;
using Terms = std::vector<std::pair<int, Mint>>;

double ms(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

std::uint64_t hash_bytes(const std::string& s) {
  std::uint64_t h = 14695981039346656037ULL;
  for (unsigned char c : s) h = (h ^ c) * 1099511628211ULL;
  return h;
}

std::string input(int n, int k, const std::string& distribution) {
  std::ostringstream out;
  out << n << ' ' << k << '\n';
  // Reverse order exercises the documented unordered-input normalization.
  for (int j = k; j >= 1; --j) {
    int d = j;
    if (distribution == "late") d = n - k + j - 1;
    if (distribution == "spread") d = 1 + static_cast<long long>(j - 1) * (n - 2) / std::max(1, k - 1);
    out << d << ' ' << (48271LL * j + 1234567) % Mint::mod() << '\n';
  }
  return out.str();
}

Terms normalize(const Terms& terms, int n) {
  Terms s;
  s.reserve(terms.size());
  Mint constant = 0;
  for (auto [d, c] : terms) {
    if (d < 0) throw std::runtime_error("negative degree");
    if (!d) constant += c;
    else if (d < n && c != Mint(0)) s.emplace_back(d, c);
  }
  if (constant != Mint(0)) throw std::runtime_error("nonzero constant");
  std::sort(s.begin(), s.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
  std::size_t count = 0;
  for (auto term : s) {
    if (count && s[count - 1].first == term.first) s[count - 1].second += term.second;
    else s[count++] = term;
  }
  s.resize(count);
  count = 0;
  for (auto [d, c] : s) if (c != Mint(0)) s[count++] = {d, c * Mint(d)};
  s.resize(count);
  return s;
}

struct Result {
  blueberry::FPS<Mint> coefficients;
  double normalize_ms = -1, inverse_ms = -1, recurrence_ms = -1;
};

Result independent(const Terms& terms, int n, const std::string& method) {
  Result r;
  auto start = Clock::now();
  auto s = normalize(terms, n);
  r.normalize_ms = ms(start);
  start = Clock::now();
  std::vector<Mint> inverse;
  if (method != "division" && n && !s.empty()) {
    inverse.resize(n);
    if (n > 1) inverse[1] = 1;
    for (int i = 2; i < n; ++i) inverse[i] = -Mint(Mint::mod() / i) * inverse[Mint::mod() % i];
  }
  r.inverse_ms = ms(start);
  start = Clock::now();
  r.coefficients.resize(n);
  if (n) r.coefficients[0] = 1;
  // Each product is <= (p-1)^2. Reduce before a uint64 sum could overflow.
  constexpr std::uint64_t largest = std::uint64_t(Mint::mod() - 1) * (Mint::mod() - 1);
  constexpr int block = static_cast<int>(std::min<std::uint64_t>(16, std::numeric_limits<std::uint64_t>::max() / largest));
  for (int i = 1; i < n && !s.empty(); ++i) {
    Mint value = 0;
    if (method == "batch") {
      std::uint64_t sum = 0;
      int used = 0;
      for (auto [d, c] : s) {
        if (d > i) break;
        sum += std::uint64_t(c.val()) * r.coefficients[i - d].val();
        if (++used == block) {
          value += Mint::raw(static_cast<int>(sum % Mint::mod()));
          used = 0;
          sum = 0;
        }
      }
      value += Mint::raw(static_cast<int>(sum % Mint::mod()));
    } else {
      for (auto [d, c] : s) {
        if (d > i) break;
        value += c * r.coefficients[i - d];
      }
    }
    r.coefficients[i] = method == "division" ? value / Mint(i) : value * inverse[i];
  }
  r.recurrence_ms = ms(start);
  return r;
}

int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const std::string method = argv[1], distribution = argv[4];
  int n = std::stoi(argv[2]), k = std::stoi(argv[3]);
  if (n < 2 || k < 1 || k >= n || (method != "api" && method != "table" && method != "division" && method != "batch")) return 2;
  const auto bytes = input(n, k, distribution);
  auto start = Clock::now();
  std::istringstream in(bytes);
  int parsed_n, parsed_k;
  in >> parsed_n >> parsed_k;
  Terms terms;
  terms.reserve(parsed_k);
  for (int j = 0; j < parsed_k; ++j) {
    int d, c;
    in >> d >> c;
    terms.emplace_back(d, Mint(c));
  }
  double parse_ms = ms(start);
  start = Clock::now();
  Result r;
  if (method == "api") r.coefficients = blueberry::sparse_exp<Mint>(terms, parsed_n);
  else r = independent(terms, parsed_n, method);
  double algorithm_ms = ms(start);
  start = Clock::now();
  std::string formatted;
  formatted.reserve(static_cast<std::size_t>(n) * 11);
  char buffer[16];
  for (auto c : r.coefficients) {
    auto converted = std::to_chars(buffer, buffer + sizeof buffer, c.val());
    formatted.append(buffer, converted.ptr);
    formatted += '\n';
  }
  double format_ms = ms(start);
  std::cout.precision(12);
  std::cout << "{\"parse_ms\":" << parse_ms << ",\"algorithm_ms\":" << algorithm_ms
            << ",\"format_ms\":" << format_ms << ",\"input_hash\":\"" << hash_bytes(bytes)
            << "\",\"output_hash\":\"" << hash_bytes(formatted) << "\",\"output_bytes\":" << formatted.size();
  if (method != "api") {
    std::cout << ",\"normalize_ms\":" << r.normalize_ms << ",\"inverse_ms\":" << r.inverse_ms
              << ",\"recurrence_ms\":" << r.recurrence_ms;
  }
  std::cout << "}\n";
}
