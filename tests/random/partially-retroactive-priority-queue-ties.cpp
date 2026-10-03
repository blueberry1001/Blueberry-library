#include "blueberry/data-structure/partially-retroactive-priority-queue.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Distinct payloads are equivalent under <. The queue must break ties by the
// insertion's time slot, not by its payload or the order of retroactive edits.
struct Value {
  int priority, payload;
  Value() = delete;
  Value(int p, int id) : priority(p), payload(id) {}
  bool operator<(const Value& other) const { return priority < other.priority; }
  bool operator==(const Value&) const = delete;
};
static_assert(std::is_copy_constructible_v<Value>);
static_assert(std::is_copy_assignable_v<Value>);
static_assert(!std::is_default_constructible_v<Value>);
using Queue = blueberry::PartiallyRetroactivePriorityQueue<Value>;

enum class Kind { empty, insert, pop };
enum class Action { insert, pop, erase };
struct Operation {
  Kind kind = Kind::empty;
  int priority = 0, payload = 0;
};
struct Edit {
  Action action;
  int time, priority, payload;
  bool expected = false, actual = false;
};

std::uint64_t seed = 1, checks = 0, edits = 0;

// Ordinary chronological replay; no bridge information or library comparator
// is used. A delete-min scans all currently present insertion identities.
std::optional<std::vector<int>> replay(const std::vector<Operation>& timeline) {
  std::vector<int> alive;
  const auto before = [&](int a, int b) {
    return timeline[a].priority < timeline[b].priority ||
           (timeline[a].priority == timeline[b].priority && a < b);
  };
  for (int t = 0; t < static_cast<int>(timeline.size()); ++t) {
    if (timeline[t].kind == Kind::insert) alive.push_back(t);
    if (timeline[t].kind == Kind::pop) {
      if (alive.empty()) return std::nullopt;
      auto best = alive.begin();
      for (auto it = alive.begin() + 1; it != alive.end(); ++it) {
        if (before(*it, *best)) best = it;
      }
      alive.erase(best);
    }
  }
  std::sort(alive.begin(), alive.end(), before);
  return alive;
}

struct Scenario {
  std::string label;
  int edit_slots;
  Queue queue;
  std::vector<Operation> timeline;
  std::vector<Edit> history;

  Scenario(std::string name, int n)
      : label(std::move(name)), edit_slots(n), queue(2 * n + 1), timeline(n) {
    verify();
  }

  void require(bool ok, const char* what) const {
    ++checks;
    if (ok) return;
    std::cerr << "seed=" << seed << " case=" << label << " slots=" << edit_slots
              << " check=" << what << '\n';
    for (std::size_t i = 0; i < history.size(); ++i) {
      const auto& e = history[i];
      const char* action = e.action == Action::insert ? "insert"
                           : e.action == Action::pop ? "pop" : "erase";
      std::cerr << i << ": " << action << " t=" << e.time << " priority=" << e.priority
                << " payload=" << e.payload << " expected=" << e.expected
                << " actual=" << e.actual << '\n';
    }
    for (int t = 0; t < edit_slots; ++t) {
      const auto& op = timeline[t];
      std::cerr << "slot " << t << " kind=" << static_cast<int>(op.kind)
                << " priority=" << op.priority << " payload=" << op.payload << '\n';
    }
    std::exit(1);
  }

  void expect_value(const std::optional<Value>& actual, int t, const char* what) const {
    require(actual.has_value(), what);
    if (actual->priority != timeline[t].priority || actual->payload != timeline[t].payload) {
      std::cerr << "expected priority=" << timeline[t].priority << " payload=" << timeline[t].payload
                << " actual priority=" << actual->priority << " payload=" << actual->payload << '\n';
      require(false, what);
    }
  }

  void verify() const {
    const auto expected = replay(timeline);
    require(expected.has_value(), "oracle history remains valid");
    require(queue.time_slots() == 2 * edit_slots + 1, "time_slots");
    require(queue.size() == static_cast<int>(expected->size()), "size");
    require(queue.empty() == expected->empty(), "empty");
    for (int t = 0; t < queue.time_slots(); ++t) {
      require(queue.has_op(t) == (t < edit_slots && timeline[t].kind != Kind::empty), "has_op");
    }
    if (expected->empty()) require(!queue.min(), "empty min");
    else expect_value(queue.min(), expected->front(), "present min identity");

    // Reveal every survivor identity, including equal-priority non-minima, by
    // appending delete-mins to a copy in reserved slots after the whole history.
    Queue drained(queue);
    int t = edit_slots;
    for (int id : *expected) {
      expect_value(drained.min(), id, "survivor identity order");
      require(drained.pop_op(t++), "append delete-min to copied history");
    }
    require(drained.empty() && drained.size() == 0 && !drained.min(), "drained copy");
    require(!drained.pop_op(t), "reject empty delete-min on copied history");
    require(!drained.has_op(t) && drained.empty() && !drained.min(), "rejected delete-min unchanged");
    require(queue.size() == static_cast<int>(expected->size()), "original unchanged by drain");
    if (expected->empty()) require(!queue.min(), "original remains empty");
    else expect_value(queue.min(), expected->front(), "original min unchanged by drain");
  }

  void edit(Action action, int t, int priority = 0, int payload = 0) {
    ++edits;
    auto next = timeline;
    bool expected = action == Action::erase ? next[t].kind != Kind::empty
                                           : next[t].kind == Kind::empty;
    if (expected) {
      next[t] = action == Action::insert ? Operation{Kind::insert, priority, payload}
                : action == Action::pop ? Operation{Kind::pop, 0, 0} : Operation{};
      expected = replay(next).has_value();
    }
    history.push_back({action, t, priority, payload, expected, false});
    const bool actual = action == Action::insert ? queue.insert_op(t, Value(priority, payload))
                        : action == Action::pop ? queue.pop_op(t) : queue.erase_op(t);
    history.back().actual = actual;
    require(actual == expected, "edit acceptance");
    if (expected) timeline = std::move(next);
    verify();  // Rejected edits must retain every observable insertion identity.
  }

  void expect_payload(int payload) const {
    const auto value = queue.min();
    require(value && value->payload == payload, "scripted minimum payload");
  }
};

int main(int argc, char** argv) {
  seed = argc > 1 ? std::stoull(argv[1]) : 1;
  std::cerr << "seed=" << seed << '\n';
  Scenario fixed("retroactive ties", 8);
  fixed.edit(Action::insert, 5, 7, 500);
  fixed.edit(Action::insert, 3, 7, 700);  // Payload order disagrees with time order.
  fixed.expect_payload(700);
  fixed.edit(Action::pop, 6);
  fixed.expect_payload(500);
  fixed.edit(Action::insert, 1, 7, 900);  // Earlier equal key revives payload 700.
  fixed.expect_payload(700);
  fixed.edit(Action::pop, 4);             // Retroactive delete consumes payload 900.
  fixed.expect_payload(500);
  fixed.edit(Action::erase, 1);           // Remove a consumed equal-priority insert.
  fixed.require(fixed.queue.empty(), "scripted empty present");
  fixed.edit(Action::erase, 3);           // Rejected: the pop at 4 would be invalid.
  fixed.edit(Action::erase, 4);           // Undo the earlier delete-min.
  fixed.expect_payload(500);
  fixed.edit(Action::erase, 6);           // Undo the later delete-min.
  fixed.expect_payload(700);
  fixed.edit(Action::erase, 3);           // Remove a surviving insert.
  fixed.expect_payload(500);
  fixed.edit(Action::insert, 5, -9, 999);  // Occupied slot: preserve stored payload.
  fixed.edit(Action::pop, 5);             // Occupied slot.
  fixed.edit(Action::erase, 2);           // Empty slot.
  fixed.edit(Action::pop, 0);             // Invalid prefix despite a nonempty present.
  fixed.edit(Action::insert, 2, -4, 200);
  fixed.edit(Action::pop, 6);             // Priority still takes precedence over time.
  fixed.expect_payload(500);

  std::mt19937_64 rng(seed);
  int payload = 1000;
  for (int n : {1, 2, 3, 7, 16, 31}) {
    for (int mode = 0; mode < 2; ++mode) {
      Scenario random(mode == 0 ? "all equal" : "three priorities", n);
      for (int step = 0; step < 300; ++step) {
        const auto action = static_cast<Action>(rng() % 3);
        const int t = static_cast<int>(rng() % static_cast<unsigned>(n));
        const int priority = mode == 0 ? 0 : static_cast<int>(rng() % 3) - 1;
        random.edit(action, t, priority, payload++);
      }
    }
  }
  std::cout << "retroactive priority queue ties: " << edits << " edits, " << checks << " checks passed\n";
}
