#include "blueberry/utility/contest-template.hpp"
#include <cstdlib>

namespace {
unsigned long long test_seed;
// A field without ACL's raw()/mod() APIs exercises the generic fallback.
struct SmallField {
  int value;
  SmallField(int x = 0) : value((x % 7 + 7) % 7) {}
  SmallField operator*(SmallField other) const { return SmallField(value * other.value); }
  SmallField inv() const { for (int x = 1; x < 7; ++x) if (value * x % 7 == 1) return SmallField(x); std::abort(); }
};
struct AddOnly {
  int value = 0;
  AddOnly& operator+=(const AddOnly& other) { value += other.value; return *this; }
};
void check(bool valid, const std::string& input, const std::string& expected, const std::string& actual) {
  if (!valid) {
    std::cerr << "seed=" << test_seed << " input=" << input << " expected=" << expected
              << " actual=" << actual << '\n';
    std::exit(1);
  }
}
void combinations() {
  blueberry::Combinations<SmallField> small(6);
  assert(small.C(5,2).value == 3 && small.P(5,2).value == 6 && small.fact(0).value == 1);
  using M = atcoder::dynamic_modint<17>;
  for (int prime : {2, 3, 5, 7, 31, 101}) {
    M::set_mod(prime);
    blueberry::Combinations<M> comb(prime - 1), zero(0);
    assert(zero.C(0, 0).val() == 1 && zero.P(0, 0).val() == 1);
    std::vector<std::vector<int>> pascal(prime, std::vector<int>(prime));
    for (int n = 0; n < prime; ++n) {
      pascal[n][0] = 1;
      for (int k = 1; k <= n; ++k) pascal[n][k] = (pascal[n - 1][k - 1] + pascal[n - 1][k]) % prime;
      for (int k = -1; k <= n + 1; ++k) {
        const int expected = k < 0 || k > n ? 0 : pascal[n][k];
        check(comb.C(n, k).val() == static_cast<unsigned>(expected), "C " + std::to_string(prime) + " " + std::to_string(n) + " " + std::to_string(k),
              std::to_string(expected), std::to_string(comb.C(n, k).val()));
        M permutation = 1;
        if (k < 0 || k > n) permutation = 0;
        else for (int i = 0; i < k; ++i) permutation *= n - i;
        assert(comb.P(n, k) == permutation);
      }
      assert(comb.fact(n) * comb.inv_fact(n) == 1);
    }
    assert(comb.C(-1, 0) == 0 && comb.P(-1, 0) == 0);
  }
  blueberry::Combinations<atcoder::modint998244353> a(5);
  blueberry::Combinations<atcoder::modint1000000007> b(5);
  assert(a.C(5,2).val() == 10 && b.C(5,2).val() == 10);
}
void streams() {
  std::vector<std::pair<int, int>> vp(2);
  std::pair<std::vector<int>, int> pv{std::vector<int>(2), 0};
  std::vector<std::vector<int>> vv(2, std::vector<int>(2));
  std::deque<std::pair<std::deque<int>, int>> dq(1, {std::deque<int>(2), 0});
  std::istringstream input("1 2 3 4 5 6 7 8 9 10 11 12 13 14");
  input >> vp >> pv >> vv >> dq;
  assert(input && vp[1].second == 4 && pv.second == 7 && vv[1][1] == 11 && dq[0].second == 14);
  std::ostringstream output;
  output << vp << ' ' << pv << ' ' << vv << ' ' << dq;
  assert(output.str() == "1 2 3 4 5 6 7 8 9 10 11 12 13 14");
  std::vector<int> empty; std::deque<int> ed;
  std::ostringstream empty_out; empty_out << empty << ed; assert(empty_out.str().empty());
  const auto smallest = std::numeric_limits<__int128>::min();
  const auto largest = std::numeric_limits<__int128>::max();
  std::ostringstream int_out; int_out << smallest << ' ' << largest;
  std::istringstream int_in(int_out.str()); __int128 x = 0, y = 0; int_in >> x >> y;
  assert(x == smallest && y == largest);
  for (const auto& text : {"+", "-", "12x", "--2", "170141183460469231731687303715884105728"}) {
    std::istringstream bad(text); x = 123; bad >> x; assert(bad.fail() && x == 123);
  }
  auto* original_out = std::cout.rdbuf(); std::ostringstream capture;
  std::cout.rdbuf(capture.rdbuf()); print(); print(vp, 9); YesNo(true); YESNO(false); std::cout.rdbuf(original_out);
  assert(capture.str() == "\n1 2 3 4 9\nYes\nNO\n");
  auto* original_in = std::cin.rdbuf(); std::istringstream scanner("17 ok 3 4");
  std::cin.rdbuf(scanner.rdbuf());
  { LL(n); STR(s); std::vector<int> v(2); scan(v); scan(); assert(n == 17 && s == "ok" && v[1] == 4); }
  std::cin.rdbuf(original_in);
}
}  // namespace
int main(int argc, char** argv) {
  test_seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(test_seed);
  combinations(); streams();
  const auto low = std::numeric_limits<long long>::min(), high = std::numeric_limits<long long>::max();
  for (ll a : {low, low + 1, -1LL, 0LL, 1LL, high}) for (ll b : {low, -3LL, -1LL, 1LL, 3LL, high}) {
    if (a == low && b == -1) continue;
    __int128 q = __int128(a) / b, r = __int128(a) % b;
    if (r != 0 && (r < 0) != (b < 0)) --q;
    assert(blueberry::floor_div(a, b) == q);
  }
  assert(bin_search(low, high, [](ll v) { return v <= 0; }) == 0);
  assert(bin_search(high, low, [](ll v) { return v >= 0; }) == 0);
  assert(bin_search(5,5,[](ll) { std::abort(); return false; }) == 5);
  assert(blueberry::range_sum(low, high) == __int128(low));
  assert(ran(3, 2) == 0 && ran(1,4) == 10);
  for (const auto& text : {"", "+", "-", " 1", "1 ", "1x", "--1", "+-1",
                          "170141183460469231731687303715884105728",
                          "-170141183460469231731687303715884105729"}) assert(!parse128(text));
  assert(parse128("+0").value() == 0 && parse128("-0").value() == 0 && parse128("0001").value() == 1);
  for (int round = 0; round < 2000; ++round) {
    const unsigned __int128 magnitude = (static_cast<unsigned __int128>(rng()) << 64) | rng();
    const __int128 value = static_cast<__int128>(magnitude);
    const auto text = to_string128(value);
    check(parse128(text).value() == value, text, text, to_string128(parse128(text).value()));
    ll a = static_cast<ll>(rng() % 2001) - 1000, b = static_cast<ll>(rng() % 100) + 1;
    if (rng() & 1) b = -b;
    const ll expected = static_cast<ll>(std::floor(static_cast<long double>(a) / b));
    check(divide(a,b) == expected, std::to_string(a) + "/" + std::to_string(b), std::to_string(expected), std::to_string(divide(a,b)));
    std::vector<ll> values(rng() % 25);
    for (auto& v : values) v = static_cast<ll>(rng() % 21) - 10;
    const auto original = values;
    std::set<ll> distinct(original.begin(), original.end());
    assert(zaatu(values) == static_cast<int>(distinct.size()));
    for (std::size_t i = 0; i < values.size(); ++i) assert(values[i] == std::distance(distinct.begin(), distinct.find(original[i])));
    const int n = static_cast<int>(rng() % 7);
    std::vector<std::vector<int>> graph(n);
    for (int u = 0; u < n; ++u) for (int v = u; v < n; ++v) if (rng() % 4 == 0) {
      graph[u].push_back(v); if (u != v) graph[v].push_back(u);
    }
    bool expected_bipartite = false;
    for (unsigned coloring = 0; coloring < (1U << n); ++coloring) {
      bool valid = true;
      for (int u = 0; u < n; ++u) for (int v : graph[u]) if (((coloring >> u) & 1) == ((coloring >> v) & 1)) valid = false;
      expected_bipartite |= valid;
    }
    check(isbipartite_graph(graph) == expected_bipartite, "bipartite n=" + std::to_string(n), std::to_string(expected_bipartite), std::to_string(isbipartite_graph(graph)));
  }
  for (unsigned n = 1; n <= 300; ++n) {
    std::vector<std::uint64_t> expected;
    for (unsigned d = 1; d <= n; ++d) if (n % d == 0) expected.push_back(d);
    assert(blueberry::divisors(n) == expected);
  }
  int array[2][3][4]; Fill(array, -5); assert(array[1][2][3] == -5);
  ll total = 0; rep(3) total += i; rep(j,3) total += j; rep(j,1,4) total += j; rep(j,0,6,2) total += j;
  assert(total == 18 && popc(1ULL << 40) == 1 && bit(8ULL,3) == 1 && crmp(3,1,4));
  assert(len(std::vector<int>{}) == 0 && sum(std::vector<int>{}) == 0);
  assert(sum(std::vector<AddOnly>{{1},{2},{3}}).value == 6);
  assert(min_of(std::vector<int>{2,1,3}) == 1 && max_of(std::vector<int>{2,1,3}) == 3);
  assert(iskaibun("") && iskaibun("abba") && !iskaibun("abc"));
  std::vector<std::vector<unsigned>> unsigned_graph{{1},{0}};
  assert(blueberry::is_bipartite(unsigned_graph));
  std::vector<std::vector<int>> path(100000);
  for (int i = 1; i < 100000; ++i) { path[i-1].push_back(i); path[i].push_back(i-1); }
  assert(blueberry::is_bipartite(path));
  Rande random_a(17), random_b(17), random_range(19);
  for (int i = 0; i < 1000; ++i) { assert(random_a(100) == random_b(100)); const int x = random_range(INT_MIN, INT_MAX); assert(x < INT_MAX); }
  Rande singleton(0); assert(singleton(1) == 0);
}
