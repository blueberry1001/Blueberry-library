#include "blueberry/data-structure/implicit-treap.hpp"
#include <array>
#include <tuple>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/implicit-treap.hpp"
#include "blueberry/data-structure/ordered-multiset.hpp"

using namespace std;
using mint = atcoder::modint998244353;
unsigned long long test_seed;
int step;
void check(bool ok, const char* what) {
  if (ok) return;
  cerr << "test_seed=" << test_seed << " step=" << step << " failure=" << what << '\n';
  exit(1);
}
// Deleted default constructors and no action equality operator catch accidental
// requirements beyond the documented monoid/action interface.
struct Sum {
  mint value; int length;
  Sum() = delete;
  Sum(mint value_, int length_) : value(value_), length(length_) {}
};
struct Affine {
  mint a, b;
  Affine() = delete;
  Affine(mint a_, mint b_) : a(a_), b(b_) {}
};
Sum sum_op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum sum_e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.value + f.b * x.length, x.length}; }
Affine compose(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine action_id() { return {1, 0}; }
using Sequence = blueberry::ImplicitTreap<Sum, sum_op, sum_e, Affine, mapping, compose, action_id>;
string concat(string a, string b) { return a + b; }
string empty_string() { return {}; }
using Text = blueberry::ImplicitTreap<string, concat, empty_string>;
struct LetterAction { int a, b; };
string map_letters(LetterAction f, string value) {
  for (char& c : value) c = static_cast<char>('a' + (f.a * (c - 'a') + f.b) % 26);
  return value;
}
LetterAction compose_letters(LetterAction f, LetterAction g) {
  return {f.a * g.a % 26, (f.a * g.b + f.b) % 26};
}
LetterAction letters_id() { return {1, 0}; }
using LazyText = blueberry::ImplicitTreap<string, concat, empty_string, LetterAction,
                                        map_letters, compose_letters, letters_id>;
struct Tracked {
  static inline int alive = 0;
  int value;
  Tracked() = delete;
  explicit Tracked(int value_) : value(value_) { ++alive; }
  Tracked(const Tracked& other) : value(other.value) { ++alive; }
  Tracked(Tracked&& other) noexcept : value(other.value) { ++alive; }
  Tracked& operator=(const Tracked&) = default;
  Tracked& operator=(Tracked&&) = default;
  ~Tracked() { --alive; }
  bool operator<(const Tracked& other) const { return value < other.value; }
};
Tracked tracked_op(Tracked a, Tracked b) { return Tracked(a.value + b.value); }
Tracked tracked_e() { return Tracked(0); }
struct ByLength { bool descending = false; bool operator()(const string& a, const string& b) const {
  return descending ? a.size() > b.size() : a.size() < b.size();
} };
using Bag = blueberry::OrderedMultiset<string, concat, empty_string, ByLength>;
long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }
using NumericBag = blueberry::OrderedMultiset<long long, add, zero>;

void verify_sequence(Sequence& tree, const vector<mint>& a) {
  check(tree.size() == static_cast<int>(a.size()) && tree.empty() == a.empty(), "sequence size");
  mint total = 0;
  auto actual = tree.to_vector();
  check(actual.size() == a.size(), "to_vector size");
  for (size_t i = 0; i < a.size(); ++i) {
    total += a[i];
    check(actual[i].value == a[i] && actual[i].length == 1, "to_vector item");
    check(tree.get(i).value == a[i], "get");
  }
  check(tree.all_prod().value == total && tree.all_prod().length == static_cast<int>(a.size()), "all_prod");
  check(tree.prod(0, 0).value == mint(0) && tree.prod(tree.size(), tree.size()).length == 0, "empty prod");
}

using Array = std::array<int, 2>;
Array array_op(Array a, Array b) { return {a[0] + b[0], a[1] + b[1]}; }
Array array_e() { return {0, 0}; }
using Tuple = std::tuple<int, int>;
Tuple tuple_op(Tuple a, Tuple b) { return {get<0>(a) + get<0>(b), get<1>(a) + get<1>(b)}; }
Tuple tuple_e() { return {0, 0}; }
int main(int argc, char** argv) {
  blueberry::ImplicitTreap<Array, array_op, array_e> arrays({{1, 2}, {3, 4}});
  arrays.reverse(0, 2);
  check(arrays.get(0) == Array{3, 4}, "header before array ADL swap");
  blueberry::ImplicitTreap<Tuple, tuple_op, tuple_e> tuples({{1, 2}, {3, 4}});
  tuples.reverse(0, 2);
  check(tuples.get(0) == Tuple{3, 4}, "header before tuple ADL swap");
  test_seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(test_seed);
  Sequence tree;
  vector<mint> a;
  for (step = 0; step < 4000; ++step) {
    const int n = a.size();
    int l = rng() % (n + 1), r = rng() % (n + 1);
    if (l > r) swap(l, r);
    const int operation = rng() % 8;
    if (operation == 0 || n == 0) {
      mint value = rng() % 100;
      tree.insert(l, Sum(value, 1)); a.insert(a.begin() + l, value);
    } else if (operation == 1) {
      int p = rng() % n;
      tree.erase(p); a.erase(a.begin() + p);
    } else if (operation == 2) {
      int p = rng() % n; mint value = rng() % 100;
      tree.set(p, Sum(value, 1)); a[p] = value;
    } else if (operation == 3) {
      tree.reverse(l, r); reverse(a.begin() + l, a.begin() + r);
    } else if (operation <= 5) {
      Affine f(rng() % 5, rng() % 10);
      tree.apply(l, r, f);
      for (int i = l; i < r; ++i) a[i] = f.a * a[i] + f.b;
    } else {
      mint expected = 0;
      for (int i = l; i < r; ++i) expected += a[i];
      check(tree.prod(l, r).value == expected, "range affine/sum");
    }
    if (step % 50 == 0) verify_sequence(tree, a);
  }
  verify_sequence(tree, a);
  // Copy while lazy/reverse flags are pending, then mutate independent owners.
  tree.apply(0, tree.size(), Affine(3, 7));
  for (auto& value : a) value = value * 3 + 7;
  tree.reverse(0, tree.size()); reverse(a.begin(), a.end());
  Sequence copied(tree), assigned; assigned = tree;
  tree.clear(); check(tree.empty(), "sequence clear");
  verify_sequence(copied, a); verify_sequence(assigned, a);
  Sequence moved(std::move(copied));
  check(copied.empty(), "sequence moved-from");
  copied.insert(0, Sum(9, 1)); check(copied.get(0).value == mint(9), "sequence moved-from reuse");
  assigned = std::move(moved); check(moved.empty(), "sequence move assignment");
  verify_sequence(assigned, a);
  Sequence* alias = &assigned;
  assigned = *alias; assigned = std::move(*alias); verify_sequence(assigned, a);

  Text text(vector<string>{"a", "b", "c", "d"});
  vector<string> letters{"a", "b", "c", "d"};
  for (step = 0; step < 1000; ++step) {
    int n = letters.size(), l = rng() % (n + 1), r = rng() % (n + 1);
    if (l > r) swap(l, r);
    if (step % 3 == 0 && n < 40) {
      string value(1, 'a' + rng() % 26);
      text.insert(l, value); letters.insert(letters.begin() + l, value);
    } else if (step % 3 == 1 && n) {
      int p = rng() % n; text.erase(p); letters.erase(letters.begin() + p);
    } else {
      text.reverse(l, r); reverse(letters.begin() + l, letters.begin() + r);
    }
    string expected;
    for (const auto& value : letters) expected += value;
    check(text.all_prod() == expected && text.prod(0, text.size()) == expected, "noncommutative reverse");
  }
  for (bool descending : {false, true}) {
    ByLength compare{descending}; Bag bag(compare); vector<string> sorted;
    for (step = 0; step < 3000; ++step) {
      string value(1 + rng() % 5, 'a' + rng() % 26);
      auto lower = lower_bound(sorted.begin(), sorted.end(), value, compare);
      auto upper = upper_bound(sorted.begin(), sorted.end(), value, compare);
      int first = lower - sorted.begin(), last = upper - sorted.begin();
      check(bag.rank(value) == first && bag.count(value) == last - first, "duplicate rank/count");
      check(bag.contains(value) == (first != last), "contains equivalence");
      check(bag.lower_bound(value) == (lower == sorted.end() ? optional<string>{} : *lower), "lower_bound");
      check(bag.upper_bound(value) == (upper == sorted.end() ? optional<string>{} : *upper), "upper_bound");
      check(bag.floor(value) == (upper == sorted.begin() ? optional<string>{} : *(upper - 1)), "floor");
      if (rng() % 3) {
        bag.insert(value); sorted.insert(upper, value);
      } else {
        check(bag.erase(value) == (first != last), "erase absent/equivalent");
        if (first != last) sorted.erase(lower);
      }
      if (!sorted.empty() && rng() % 4 == 0) {
        int p = rng() % sorted.size(); bag.erase_kth(p); sorted.erase(sorted.begin() + p);
      }
      check(bag.size() == static_cast<int>(sorted.size()), "multiset size");
      check(!bag.kth(-1) && !bag.kth(bag.size()), "invalid kth");
      int l = rng() % (sorted.size() + 1), r = rng() % (sorted.size() + 1);
      if (l > r) swap(l, r);
      string expected;
      for (int i = l; i < r; ++i) expected += sorted[i];
      check(bag.prod(l, r) == expected, "noncommutative rank aggregate");
      for (int i = 0; i < static_cast<int>(sorted.size()); ++i)
        check(bag.kth(i).value() == sorted[i], "stable equal-class order");
    }
    Bag copy = bag, assignment; assignment = bag;
    bag.clear(); check(bag.empty(), "multiset clear");
    Bag moved_bag(std::move(copy)); check(copy.empty(), "bag moved-from");
    copy.insert("x"); check(copy.kth(0).value() == "x", "bag moved-from reuse");
    assignment = std::move(moved_bag); check(moved_bag.empty(), "bag move assignment");
    string expected;
    for (const auto& value : sorted) expected += value;
    check(assignment.all_prod() == expected, "bag copy/move independence");
  }
  LazyText lazy_letters(vector<string>{"ab", "c", "def", "gh"});
  vector<string> words{"ab", "c", "def", "gh"};
  for (step = 0; step < 800; ++step) {
    int l = rng() % 5, r = rng() % 5;
    if (l > r) swap(l, r);
    LetterAction action{static_cast<int>(rng() % 26), static_cast<int>(rng() % 26)};
    lazy_letters.apply(l, r, action);
    for (int i = l; i < r; ++i) words[i] = map_letters(action, words[i]);
    l = rng() % 5; r = rng() % 5; if (l > r) swap(l, r);
    lazy_letters.reverse(l, r); reverse(words.begin() + l, words.begin() + r);
    string expected;
    for (int i = l; i < r; ++i) expected += words[i];
    check(lazy_letters.prod(l, r) == expected, "lazy action + noncommutative reversal");
  }
  {
    blueberry::OrderedMultiset<Tracked, tracked_op, tracked_e> tracked;
    for (step = 0; step < 10000; ++step) {
      tracked.insert(Tracked(step)); tracked.erase_kth(0);
      check(Tracked::alive <= 2, "pool slots reused and erased payload released");
    }
    tracked.clear(); check(Tracked::alive == 0, "clear releases pool objects");
  }
  check(Tracked::alive == 0, "destructor releases pool objects");
  NumericBag recycle;
  for (step = 0; step < 10000; ++step) {
    recycle.insert(step); check(recycle.erase(step), "recycle erase");
    check(recycle.empty() && recycle.count(step) == 0 && recycle.prod(0, 0) == 0, "recycle empty");
  }
  cout << "PASS legacy-treap test_seed=" << test_seed << '\n';
}
