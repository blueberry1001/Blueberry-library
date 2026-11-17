#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include "wavelet-matrix.hpp"
using namespace std;
using Clock = chrono::steady_clock;
int main(int argc, char** argv) {
  int n = stoi(argv[1]), q = stoi(argv[2]);
  string shape = argv[3];
  mt19937_64 gen(0x52ff031abULL);
  vector<long long> a(n);
  for (int i = 0; i < n; ++i) {
    if (shape == "full") a[i] = static_cast<long long>(gen());
    if (shape == "duplicates") a[i] = static_cast<long long>(gen() % 32) - 16;
    if (shape == "narrow") a[i] = static_cast<long long>(gen() % 2000001) - 1000000;
    if (shape == "sorted") a[i] = i / 2;
    if (shape == "extreme") a[i] = (gen() & 1) ? numeric_limits<long long>::min() : numeric_limits<long long>::max();
    if (shape == "biased") a[i] = i < 256 ? static_cast<long long>(gen()) : ((gen() & 1) ? numeric_limits<long long>::min() : numeric_limits<long long>::max());
    if (shape == "dense") a[i] = static_cast<long long>(gen() % (1 << 20));
    if (shape == "sparse-dense") a[i] = static_cast<long long>(gen() % 32) * 30000;
    if (shape == "min-dense") a[i] = numeric_limits<long long>::min() + static_cast<long long>(gen() % (1 << 20));
    if (shape == "sparse-256") a[i] = static_cast<long long>(gen() % 256) * 4000;
    if (shape == "sparse-4096") a[i] = static_cast<long long>(gen() % 4096) * 250;
  }
  vector<int> ls(q), rs(q), ks(q);
  vector<long long> low(q), high(q);
  for (int i = 0; i < q; ++i) {
    ls[i] = static_cast<int>(gen() % n); rs[i] = static_cast<int>(gen() % n);
    if (ls[i] > rs[i]) swap(ls[i], rs[i]);
    ++rs[i]; ks[i] = static_cast<int>(gen() % (rs[i] - ls[i]));
    low[i] = a[gen() % n]; high[i] = a[gen() % n];
    if (low[i] > high[i]) swap(low[i], high[i]);
  }
  auto t0 = Clock::now();
  blueberry::WaveletMatrix<long long> wm(a);
  auto t1 = Clock::now();
  PROCESS_MEMORY_COUNTERS memory{};
  memory.cb = sizeof(memory);
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory))) return 2;
  auto queries = Clock::now();
  uint64_t h = 0;
  for (int i = 0; i < q; ++i) h += static_cast<uint64_t>(wm.kth_smallest(ls[i], rs[i], ks[i]));
  auto t2 = Clock::now();
  for (int i = 0; i < q; ++i) h += static_cast<uint64_t>(wm.range_freq(ls[i], rs[i], low[i], high[i]));
  auto t3 = Clock::now();
  for (int i = 0; i < q; ++i) h += static_cast<uint64_t>(wm.range_freq(ls[i], rs[i], low[i], low[i]));
  auto t4 = Clock::now();
  for (int i = 0; i < q; ++i) h += static_cast<uint64_t>(wm.range_freq(ls[i], rs[i], low[i], low[i] == numeric_limits<long long>::max() ? low[i] : low[i] + 1));
  auto t5 = Clock::now();
  auto ms = [](auto x, auto y) { return chrono::duration<double, milli>(y - x).count(); };
  cout << h << ' ' << ms(t0, t1) << ' ' << ms(queries, t2) << ' ' << ms(t2, t3) << ' ' << ms(t3, t4) << ' ' << ms(t4, t5) << ' ' << memory.PeakWorkingSetSize << '\n';
}
