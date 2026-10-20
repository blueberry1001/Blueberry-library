// Many A+B workload; parsing, formatting and complete I/O measured separately.
#ifndef IO_FACADE
#define IO_FACADE 0
#endif
#if IO_FACADE
#include "blueberry/utility/contest-fast-io.hpp"
#else
#include "blueberry/utility/fast-io.hpp"
#endif
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

struct Backend {
#if !IO_FACADE
  blueberry::FastInput<> input;
  blueberry::FastOutput output;
#endif
  template<class... T> bool read(T&... value) {
#if IO_FACADE
    return scan(value...);
#else
    return input.read(value...);
#endif
  }
  template<class T> bool line(T value) {
#if IO_FACADE
    return print(value);
#else
    return output.writeln(value);
#endif
  }
  bool flush_output() {
#if IO_FACADE
    return flush();
#else
    return output.flush();
#endif
  }
};

template<class T> int run(const std::string& mode, int expected_count) {
  using Clock = std::chrono::steady_clock;
  auto ms = [](auto begin, auto end) {
    return std::chrono::duration<double, std::milli>(end-begin).count();
  };
  const auto setup_begin = Clock::now();
  Backend io;
  const auto setup_end = Clock::now();
  std::vector<T> answers;
  if (mode == "output") {
    int n;
    if (!io.read(n) || n != expected_count) return 3;
    answers.resize(n);
    for (int i = 0; i < n; ++i) {
      T a, b;
      if (!io.read(a, b)) return 3;
      answers[i] = a+b;
    }
  }
  const auto begin = Clock::now();
  std::uint64_t checksum = 0;
  auto consume = [&]<bool Emit>() {
    int n;
    if (!io.read(n) || n != expected_count) return 3;
    for (int i = 0; i < n; ++i) {
      T a, b;
      if (!io.read(a, b)) return 3;
      const T sum = a+b;
      checksum += static_cast<std::uint64_t>(sum);
      if constexpr (Emit) if (!io.line(sum)) return 4;
    }
    return 0;
  };
  if (mode == "input" || mode == "end-to-end") {
    const int result = mode == "input" ? consume.template operator()<false>()
                                       : consume.template operator()<true>();
    if (result) return result;
  } else if (mode == "output") {
    for (const T value : answers) if (!io.line(value)) return 4;
  } else return 2;
  if (!io.flush_output()) return 4;
  const auto end = Clock::now();
  if (mode == "input" && (!io.line(checksum) || !io.flush_output())) return 4;
  std::fprintf(stderr, "{\"setup_ms\":%.9f,\"operation_ms\":%.9f}\n",
               ms(setup_begin, setup_end), ms(begin, end));
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 4) return 2;
  const std::string type = argv[1], mode = argv[2];
  const int count = std::atoi(argv[3]);
  if (count <= 0) return 2;
  if (type == "i64") return run<long long>(mode, count);
  if (type == "u64") return run<unsigned long long>(mode, count);
  if (type == "i128") return run<__int128>(mode, count);
  if (type == "u128") return run<__uint128_t>(mode, count);
  return 2;
}
