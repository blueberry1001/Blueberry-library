// Same workload and formatting for stream, contest-fast and direct fast I/O.
#ifndef IO_BACKEND
#define IO_BACKEND 0
#endif
#if IO_BACKEND == 0
#include "blueberry/utility/contest-template.hpp"
#elif IO_BACKEND == 1
#include "blueberry/utility/contest-fast-io.hpp"
#else
#include "blueberry/utility/fast-io.hpp"
#endif
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct Backend {
#if IO_BACKEND == 2
  blueberry::FastInput<> input;
  blueberry::FastOutput output;
#else
  Backend() { init_io(30); }
#endif
  template<class... T> bool read(T&... value) {
#if IO_BACKEND == 0
    scan(value...); return bool(std::cin);
#elif IO_BACKEND == 1
    return scan(value...);
#else
    return input.read(value...);
#endif
  }
  template<class T> bool line(T value) {
#if IO_BACKEND == 0
    print(value); return bool(std::cout);
#elif IO_BACKEND == 1
    return print(value);
#else
    return output.writeln(value);
#endif
  }
  bool floating(double value) {
#if IO_BACKEND == 2
    return output.write(value, 30) && output.write('\n');
#else
    return line(value);
#endif
  }
  bool flush_output() {
#if IO_BACKEND == 0
    std::cout.flush(); return bool(std::cout);
#elif IO_BACKEND == 1
    return flush();
#else
    return output.flush();
#endif
  }
};

int main(int argc, char** argv) {
  if (argc != 3) return 2;
  using Clock = std::chrono::steady_clock;
  auto elapsed = [](auto begin, auto end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
  };
  const std::string mode = argv[1];
  const int count = std::atoi(argv[2]);
  if (count <= 0) return 2;
  const auto construct_begin = Clock::now();
  Backend io;
  const auto construct_end = Clock::now();
  std::vector<long long> values;
  std::vector<double> floats;
  if (mode == "output") {
    values.resize(count);
    for (int i = 0; i < count; ++i)
      values[i] = static_cast<long long>(std::uint64_t(i) * 1000000007ULL % 2000000000000000000ULL) - 1000000000000000000LL;
  } else if (mode == "float-output") {
    floats.resize(count);
    for (int i = 0; i < count; ++i) floats[i] = (i % 100000 - 50000) / 16.0;
  }
  const auto start = Clock::now();
  std::uint64_t checksum = 0;
  if (mode == "input" || mode == "end-to-end") {
    int n;
    if (!io.read(n) || n != count) return 3;
    for (int i = 0; i < n; ++i) {
      long long a, b;
      if (!io.read(a, b)) return 3;
      const long long sum = a + b;
      checksum += static_cast<std::uint64_t>(sum);
      if (mode == "end-to-end" && !io.line(sum)) return 4;
    }
  } else if (mode == "output") {
    for (const auto value : values) if (!io.line(value)) return 4;
  } else if (mode == "float-output") {
    for (const auto value : floats) if (!io.floating(value)) return 4;
  } else return 2;
  if (!io.flush_output()) return 4;
  const auto end = Clock::now();
  // Input-only emits its checksum after the timed parse loop.
  if (mode == "input" && (!io.line(checksum) || !io.flush_output())) return 4;
  std::fprintf(stderr, "{\"construction_ms\":%.9f,\"operation_ms\":%.9f}\n",
               elapsed(construct_begin, construct_end), elapsed(start, end));
}
