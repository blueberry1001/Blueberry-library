#include "blueberry/graph/offline-dynamic-component-sum.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using Wide = __int128;

// A sum type need not have a default constructor, identity constructor, +=,
// subtraction, equality, or ordering. Its stored integer is only for the oracle.
struct Value {
  Wide value;
 private:
  explicit Value(Wide x) : value(x) {}
 public:
  Value() = delete;
  static Value from(Wide x) { return Value(x); }
  Value(const Value&) = default;
  Value& operator=(const Value&) = default;
  friend Value operator+(const Value& a, const Value& b) { return Value(a.value + b.value); }
  Value& operator+=(const Value&) = delete;
  Value operator-(const Value&) const = delete;
  bool operator==(const Value&) const = delete;
  bool operator<(const Value&) const = delete;
};
static_assert(!std::is_default_constructible_v<Value>);
static_assert(!std::is_constructible_v<Value, int>);
static_assert(std::is_copy_constructible_v<Value>);
static_assert(std::is_copy_assignable_v<Value>);

struct CopyOnly {
  long long value;
  CopyOnly() = delete;
  explicit CopyOnly(long long x) : value(x) {}
  CopyOnly(const CopyOnly&) = default;
  CopyOnly& operator=(const CopyOnly&) = default;
  CopyOnly(CopyOnly&&) = delete;
  CopyOnly& operator=(CopyOnly&&) = delete;
  friend CopyOnly operator+(const CopyOnly& a, const CopyOnly& b) {
    return CopyOnly(a.value + b.value);
  }
};
static_assert(std::is_copy_constructible_v<CopyOnly>);
static_assert(std::is_copy_assignable_v<CopyOnly>);
static_assert(!std::is_move_constructible_v<CopyOnly>);
static_assert(!std::is_move_assignable_v<CopyOnly>);

std::uint64_t seed = 1, checks = 0, queries = 0, solves = 0;

std::string decimal(Wide value) {
  const bool negative = value < 0;
  auto magnitude = static_cast<unsigned __int128>(value);
  if (negative) magnitude = 0 - magnitude;
  std::string result;
  do {
    result.push_back(static_cast<char>('0' + magnitude % 10));
    magnitude /= 10;
  } while (magnitude != 0);
  if (negative) result.push_back('-');
  std::reverse(result.begin(), result.end());
  return result;
}

template <class T>
Wide scalar(const T& value) {
  if constexpr (std::is_same_v<T, Value>) return value.value;
  else return static_cast<Wide>(value);
}

template <class T>
T make_value(Wide value) {
  if constexpr (std::is_same_v<T, Value>) return Value::from(value);
  else return static_cast<T>(value);
}

template <class T>
struct Scenario {
  using Graph = blueberry::OfflineDynamicComponentSum<T>;
  std::string label;
  Graph graph;
  std::vector<Wide> initial, values, answers;
  std::vector<std::vector<int>> edges;
  std::vector<std::string> history;

  static Graph make_graph(const std::vector<Wide>& initial) {
    std::vector<T> input;
    for (Wide value : initial) input.push_back(make_value<T>(value));
    Graph result(input);
    // Construction must retain values independently of caller storage.
    for (T& value : input) value = make_value<T>(777);
    input.clear();
    input.shrink_to_fit();
    return result;
  }

  Scenario(std::string name, std::vector<Wide> input)
      : label(std::move(name)), graph(make_graph(input)), initial(input),
        values(std::move(input)), edges(values.size(), std::vector<int>(values.size())) {
    require(graph.size() == static_cast<int>(values.size()), "constructor size");
    verify(true);
  }

  void require(bool ok, const char* what) const {
    ++checks;
    if (ok) return;
    std::cerr << "seed=" << seed << " case=" << label << " n=" << values.size()
              << " check=" << what << "\ninitial:";
    for (Wide value : initial) std::cerr << ' ' << decimal(value);
    std::cerr << '\n';
    for (std::size_t i = 0; i < history.size(); ++i) {
      std::cerr << i << ": " << history[i] << '\n';
    }
    std::exit(1);
  }

  void add_edge(int u, int v) {
    history.push_back("add_edge " + std::to_string(u) + " " + std::to_string(v));
    graph.add_edge(u, v);
    ++edges[u][v];
    if (u != v) ++edges[v][u];
  }

  void remove_edge(int u, int v) {
    const bool expected = edges[u][v] != 0;
    history.push_back("remove_edge " + std::to_string(u) + " " + std::to_string(v) +
                      " expected=" + std::to_string(expected));
    const bool actual = graph.remove_edge(u, v);
    require(actual == expected, "remove acceptance");
    if (expected) {
      --edges[u][v];
      if (u != v) --edges[v][u];
    } else {
      verify();  // Rejection must not change any already recorded answer.
    }
  }

  void add_value(int v, Wide delta) {
    history.push_back("add_value " + std::to_string(v) + " " + decimal(delta));
    T input = make_value<T>(delta);
    graph.add_value(v, input);
    input = make_value<T>(777);  // The recorded delta must not retain this reference.
    values[v] += delta;
  }

  // Independent chronological model: count parallel edges and traverse the
  // present graph with BFS. No DSU, time intervals, or rollback is used.
  Wide component_sum(int start) const {
    std::vector<unsigned char> seen(values.size());
    std::vector<int> pending{start};
    seen[start] = true;
    Wide total = 0;
    for (std::size_t head = 0; head < pending.size(); ++head) {
      const int u = pending[head];
      total += values[u];
      for (int v = 0; v < static_cast<int>(values.size()); ++v) {
        if (edges[u][v] != 0 && !seen[v]) {
          seen[v] = true;
          pending.push_back(v);
        }
      }
    }
    return total;
  }

  void query(int v) {
    ++queries;
    const Wide answer = component_sum(v);
    history.push_back("query " + std::to_string(v) + " id=" + std::to_string(answers.size()) +
                      " expected=" + decimal(answer));
    const int id = graph.query(v);
    require(id == static_cast<int>(answers.size()), "stable consecutive query id");
    answers.push_back(answer);
  }

  void expect(int v, Wide wanted) {
    query(v);
    require(answers.back() == wanted, "hand-checked BFS answer");
  }

  void check_answers(const std::vector<T>& actual) const {
    require(actual.size() == answers.size(), "answer count");
    for (std::size_t i = 0; i < answers.size(); ++i) {
      if (scalar(actual[i]) != answers[i]) {
        std::cerr << "answer id=" << i << " expected=" << decimal(answers[i])
                  << " actual=" << decimal(scalar(actual[i])) << '\n';
        require(false, "component sum");
      }
      ++checks;
    }
  }

  void verify(bool repeat = false) {
    const Graph& view = graph;
    history.push_back("solve const");
    ++solves;
    auto result = view.solve();
    check_answers(result);
    require(view.size() == static_cast<int>(values.size()), "size after solve");
    if (repeat) {
      if (!result.empty()) result.front() = make_value<T>(777);
      history.push_back("solve const again after mutating returned copy");
      ++solves;
      check_answers(view.solve());
    }
  }
};

template <class T>
void fixed_cases(const std::string& type) {
  Scenario<T> empty(type + " empty", {});
  const typename Scenario<T>::Graph default_empty;
  empty.require(default_empty.size() == 0 && default_empty.solve().empty(), "default empty graph");

  Scenario<T> prefix(type + " collapsed mutations", {5, -2, 11});
  prefix.add_edge(0, 1);
  prefix.add_edge(1, 0);
  prefix.remove_edge(1, 0);
  prefix.remove_edge(0, 1);
  prefix.remove_edge(0, 1);
  prefix.add_edge(0, 0);
  prefix.add_edge(0, 0);
  prefix.remove_edge(0, 0);
  prefix.remove_edge(0, 0);
  prefix.remove_edge(0, 0);
  prefix.add_value(0, 7);
  prefix.add_value(0, -4);
  prefix.add_value(2, -6);
  prefix.add_edge(2, 1);
  prefix.remove_edge(1, 2);
  prefix.verify(true);  // N > 0 and many mutations, but still no queries.
  prefix.expect(0, 8);
  prefix.expect(1, -2);
  prefix.expect(2, 5);
  prefix.verify(true);
  prefix.add_edge(0, 1);
  prefix.add_value(1, 9);
  prefix.expect(0, 15);
  prefix.verify(true);
  prefix.add_value(0, -3);
  prefix.remove_edge(1, 0);
  prefix.verify(true);  // Trailing mutations must not alter earlier answers.
  prefix.expect(0, 5);
  prefix.expect(1, 7);
  prefix.add_edge(1, 0);
  prefix.remove_edge(0, 2);  // Rejected removal must preserve the active edge.
  prefix.expect(0, 12);
  prefix.expect(2, 5);
  prefix.verify(true);
  prefix.remove_edge(0, 1);
  prefix.add_edge(1, 0);  // The disconnected interval contains no query.
  prefix.expect(1, 12);
  prefix.verify(true);

  Scenario<T> cycle(type + " replacement and rollback", {1, 10, 100, 1000, 10000});
  cycle.add_edge(0, 1);
  cycle.add_edge(1, 2);
  cycle.expect(0, 111);
  cycle.add_edge(2, 0);
  cycle.remove_edge(1, 2);
  cycle.expect(2, 111);  // Removing a cycle edge must not split the component.
  cycle.add_edge(2, 3);
  cycle.add_value(1, 5);
  cycle.expect(3, 1116);
  cycle.verify(true);
  cycle.remove_edge(0, 2);
  cycle.expect(2, 1100);
  cycle.expect(0, 16);  // The update stays on vertex 1 after a split.
  cycle.add_edge(1, 3);
  cycle.expect(0, 1116);
  cycle.remove_edge(0, 1);
  cycle.expect(0, 1);
  cycle.expect(1, 1115);
  cycle.remove_edge(1, 3);
  cycle.expect(1, 15);
  cycle.expect(3, 1100);
  cycle.add_edge(3, 2);
  cycle.remove_edge(2, 3);
  cycle.expect(3, 1100);  // One parallel copy remains.
  cycle.remove_edge(3, 2);
  cycle.expect(2, 100);
  cycle.expect(3, 1000);
  cycle.expect(4, 10000);
  cycle.verify(true);

  // Histories, active multiplicities, vertex updates, and existing answer IDs
  // must be copied independently, including mutations following the last query.
  prefix.add_edge(1, 2);
  prefix.add_value(2, 4);
  Scenario<T> branch(prefix);
  branch.label += " copied branch";
  branch.add_value(0, -20);
  branch.remove_edge(1, 2);
  branch.expect(0, -8);
  branch.expect(2, 9);
  branch.verify(true);
  prefix.expect(0, 21);
  prefix.add_value(1, 30);
  prefix.expect(2, 51);
  prefix.verify(true);
  branch.verify(true);
  Scenario<T> assigned(type + " assignment target", {99});
  assigned = prefix;
  assigned.label += " assigned branch";
  assigned.remove_edge(0, 1);
  assigned.expect(0, 5);
  assigned.expect(2, 46);
  assigned.verify(true);
  prefix.verify(true);
}

template <class T>
void magnitude_case(const std::string& type, Wide large) {
  // Even the sum of absolute initial values and all deltas is representable;
  // arbitrary regrouping of signed additions is safe for every implementation.
  Scenario<T> s(type + " exact large sums", {large, -large / 2, 17});
  s.add_edge(0, 1);
  s.expect(0, large / 2);
  s.add_value(1, large / 4);
  s.add_edge(1, 2);
  s.expect(2, 3 * large / 4 + 17);
  s.verify(true);
  s.remove_edge(1, 0);
  s.add_value(0, -large / 4);
  s.expect(0, 3 * large / 4);
  s.expect(1, -large / 4 + 17);
  s.add_edge(0, 2);
  s.expect(1, large / 2 + 17);
  s.verify(true);
}

template <class T>
void random_cases(const std::string& type, std::mt19937_64& rng) {
  for (int n : {1, 2, 3, 7, 15}) {
    for (int trial = 0; trial < 2; ++trial) {
      std::vector<Wide> initial(n);
      for (Wide& value : initial) value = static_cast<int>(rng() % 31) - 15;
      Scenario<T> s(type + " random trial=" + std::to_string(trial), initial);
      for (int step = 0; step < 320; ++step) {
        const int u = static_cast<int>(rng() % static_cast<unsigned>(n));
        const int v = step % 7 == 0 ? u : static_cast<int>(rng() % static_cast<unsigned>(n));
        const int action = static_cast<int>(rng() % (step < 32 ? 6 : 10));
        if (action < 2) s.add_edge(u, v);
        else if (action < 4) s.remove_edge(v, u);
        else if (action < 6) s.add_value(u, static_cast<int>(rng() % 31) - 15);
        else if (action < 9) s.query(u);
        else s.verify();
        if (step % 31 == 0) s.verify(true);
      }
      for (int v = 0; v < n; ++v) s.query(v);
      s.verify(true);
    }
  }
}

void copy_only_case() {
  // Separate from Scenario: its test-input helpers deliberately assign
  // temporaries, while this fixture permits only the promised copy operations.
  const CopyOnly a(2), b(5), c(11), delta(3), decrease(-4), sentinel(99);
  std::vector<CopyOnly> input{a, b, c};
  blueberry::OfflineDynamicComponentSum<CopyOnly> graph(input);
  input[0] = sentinel;
  const auto& view = graph;
  std::vector<std::string> history;
  const auto require = [&](bool ok, const char* what) {
    ++checks;
    if (ok) return;
    std::cerr << "seed=" << seed << " case=deleted moves check=" << what
              << "\ninitial: 2 5 11\n";
    for (const auto& call : history) std::cerr << call << '\n';
    std::exit(1);
  };
  const auto query = [&](int v, int expected_id) {
    history.push_back("query " + std::to_string(v) + " id=" + std::to_string(expected_id));
    ++queries;
    require(graph.query(v) == expected_id, "query id");
  };
  const std::vector<long long> expected{7, 10, 8, 2, 19, 17};
  const auto verify = [&](std::size_t count) {
    history.push_back("solve const");
    ++solves;
    const auto result = view.solve();
    require(result.size() == count, "answer count");
    for (std::size_t i = 0; i < count; ++i) {
      require(result[i].value == expected[i], "component sum");
    }
  };
  history.push_back("add_edge 0 1");
  graph.add_edge(0, 1);
  query(0, 0);
  history.push_back("add_value 1 3");
  graph.add_value(1, delta);
  query(0, 1);
  verify(2);
  history.push_back("remove_edge 0 1");
  require(graph.remove_edge(0, 1), "remove existing edge");
  query(1, 2);
  query(0, 3);
  verify(4);
  history.push_back("add_value 0 -4");
  graph.add_value(0, decrease);
  history.push_back("add_edge 1 2");
  graph.add_edge(1, 2);
  query(2, 4);
  history.push_back("add_edge 0 2");
  graph.add_edge(0, 2);
  query(0, 5);
  verify(6);
  verify(6);
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::stoull(argv[1]) : 1;
  std::cerr << "seed=" << seed << '\n';
  fixed_cases<long long>("long long");
  fixed_cases<Wide>("int128");
  fixed_cases<Value>("plus-only Value");
  magnitude_case<long long>("long long", Wide{1} << 58);
  magnitude_case<Wide>("int128", Wide{1} << 100);
  magnitude_case<Value>("plus-only Value", Wide{1} << 100);
  copy_only_case();
  std::mt19937_64 rng(seed);
  random_cases<long long>("long long", rng);
  random_cases<Wide>("int128", rng);
  random_cases<Value>("plus-only Value", rng);
  std::cout << "offline dynamic component sum: " << queries << " BFS answers, "
            << solves << " solves, " << checks << " checks passed\n";
}
