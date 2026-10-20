#include "blueberry/utility/contest-fast-io.hpp"
#include <cstdlib>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace {
unsigned long long test_seed;
void check(bool valid, const std::string& input, const std::string& expected,
           const std::string& actual) {
  if (!valid) {
    std::cerr << "seed=" << test_seed << " input=" << input << " expected=" << expected
              << " actual=" << actual << '\n';
    std::exit(1);
  }
}
void require(bool valid, const char* message) { check(valid, message, "success", "failure"); }
std::FILE* input_file(const std::string& text) {
  auto* file = std::tmpfile();
  require(file != nullptr, "tmpfile");
  require(std::fwrite(text.data(), 1, text.size(), file) == text.size(), "prepare input");
  std::rewind(file);
  return file;
}
std::string contents(std::FILE* file) {
  std::rewind(file);
  std::string result;
  char buffer[4096];
  for (std::size_t size; (size = std::fread(buffer, 1, sizeof buffer, file));)
    result.append(buffer, size);
  return result;
}
template<class T> std::string fixed_text(T value, int precision) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}
void nested_roundtrip(std::mt19937_64& rng) {
  using Row = std::pair<std::array<long long, 2>, std::deque<int>>;
  for (int round = 0; round < 200; ++round) {
    std::vector<Row> expected(rng() % 8);
    std::string text, expected_output;
    for (std::size_t i = 0; i < expected.size(); ++i) {
      auto& row = expected[i];
      for (auto& value : row.first) value = static_cast<long long>(rng());
      row.second.resize(rng() % 8);
      for (auto& value : row.second) value = static_cast<int>(rng() % 2001) - 1000;
      if (i) expected_output += ' ';
      expected_output += std::to_string(row.first[0]) + ' ' + std::to_string(row.first[1]) + ' ';
      for (std::size_t j = 0; j < row.second.size(); ++j) {
        if (j) expected_output += ' ';
        expected_output += std::to_string(row.second[j]);
      }
      text += std::to_string(row.first[0]) + '\t' + std::to_string(row.first[1]) + '\n';
      for (int value : row.second) text += std::to_string(value) + "\r\n";
    }
    auto actual = expected;
    for (auto& row : actual) { row.first.fill(0); std::fill(row.second.begin(), row.second.end(), 0); }
    auto* file = input_file(text);
    {
      blueberry::FastInput in(file);
      require(in.read(actual), "nested read");
      check(actual == expected, text, expected_output, "nested values differ");
      int unchanged = 17;
      require(!in.read(unchanged) && unchanged == 17, "nested EOF");
    }
    std::fclose(file);
    file = std::tmpfile(); require(file != nullptr, "nested output file");
    { blueberry::FastOutput out(file); require(out.write(actual), "nested write"); }
    const auto written = contents(file);
    check(written == expected_output, text, expected_output, written);
    std::fclose(file);
  }
}
void boundary_cases() {
  // Failure preserves the current scalar and leaves later elements untouched.
  auto* file = input_file("10 2147483648 20 1 2 0 1 1");
  {
    blueberry::FastInput in(file);
    std::array<int, 3> values{31, 32, 33};
    require(!in.read(values) && values == std::array<int, 3>{10, 32, 33}, "array overflow partial update");
    std::vector<int> empty;
    require(in.read(empty), "empty vector consumes no input");
    int next = 0; require(in.read(next) && next == 20, "resume after array overflow");
    std::vector<bool> bits(3, false);
    require(!in.read(bits) && bits[0] && !bits[1] && !bits[2], "bool vector partial failure");
    require(in.read(bits) && !bits[0] && bits[1] && bits[2], "bool vector resume");
  }
  std::fclose(file);
  file = input_file("1 2");
  {
    blueberry::FastInput in(file);
    std::pair<int, std::array<int, 2>> values{9, {8, 7}};
    require(!in.read(values) && values.first == 1 && values.second == std::array<int, 2>{2, 7}, "nested EOF partial update");
    std::array<int, 0> empty{}; require(in.read(empty), "empty array at EOF");
  }
  std::fclose(file);
  file = std::tmpfile(); require(file != nullptr, "empty output file");
  {
    blueberry::FastOutput out(file);
    const std::vector<std::vector<int>> empty_shapes{{}, {1}, {}, {2, 3}, {}};
    require(out.writeln(empty_shapes, std::deque<int>{}, std::array<int, 0>{}), "empty separators");
    require(out.writeln(std::vector<bool>{true, false, true}), "bool vector output");
  }
  const auto written = contents(file);
  check(written == " 1  2 3   \n1 0 1\n", "empty nested shapes", " 1  2 3   \\n1 0 1\\n", written);
  std::fclose(file);
}
void fixed_precision() {
  auto* file = std::tmpfile(); require(file != nullptr, "fixed output file");
  std::string expected;
  {
    blueberry::FastOutput out(file);
    for (int precision : {0, 1, 2, 30, 512}) {
      for (double value : {0.0, -0.0, 0.125, -1.375, 1e100, std::numeric_limits<double>::min()}) {
        require(out.write(value, precision) && out.write('\n'), "fixed double write");
        expected += fixed_text(value, precision) + '\n';
      }
      for (long double value : {0.125L, -0.0L, std::numeric_limits<long double>::max()}) {
        require(out.write(value, precision) && out.write('\n'), "fixed long double write");
        expected += fixed_text(value, precision) + '\n';
      }
    }
  }
  const auto written = contents(file);
  check(written == expected, "fixed formatting vs iostream", expected, written);
  std::fclose(file);
}
int descriptor(std::FILE* file) {
#if defined(_WIN32)
  return _fileno(file);
#else
  return fileno(file);
#endif
}
int duplicate(int fd) {
#if defined(_WIN32)
  return _dup(fd);
#else
  return dup(fd);
#endif
}
void redirect(int from, int to) {
#if defined(_WIN32)
  require(_dup2(from, to) == 0, "redirect descriptor");
#else
  require(dup2(from, to) >= 0, "redirect descriptor");
#endif
}
void close_descriptor(int fd) {
#if defined(_WIN32)
  require(_close(fd) == 0, "close descriptor");
#else
  require(close(fd) == 0, "close descriptor");
#endif
}
void facade() {
  // Redirect before the first facade access; keep stdin/stdout FILE objects alive.
  auto* source = input_file("17 ok 1 2 3 4 0 1 -170141183460469231731687303715884105728 2147483648 99");
  auto* destination = std::tmpfile(); require(destination != nullptr, "facade output file");
  require(std::fflush(stdout) == 0, "flush original stdout");
  const int saved_in = duplicate(descriptor(stdin)), saved_out = duplicate(descriptor(stdout));
  require(saved_in >= 0 && saved_out >= 0, "save descriptors");
  redirect(descriptor(source), descriptor(stdin));
  redirect(descriptor(destination), descriptor(stdout));
  require(scan(), "empty scan");
  LL(n); STR(s);
  std::vector<std::pair<int, int>> pairs(2);
  std::vector<bool> bits(2);
  __int128 minimum{};
  require(scan(pairs, bits, minimum), "facade recursive scan");
  require(n == 17 && s == "ok" && pairs[1] == std::pair{3, 4} && !bits[0] && bits[1]
      && minimum == std::numeric_limits<__int128>::min(), "facade scanned values");
  int overflow = 7, last = 8;
  require(!scan(overflow, last) && overflow == 7 && last == 8, "facade overflow short circuit");
  require(scan(last) && last == 99 && !scan(last) && last == 99, "facade EOF no newline");
  require(print(), "empty print");
  require(O(n, s, pairs, bits, minimum), "O alias");
  require(YesNo(true) && YesNo(false) && YESNO(true) && YESNO(false), "yes/no helpers");
  init_io(2);
  require(print(std::vector<std::array<double, 2>>{{1.25, -0.0}, {2.5, -1.375}}), "recursive fixed print");
  init_io(0); require(print(1.25, -0.0), "change precision");
  init_io(); require(print(0.125L), "default precision");
  require(flush(), "facade flush");
  redirect(saved_in, descriptor(stdin)); redirect(saved_out, descriptor(stdout));
  close_descriptor(saved_in); close_descriptor(saved_out);
  const std::string expected = "\n17 ok 1 2 3 4 0 1 -170141183460469231731687303715884105728\n"
      "Yes\nNo\nYES\nNO\n1.25 -0.00 2.50 -1.38\n1 -0\n" + fixed_text(0.125L, 30) + '\n';
  const auto written = contents(destination);
  check(written == expected, "global facade", expected, written);
  std::fclose(source); std::fclose(destination);
}
}  // namespace
int main(int argc, char** argv) {
  test_seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(test_seed);
  nested_roundtrip(rng);
  boundary_cases();
  fixed_precision();
  facade();
}
