// Targeted follow-up: keep the initial benchmark source/hash unchanged.
#include "blueberry/geometry/convex-hull.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Points = std::vector<std::pair<long long, long long>>;
using Clock = std::chrono::steady_clock;

[[gnu::noinline]] static Points original(Points points) {
  return blueberry::convex_hull(std::move(points));
}

[[gnu::noinline]] static Points compact(Points points) {
  auto result = blueberry::convex_hull(std::move(points));
  if (result.size() <= 2) return {result.begin(), result.end()};
  return result;
}

[[gnu::noinline]] static Points guarded(Points points) {
  auto result = blueberry::convex_hull(std::move(points));
  if (result.size() <= 2 && result.capacity() > result.size())
    return {result.begin(), result.end()};
  return result;
}

int main() {
  using Function = Points (*)(Points);
  const std::vector<Function> functions{original, compact, guarded};
  const std::vector<std::string> names{"original", "compact", "guarded"};
  const std::vector<std::string> workloads{
      "empty", "singleton", "two", "all-same", "two-unique", "random", "collinear"};
  bool first = true;
  std::cout << '[';
  for (const auto& workload : workloads) {
    Points input;
    const bool tiny = workload == "empty" || workload == "singleton" || workload == "two";
    const std::size_t n = workload == "empty" ? 0 : workload == "singleton" ? 1 : workload == "two" ? 2 : 200000;
    input.reserve(n);
    std::mt19937_64 rng(0x434f4e564558ULL);
    for (std::size_t i = 0; i < n; ++i) {
      if (workload == "all-same") input.emplace_back(7, -11);
      else if (workload == "two-unique") input.emplace_back(i % 2, i % 2);
      else if (workload == "random") input.emplace_back(rng() % 1000000001, rng() % 1000000001);
      else input.emplace_back(i, 2 * i + 3);
    }
    const auto expected = original(input);
    for (auto function : functions)
      if (function(input) != expected) return 1;
    const int iterations = tiny ? 500000 : 1;
    for (int repeat = -1; repeat < 5; ++repeat) {
      for (std::size_t i = 0; i < functions.size(); ++i) {
        const auto variant = repeat % 2 == 0 ? i : functions.size() - 1 - i;
        const auto begin = Clock::now();
        std::uint64_t checksum = 0;
        for (int iteration = 0; iteration < iterations; ++iteration) {
          const auto hull = functions[variant](input);
          checksum += hull.size();
          if (!hull.empty()) checksum += std::uint64_t(hull.front().first);
        }
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
        if (repeat >= 0) {
          if (!first) std::cout << ',';
          first = false;
          const auto returned = functions[variant](input);
          std::cout << "{\"workload\":\"" << workload << "\",\"variant\":\"" << names[variant]
                    << "\",\"repeat\":" << repeat << ",\"iterations\":" << iterations
                    << ",\"elapsed_ns\":" << elapsed << ",\"checksum\":" << checksum
                    << ",\"size\":" << returned.size() << ",\"capacity\":" << returned.capacity() << '}';
        }
      }
    }
  }
  std::cout << "]\n";
}
