#include "blueberry/utility/fast-io.hpp"
#include "trusted-input.hpp.txt"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")
#else
#include <sys/resource.h>
#endif

#ifdef TRUSTED
using Input = TrustedInput;
template<class T> bool integer(Input& input, T& value) { return input.read_int(value); }
bool word(Input& input, std::string& value) { return input.read_word(value); }
#else
using Input = blueberry::FastInput<>;
template<class T> bool integer(Input& input, T& value) { return input.read(value); }
bool word(Input& input, std::string& value) { return input.read(value); }
#endif

template<class T> int echo() {
  Input input;
  int count;
  if (!integer(input, count)) return 1;
  blueberry::FastOutput output;
  for (int i = 0; i < count; ++i) {
    T value{};
    if (!integer(input, value) || !output.writeln(value)) return 2;
  }
  T after = T(7);
  if (integer(input, after) || after != T(7)) return 3;
  return output.flush() ? 0 : 4;
}

int main(int argc, char** argv) {
  auto entry = std::chrono::steady_clock::now();
#ifdef _WIN32
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  if (argc == 2 && std::strcmp(argv[1], "i32") == 0) return echo<std::int32_t>();
  if (argc == 2 && std::strcmp(argv[1], "u32") == 0) return echo<std::uint32_t>();
  if (argc == 2 && std::strcmp(argv[1], "i64") == 0) return echo<std::int64_t>();
  if (argc == 2 && std::strcmp(argv[1], "u64") == 0) return echo<std::uint64_t>();
  Input input;
  int n, q;
  std::string bits;
  auto begin = std::chrono::steady_clock::now();
  if (!integer(input, n) || !integer(input, q) || !word(input, bits)) return 5;
  auto queries_begin = std::chrono::steady_clock::now();
  std::uint64_t checksum = 14695981039346656037ULL;
  for (int i = 0; i < q; ++i) {
    int type, value;
    if (!integer(input, type) || !integer(input, value)) return 6;
    checksum = (checksum ^ static_cast<unsigned>(type)) * 1099511628211ULL;
    checksum = (checksum ^ static_cast<unsigned>(value)) * 1099511628211ULL;
  }
  auto end = std::chrono::steady_clock::now();
  int extra = 7;
  if (integer(input, extra) || extra != 7) return 7;
  const auto ones = std::count(bits.begin(), bits.end(), '1');
  std::printf("%d %d %zu %zu %llu\n", n, q, bits.size(), static_cast<std::size_t>(ones),
              static_cast<unsigned long long>(checksum));
  std::fflush(stdout);
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS info{};
  info.cb = sizeof(info);
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) return 8;
  auto peak = static_cast<unsigned long long>(info.PeakWorkingSetSize);
  const char* memory_key = "peak_working_set_bytes";
#else
  struct rusage info{};
  if (getrusage(RUSAGE_SELF, &info)) return 8;
  auto peak = static_cast<unsigned long long>(info.ru_maxrss) * 1024;
  const char* memory_key = "peak_rss_bytes";
#endif
  auto final = std::chrono::steady_clock::now();
  std::fprintf(stderr, "{\"prefix_ms\":%.6f,\"queries_ms\":%.6f,\"parse_ms\":%.6f,\"post_parse_ms\":%.6f,\"main_ms\":%.6f,\"%s\":%llu}\n",
      std::chrono::duration<double, std::milli>(queries_begin - begin).count(),
      std::chrono::duration<double, std::milli>(end - queries_begin).count(),
      std::chrono::duration<double, std::milli>(end - begin).count(),
      std::chrono::duration<double, std::milli>(final - end).count(),
      std::chrono::duration<double, std::milli>(final - entry).count(), memory_key, peak);
}
