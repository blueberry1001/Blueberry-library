#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Clock = std::chrono::steady_clock;
using Coord = long long;
struct Update { Coord x, y; long long value; };
struct Query { Coord l, d, r, u; };
struct Result { long long ns = 0; std::uint64_t checksum = 0; };

void mix(std::uint64_t& hash, std::uint64_t value) {
  hash ^= value + UINT64_C(0x9e3779b97f4a7c15) + (hash << 6) + (hash >> 2);
}
std::pair<Coord, Coord> make_range(std::mt19937_64& rng, Coord n, int kind) {
  if (kind == 2) return {0, n};
  if (kind == 1) {
    const Coord l = rng() % (n / 4 + 1);
    const Coord base = n / 4 * 3;
    return {l, base + static_cast<Coord>(rng() % (n - base + 1))};
  }
  const Coord l = rng() % (n + 1);
  const Coord width = rng() % (std::min<Coord>(16, n) + 1);
  return {l, l + std::min(width, n - l)};
}
long long ns(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}
int main(int argc, char** argv) {
  if (argc != 8) return 2;
  const std::string scenario = argv[1];
  const Coord n = std::strtoll(argv[2], nullptr, 10);
  const int update_count = std::atoi(argv[3]);
  const int query_count = std::atoi(argv[4]);
  const std::uint64_t seed = std::strtoull(argv[5], nullptr, 10);
  const int rotation = std::atoi(argv[6]);
  const unsigned mask = std::strtoul(argv[7], nullptr, 10);
  std::vector<int> shapes;
  for (int shape = 0; shape < 9; ++shape) if (mask & (1u << shape)) shapes.push_back(shape);
  if (n <= 0 || update_count < 0 || query_count < 0 || mask == 0 || mask >= (1u << 9) ||
      rotation < 0 || rotation >= static_cast<int>(shapes.size())) return 2;
  std::mt19937_64 rng(seed);
  std::vector<Update> updates(update_count);
  std::array<std::vector<Query>, 9> queries;
  std::uint64_t input_checksum = UINT64_C(0xcbf29ce484222325);
  for (auto& x : updates) {
    x = {static_cast<Coord>(rng() % n), static_cast<Coord>(rng() % n),
         static_cast<long long>(rng() % 101) - 50};
    mix(input_checksum, x.x); mix(input_checksum, x.y); mix(input_checksum, x.value);
  }
  for (int shape : shapes) {
    std::mt19937_64 query_rng(seed ^ (UINT64_C(0x9e3779b97f4a7c15) * (shape + 1)));
    queries[shape].resize(query_count);
    for (auto& q : queries[shape]) {
      auto [l, r] = make_range(query_rng, n, shape / 3);
      auto [d, u] = make_range(query_rng, n, shape % 3);
      q = {l, d, r, u};
      mix(input_checksum, l); mix(input_checksum, d); mix(input_checksum, r); mix(input_checksum, u);
    }
  }
  const auto start = Clock::now();
  blueberry::DynamicFenwickTree2D<long long, Coord> tree(n, n);
  const auto built = Clock::now();
  for (const auto& x : updates) tree.add(x.x, x.y, x.value);
  const auto updated = Clock::now();
  std::array<Result, 9> result;
  for (int step = 0; step < static_cast<int>(shapes.size()); ++step) {
    const int shape = shapes[(step + rotation) % shapes.size()];
    std::uint64_t hash = UINT64_C(0xcbf29ce484222325);
    const auto q_start = Clock::now();
    for (const auto& q : queries[shape]) mix(hash, tree.sum(q.l, q.d, q.r, q.u));
    result[shape] = {ns(q_start, Clock::now()), hash};
  }
  static const char* names[] = {"short", "wide", "full"};
  std::cout << "{\"scenario\":\"" << scenario << "\",\"domain\":" << n
            << ",\"updates\":" << update_count << ",\"queries_per_shape\":" << query_count
            << ",\"seed\":" << seed << ",\"rotation\":" << rotation
            << ",\"input_checksum\":\"" << input_checksum << "\",\"construct_ns\":"
            << ns(start, built) << ",\"update_ns\":" << ns(built, updated) << ",\"queries\":{";
  bool first = true;
  for (int shape : shapes) {
    if (!first) std::cout << ',';
    first = false;
    std::cout << '"' << names[shape / 3] << '_' << names[shape % 3]
              << "\":{\"ns\":" << result[shape].ns << ",\"checksum\":\""
              << result[shape].checksum << "\"}";
  }
  std::cout << "}}\n";
}
