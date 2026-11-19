#include "blueberry/string/suffix-automaton.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>
#if defined(__linux__)
#include <sys/resource.h>
#endif

using Clock = std::chrono::steady_clock;
using U64 = std::uint64_t;
struct Result { long long ns = 0; U64 checksum = 0; };
void mix(U64& state, U64 value) { state ^= value + UINT64_C(0x9e3779b97f4a7c15) + (state << 6) + (state >> 2); }
long long elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}

template<class T> int run(const std::string& scenario, int n, int q, int small, int large, U64 seed, int rotation) {
  std::mt19937_64 rng(seed);
  std::vector<T> input(n);
  for (int i=0;i<n;++i) {
    unsigned value;
    if (scenario == "unary") value = 1;
    else if (scenario == "periodic") value = i % 5;
    else if (scenario == "small") value = rng() % 4;
    else if (scenario == "large") value = rng() % 96;
    else value = rng() % 65536;
    input[i] = static_cast<T>(value);
  }
  U64 data_hash = UINT64_C(0xcbf29ce484222325);
  for (const auto& value: input) mix(data_hash, static_cast<U64>(value));
  std::array<std::vector<std::vector<T>>, 4> queries;
  for (int kind=0;kind<4;++kind) {
    int length = kind < 2 ? small : large;
    queries[kind].resize(q);
    for (auto& pattern : queries[kind]) {
      int pos = rng() % (n - length + 1);
      pattern.assign(input.begin() + pos, input.begin() + pos + length);
      if (kind == 1) std::fill(pattern.begin(), pattern.end(), static_cast<T>(-1));
      if (kind == 3) for (int i=3;i<length;i+=7) pattern[i] = static_cast<T>(-1);
      for (const auto& value: pattern) mix(data_hash, static_cast<U64>(value));
    }
  }
  std::vector<T> first(input.begin(), input.begin() + small);
  auto started = Clock::now();
  blueberry::SuffixAutomaton<T> automaton(input);
  auto built = Clock::now();
  auto first_count = automaton.count(first);
  auto counted = Clock::now();
  std::array<Result, 4> lcs, count;
  for (int step=0;step<4;++step) {
    int kind = (step + rotation) % 4;
    U64 checksum = UINT64_C(0xcbf29ce484222325);
    auto a = Clock::now();
    for (const auto& pattern : queries[kind]) {
      auto [left, right, length] = automaton.longest_common_substring(pattern);
      mix(checksum, left); mix(checksum, right); mix(checksum, length);
    }
    lcs[kind] = {elapsed(a, Clock::now()), checksum};
    checksum = UINT64_C(0xcbf29ce484222325);
    a = Clock::now();
    for (const auto& pattern : queries[kind]) mix(checksum, automaton.count(pattern));
    count[kind] = {elapsed(a, Clock::now()), checksum};
  }
  long long max_rss = -1;
#if defined(__linux__)
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage) == 0) max_rss = usage.ru_maxrss;
#endif
  static const char* names[] = {"short_match", "short_miss", "long_match", "long_mismatch"};
  std::cout << "{\"scenario\":\"" << scenario << "\",\"input_hash\":\"" << data_hash
            << "\",\"states\":" << automaton.states() << ",\"distinct\":" << automaton.distinct_substrings()
            << ",\"first_count\":" << first_count << ",\"construct_ns\":" << elapsed(started,built)
            << ",\"first_count_ns\":" << elapsed(built,counted) << ",\"peak_rss_kib\":" << max_rss << ",\"queries\":{";
  for (int i=0;i<4;++i) {
    if (i) std::cout << ',';
    std::cout << '"' << names[i] << "\":{\"lcs_ns\":" << lcs[i].ns << ",\"lcs_checksum\":\"" << lcs[i].checksum
              << "\",\"count_ns\":" << count[i].ns << ",\"count_checksum\":\"" << count[i].checksum << "\"}";
  }
  std::cout << "}}\n";
  return 0;
}
int main(int argc, char** argv) {
  if (argc != 8) return 2;
  std::string scenario = argv[1];
  int n = std::atoi(argv[2]), q = std::atoi(argv[3]);
  int small = std::atoi(argv[4]), large = std::atoi(argv[5]);
  auto seed = std::strtoull(argv[6], nullptr, 10);
  int rotation = std::atoi(argv[7]);
  if (n < 1 || q < 1 || small < 1 || large < 1 || small > n || large > n || rotation < 0 || rotation >= 4) return 2;
  if (scenario == "wide_int") return run<int>(scenario,n,q,small,large,seed,rotation);
  if (scenario == "unary" || scenario == "periodic" || scenario == "small" || scenario == "large")
    return run<char>(scenario,n,q,small,large,seed,rotation);
  return 2;
}
