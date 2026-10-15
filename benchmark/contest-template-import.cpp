#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/combinations.hpp"
#include "blueberry/utility/contest-helpers.hpp"
using namespace std;
using Clock = chrono::steady_clock;
volatile uint64_t benchmark_sink;

// Original user's COM algorithm, restricted to its valid n>=2, n<p contract.
struct OriginalCOM {
  vector<long long> fac, finv, inv;
  long long mod;
  OriginalCOM(int n, int p) : fac(n + 1), finv(n + 1), inv(n + 1), mod(p) {
    fac[0] = fac[1] = finv[0] = finv[1] = inv[1] = 1;
    for (int i = 2; i <= n; ++i) {
      fac[i] = fac[i - 1] * i % mod;
      inv[i] = mod - inv[mod % i] * (mod / i) % mod;
      finv[i] = finv[i - 1] * inv[i] % mod;
    }
  }
  long long C(int n, int k) const { return fac[n] * (finv[k] * finv[n - k] % mod) % mod; }
};
template<class Table> void measure(const char* name, int n, int q, const vector<pair<int,int>>& queries) {
  const auto start = Clock::now();
  Table table(n);
  const auto built = Clock::now();
  uint64_t checksum = 0;
  for (const auto& [a,b] : queries) checksum += table.C(a,b).val();
  const auto done = Clock::now();
  benchmark_sink = checksum;
  cout << name << ',' << n << ',' << q << ',' << chrono::duration<double>(built-start).count()
       << ',' << chrono::duration<double>(done-built).count() << ',' << checksum << '\n';
}
int main(int argc, char** argv) {
  const int p = argc > 1 ? stoi(argv[1]) : 998244353;
  const int n = 1000000, q = 1000000;
  mt19937 random(12345);
  vector<pair<int,int>> queries(q);
  for (auto& [a,b] : queries) { a = static_cast<int>(random() % (n+1)); b = static_cast<int>(random() % (a+1)); }
  atcoder::dynamic_modint<42>::set_mod(p);
  cout << "variant,n,q,build_seconds,query_seconds,checksum\n";
  for (int run = 0; run < 5; ++run) {
    const auto start = Clock::now(); OriginalCOM original(n,p); const auto built = Clock::now();
    uint64_t checksum = 0;
    for (auto [a,b] : queries) checksum += original.C(a,b);
    const auto done = Clock::now(); benchmark_sink = checksum;
    cout << "original-com," << n << ',' << q << ',' << chrono::duration<double>(built-start).count()
         << ',' << chrono::duration<double>(done-built).count() << ',' << checksum << '\n';
    measure<blueberry::Combinations<atcoder::dynamic_modint<42>>>("acl-dynamic-combinations",n,q,queries);
    if (p == 998244353) measure<blueberry::Combinations<atcoder::modint998244353>>("acl-static-combinations",n,q,queries);
  }
  vector<long long> values(200000); for (auto& value : values) value = random() % 100000;
  for (int run = 0; run < 5; ++run) {
    auto a = values;
    const auto start = Clock::now();
    map<long long,long long> ranks; for (auto value : a) ranks[value] = 0;
    long long count = 0; for (auto& [value,rank] : ranks) { (void)value; rank = count++; }
    for (auto& value : a) value = ranks[value];
    const auto done = Clock::now(); benchmark_sink = count;
    auto b = values; const auto fresh = Clock::now(); const int compressed = blueberry::compress(b); const auto finished = Clock::now();
    if (a != b || compressed != count) return 1;
    // Compression is one full operation; there is no separate query phase.
    cout << "original-map-compress," << a.size() << ",0," << chrono::duration<double>(done-start).count() << ",," << count << '\n';
    cout << "sorted-compress," << a.size() << ",0," << chrono::duration<double>(finished-fresh).count() << ",," << compressed << '\n';
  }
}
