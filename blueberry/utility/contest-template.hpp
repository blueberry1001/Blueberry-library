#pragma once
// Explicit opt-in: short aliases, global stream overloads and contest macros.
// This header is deliberately excluded from blueberry/all.hpp.
#include <bits/stdc++.h>
#include <atcoder/all>
#include "blueberry/utility/contest-helpers.hpp"
#include "blueberry/math/combinations.hpp"
#include "blueberry/math/prime-sieve.hpp"
#include "blueberry/graph/topological-sort.hpp"

using ll = long long;
using ld = long double;
using ull = unsigned long long;
using uint = unsigned int;
using str = std::string;
using mint = atcoder::modint998244353;
using mint1 = atcoder::modint1000000007;
template<class T> using pqg = std::priority_queue<T, std::vector<T>, std::greater<T>>;
using blueberry::bin_search;
using blueberry::chmin;
using blueberry::chmax;
using blueberry::Fill;
using blueberry::len;
using blueberry::sum;
using blueberry::min_of;
using blueberry::max_of;
using blueberry::my_distance;
using blueberry::parse128;
using blueberry::to_string128;
inline constexpr ll INF = (1LL << 62) - (1LL << 31);
inline constexpr std::array<ll, 8> dx{0, 1, 0, -1, -1, -1, 1, 1};
inline constexpr std::array<ll, 8> dy{1, 0, -1, 0, -1, 1, -1, 1};
inline constexpr ld CPS = CLOCKS_PER_SEC;

#define BLUEBERRY_REP1(a) for (ll i = 0; i < static_cast<ll>(a); ++i)
#define BLUEBERRY_REP2(i,a) for (ll i = 0; i < static_cast<ll>(a); ++i)
#define BLUEBERRY_REP3(i,a,b) for (ll i = (a); i < static_cast<ll>(b); ++i)
#define BLUEBERRY_REP4(i,a,b,c) for (ll i = (a); i < static_cast<ll>(b); i += (c))
#define BLUEBERRY_REP_SELECT(a,b,c,d,e,...) e
#define rep(...) BLUEBERRY_REP_SELECT(__VA_ARGS__,BLUEBERRY_REP4,BLUEBERRY_REP3,BLUEBERRY_REP2,BLUEBERRY_REP1)(__VA_ARGS__)
#define ALL(x) std::begin(x), std::end(x)
#define rALL(x) std::rbegin(x), std::rend(x)
#define bit(x,i) (((x) >> (i)) & 1)
#define fi first
#define se second
#define pb push_back
#define eb emplace_back
#define Endl std::endl
#define spa " "
#define YesNo(x) (std::cout << ((x) ? "Yes" : "No") << '\n')
#define YESNO(x) (std::cout << ((x) ? "YES" : "NO") << '\n')
#define popc(x) std::popcount(static_cast<ull>(x))
#define crmp(x,l,r) ((l) <= (x) && (x) <= (r))
#define O print
#define LL(...) ll __VA_ARGS__; scan(__VA_ARGS__)
#define STR(...) str __VA_ARGS__; scan(__VA_ARGS__)

inline void init_io(int precision = 30) {
  assert(precision >= 0);
  std::cin.tie(nullptr);
  std::ios::sync_with_stdio(false);
  std::cout << std::fixed << std::setprecision(precision);
}
inline std::ostream& operator<<(std::ostream& out, __int128 value) {
  return out << blueberry::to_string128(value);
}
inline std::istream& operator>>(std::istream& in, __int128& value) {
  std::string text;
  if (in >> text) {
    const auto parsed = blueberry::parse128(text);
    if (parsed) value = *parsed;
    else in.setstate(std::ios::failbit);
  }
  return in;
}
// Forward declarations allow nested vector/deque/pair combinations.
template<class T> std::istream& operator>>(std::istream&, std::vector<T>&);
template<class T> std::ostream& operator<<(std::ostream&, const std::vector<T>&);
template<class T> std::istream& operator>>(std::istream&, std::deque<T>&);
template<class T> std::ostream& operator<<(std::ostream&, const std::deque<T>&);
template<class T, class U> std::istream& operator>>(std::istream&, std::pair<T, U>&);
template<class T, class U> std::ostream& operator<<(std::ostream&, const std::pair<T, U>&);
template<class T> std::istream& operator>>(std::istream& in, std::vector<T>& values) {
  for (auto& value : values) in >> value;
  return in;
}
template<class T> std::ostream& operator<<(std::ostream& out, const std::vector<T>& values) {
  bool first = true;
  for (const auto& value : values) { if (!first) out << ' '; first = false; out << value; }
  return out;
}
template<class T> std::istream& operator>>(std::istream& in, std::deque<T>& values) {
  for (auto& value : values) in >> value;
  return in;
}
template<class T> std::ostream& operator<<(std::ostream& out, const std::deque<T>& values) {
  bool first = true;
  for (const auto& value : values) { if (!first) out << ' '; first = false; out << value; }
  return out;
}
template<class T, class U> std::istream& operator>>(std::istream& in, std::pair<T, U>& value) {
  return in >> value.first >> value.second;
}
template<class T, class U> std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& value) {
  return out << value.first << ' ' << value.second;
}
template<class... Args> void scan(Args&... values) { (void)(std::cin >> ... >> values); }
template<class... Args> void print(const Args&... values) {
  bool first = true;
  [[maybe_unused]] auto one = [&](const auto& value) {
    if (!first) std::cout << ' ';
    first = false;
    std::cout << value;
  };
  (one(values), ...);
  std::cout << '\n';
}
inline ll divide(ll a, ll b) { return blueberry::floor_div(a, b); }
inline bool iskaibun(std::string_view text) { return blueberry::is_palindrome(text); }
inline __int128 ran(ll a, ll b) { return blueberry::range_sum(a, b); }
inline int zaatu(std::vector<ll>& values) { return blueberry::compress(values); }
template<class Index> bool isbipartite_graph(const std::vector<std::vector<Index>>& graph) {
  return blueberry::is_bipartite(graph);
}
class Rande {
  std::mt19937 mt_;
 public:
  Rande() : Rande(static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count())) {}
  explicit Rande(std::uint32_t seed) : mt_(seed) {}
  int operator()(int a, int b) { assert(a < b); return std::uniform_int_distribution<int>(a, b - 1)(mt_); }
  int operator()(int b) { return (*this)(0, b); }
};
