#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

#include "blueberry/data-structure/sparse-table.hpp"

#ifndef BENCH_IMPL
#define BENCH_IMPL 0
#endif

namespace {

using Clock = std::chrono::steady_clock;

struct Minimum {
  int operator()(int left, int right) const { return std::min(left, right); }
};

std::uint64_t splitmix64(std::uint64_t& state) {
  std::uint64_t value = (state += 0x9e3779b97f4a7c15ULL);
  value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
  value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
  return value ^ (value >> 31);
}

class FlatSparseTable {
 public:
  explicit FlatSparseTable(const std::vector<int>& values)
      : n_(static_cast<int>(values.size())), levels_(std::bit_width(values.size())),
        table_(static_cast<std::size_t>(n_) * levels_) {
    std::copy(values.begin(), values.end(), table_.begin());
    for (int level = 1; level < levels_; ++level) {
      const int half = 1 << (level - 1);
      const int length = half << 1;
      const std::size_t current = static_cast<std::size_t>(level) * n_;
      const std::size_t previous = static_cast<std::size_t>(level - 1) * n_;
      for (int left = 0; left + length <= n_; ++left) {
        table_[current + left] = std::min(table_[previous + left], table_[previous + left + half]);
      }
    }
  }

  int product(int left, int right) const {
    const int level = std::bit_width(static_cast<unsigned>(right - left)) - 1;
    const int length = 1 << level;
    const std::size_t offset = static_cast<std::size_t>(level) * n_;
    return std::min(table_[offset + left], table_[offset + right - length]);
  }

  std::size_t estimated_bytes() const { return table_.size() * sizeof(int); }

 private:
  int n_;
  int levels_;
  std::vector<int> table_;
};

// A from-scratch linear-space reference for performance diagnosis only. Each
// 64-element block is answered with a monotone-stack bit mask; complete blocks
// are covered by a much smaller sparse table.
class BlockedSparseTable {
 public:
  static constexpr int kBlock = 64;

  explicit BlockedSparseTable(const std::vector<int>& values)
      : values_(values), masks_(values.size()), blocks_((values.size() + kBlock - 1) / kBlock) {
    for (int block = 0; block < blocks_; ++block) {
      const int begin = block * kBlock;
      const int end = std::min<int>(values_.size(), begin + kBlock);
      std::uint64_t mask = 0;
      for (int i = begin; i < end; ++i) {
        mask <<= 1;
        while (mask != 0) {
          const int distance = std::countr_zero(mask);
          if (values_[i - distance] < values_[i]) break;
          mask &= mask - 1;
        }
        masks_[i] = mask | 1;
        mask = masks_[i];
      }
    }

    block_levels_ = blocks_ == 0 ? 0 : std::bit_width(static_cast<unsigned>(blocks_));
    block_table_.assign(static_cast<std::size_t>(blocks_) * block_levels_,
                        std::numeric_limits<int>::max());
    for (int block = 0; block < blocks_; ++block) {
      const int begin = block * kBlock;
      const int end = std::min<int>(values_.size(), begin + kBlock);
      block_table_[block] = small(begin, end);
    }
    for (int level = 1; level < block_levels_; ++level) {
      const int half = 1 << (level - 1);
      const int length = half << 1;
      const std::size_t current = static_cast<std::size_t>(level) * blocks_;
      const std::size_t previous = static_cast<std::size_t>(level - 1) * blocks_;
      for (int left = 0; left + length <= blocks_; ++left) {
        block_table_[current + left] =
            std::min(block_table_[previous + left], block_table_[previous + left + half]);
      }
    }
  }

  int product(int left, int right) const {
    const int first_block = left / kBlock;
    const int last_block = (right - 1) / kBlock;
    if (first_block == last_block) return small(left, right);

    int answer = std::min(small(left, (first_block + 1) * kBlock),
                          small(last_block * kBlock, right));
    const int block_left = first_block + 1;
    const int block_right = last_block;
    if (block_left < block_right) {
      const int level = std::bit_width(static_cast<unsigned>(block_right - block_left)) - 1;
      const int length = 1 << level;
      const std::size_t offset = static_cast<std::size_t>(level) * blocks_;
      answer = std::min(answer, std::min(block_table_[offset + block_left],
                                         block_table_[offset + block_right - length]));
    }
    return answer;
  }

  std::size_t estimated_bytes() const {
    return values_.size() * sizeof(int) + masks_.size() * sizeof(std::uint64_t) +
           block_table_.size() * sizeof(int);
  }

 private:
  int small(int left, int right) const {
    const int width = right - left;
    const std::uint64_t allowed =
        width == kBlock ? ~std::uint64_t{0} : (std::uint64_t{1} << width) - 1;
    const std::uint64_t candidates = masks_[right - 1] & allowed;
    const int distance = std::bit_width(candidates) - 1;
    return values_[right - 1 - distance];
  }

  std::vector<int> values_;
  std::vector<std::uint64_t> masks_;
  int blocks_;
  int block_levels_ = 0;
  std::vector<int> block_table_;
};

class BlueberrySparseTable {
 public:
  explicit BlueberrySparseTable(const std::vector<int>& values) : table_(values, Minimum{}) {
    const int n = static_cast<int>(values.size());
    levels_ = std::bit_width(static_cast<unsigned>(n));
    entries_ = n;
    for (int level = 1; level < levels_; ++level) entries_ += n - (1 << level) + 1;
    entries_ += n + 1;  // SparseTable::log_.
  }

  int product(int left, int right) const { return table_.product(left, right); }

  std::size_t estimated_bytes() const { return entries_ * sizeof(int); }

 private:
  blueberry::SparseTable<int, Minimum> table_;
  int levels_ = 0;
  std::size_t entries_ = 0;
};

#if BENCH_IMPL == 0
using Table = BlueberrySparseTable;
constexpr std::string_view kImplementation = "blueberry";
#elif BENCH_IMPL == 1
using Table = FlatSparseTable;
constexpr std::string_view kImplementation = "flat";
#elif BENCH_IMPL == 2
using Table = BlockedSparseTable;
constexpr std::string_view kImplementation = "blocked";
#else
#error "unknown BENCH_IMPL"
#endif

double seconds(Clock::duration duration) {
  return std::chrono::duration<double>(duration).count();
}

int read_positive(const char* text, const char* name) {
  const long long value = std::strtoll(text, nullptr, 10);
  if (value <= 0 || value > std::numeric_limits<int>::max()) {
    std::cerr << name << " must be in [1, INT_MAX]\n";
    std::exit(2);
  }
  return static_cast<int>(value);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cerr << "usage: static_rmq_benchmark N Q PASSES SEED random|small\n";
    return 2;
  }
  const int n = read_positive(argv[1], "N");
  const int q = read_positive(argv[2], "Q");
  const int passes = read_positive(argv[3], "PASSES");
  std::uint64_t state = std::strtoull(argv[4], nullptr, 10);
  const std::string_view workload = argv[5];
  if (workload != "random" && workload != "small") {
    std::cerr << "workload must be random or small\n";
    return 2;
  }

  std::vector<int> values(n);
  for (int& value : values) value = static_cast<int>(splitmix64(state) & 0x7fffffffU);
  std::vector<std::pair<int, int>> queries(q);
  for (auto& [left, right] : queries) {
    if (workload == "small") {
      left = static_cast<int>(splitmix64(state) % n);
      right = std::min(n, left + 1 + static_cast<int>(splitmix64(state) % 64));
    } else {
      left = static_cast<int>(splitmix64(state) % n);
      right = static_cast<int>(splitmix64(state) % n);
      if (left > right) std::swap(left, right);
      ++right;
    }
  }

  const auto build_start = Clock::now();
  const Table table(values);
  const auto build_end = Clock::now();

  std::uint64_t checksum = 0;
  const auto query_start = Clock::now();
  for (int pass = 0; pass < passes; ++pass) {
    for (const auto& [left, right] : queries) {
      checksum += static_cast<std::uint32_t>(table.product(left, right));
      checksum ^= checksum << 7;
      checksum ^= checksum >> 9;
    }
  }
  const auto query_end = Clock::now();

  std::cout << "{\"implementation\":\"" << kImplementation << "\",\"workload\":\""
            << workload << "\",\"checksum\":" << checksum << ",\"build_seconds\":"
            << seconds(build_end - build_start) << ",\"query_seconds\":"
            << seconds(query_end - query_start) << ",\"estimated_structure_bytes\":"
            << table.estimated_bytes() << "}\n";
}
