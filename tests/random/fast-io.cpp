#include "blueberry/utility/fast-io.hpp"
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>

unsigned long long seed;
void check(bool condition, const char* message) {
  if (!condition) { std::cerr << "seed=" << seed << " fast-io: " << message << '\n'; std::exit(1); }
}
std::FILE* input(const std::string& text) {
  auto* file = std::tmpfile(); check(file != nullptr, "tmpfile");
  check(std::fwrite(text.data(), 1, text.size(), file) == text.size(), "prepare input");
  std::rewind(file); return file;
}
template<class T> void integers(std::mt19937_64& rng) {
  std::vector<T> values{0, 1, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
  if constexpr (std::is_signed_v<T>) values.push_back(-1);
  for (int i = 0; i < 1000; ++i) values.push_back(static_cast<T>(rng()));
  std::string text(65531, ' '), output_expected;
  for (T x : values) {
    char buffer[32];
    auto result = std::to_chars(buffer, buffer + sizeof buffer, x);
    check(result.ec == std::errc{}, "oracle conversion");
    std::string token(buffer, result.ptr);
    text += token + "\t\r\n\v\f "; output_expected += token + '\n';
  }
  // The final number ends exactly at EOF, without a delimiter.
  text += "17"; values.push_back(17); output_expected += "17\n";
  auto* file = input(text);
  { blueberry::FastInput in(file);
    for (T expected : values) { T actual{}; check(in.read(actual) && actual == expected, "integer parse"); }
    T unchanged = 3; check(!in.read(unchanged) && unchanged == 3, "integer EOF");
  }
  std::fclose(file);
  file = std::tmpfile(); check(file != nullptr, "output tmpfile");
  { blueberry::FastOutput out(file); for (T x : values) check(out.writeln(x), "integer write"); check(out.flush(), "integer flush"); }
  std::rewind(file);
  std::string actual(output_expected.size() + 1, '\0');
  const auto count = std::fread(actual.data(), 1, actual.size(), file); actual.resize(count);
  check(actual == output_expected, "integer output oracle"); std::fclose(file);
}
template<class T> void floating() {
  auto* file = std::tmpfile(); check(file != nullptr, "floating tmpfile");
  const std::vector<T> values{T(0), T(-0.0), T(1.25), std::numeric_limits<T>::min(),
      std::numeric_limits<T>::max(), std::numeric_limits<T>::denorm_min()};
  { blueberry::FastOutput out(file);
    for (T value : values) check(out.writeln(value), "floating write");
  }
  std::rewind(file);
  { blueberry::FastInput in(file);
    for (T expected : values) { T actual{};
      check(in.read(actual) && actual == expected && std::signbit(actual) == std::signbit(expected), "floating roundtrip");
    }
  } std::fclose(file);
}
int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  static_assert(!std::is_copy_constructible_v<blueberry::FastInput<>>);
  static_assert(!std::is_move_constructible_v<blueberry::FastOutput>);
  integers<signed char>(rng); integers<unsigned char>(rng);
  integers<short>(rng); integers<unsigned short>(rng);
  integers<int>(rng); integers<unsigned>(rng);
  integers<long>(rng); integers<unsigned long>(rng);
  integers<long long>(rng); integers<unsigned long long>(rng);
  floating<float>(); floating<double>(); floating<long double>();
  auto* bool_output = std::tmpfile(); check(bool_output != nullptr, "bool output file");
  { blueberry::FastOutput out(bool_output);
    const char* word = "word";
    check(out.writeln(true, false, "literal", word), "bool/literal output");
  }
  std::rewind(bool_output); char bool_text[32]{};
  check(std::fread(bool_text, 1, 32, bool_output) == 17 && std::string(bool_text) == "1 0 literal word\n", "bool/literal output bytes");
  std::fclose(bool_output);
  auto* bytes = std::tmpfile(); check(bytes != nullptr, "byte output file");
  { blueberry::FastOutput out(bytes);
    check(out.write(std::string_view("a\0b", 3)) && out.write(std::string_view{}), "byte output");
  }
  std::rewind(bytes); char byte_text[4]{};
  check(std::fread(byte_text, 1, 4, bytes) == 3 && std::string(byte_text, 3) == std::string("a\0b", 3), "embedded NUL");
  std::fclose(bytes);
  auto* boolean_file = input("0 +1 2 -1 true");
  { blueberry::FastInput in(boolean_file); bool b = true;
    check(in.read(b) && !b, "bool zero"); check(in.read(b) && b, "bool one");
    for (int i = 0; i < 3; ++i) check(!in.read(b) && b, "bool invalid");
  } std::fclose(boolean_file);
  auto* file = input("+2147483647 -2147483648 2147483648 -2147483649 + - --1 1x 2.0 99\n"
                    "-1 4294967296 +4294967295 7\n" + std::string(150000, '9') + " -123");
  { blueberry::FastInput in(file); int x = 77;
    check(in.read(x) && x == 2147483647, "positive sign");
    check(in.read(x) && x == std::numeric_limits<int>::min(), "int min");
    for (int i = 0; i < 7; ++i) { x = 77; check(!in.read(x) && x == 77, "invalid/overflow unchanged"); }
    check(in.read(x) && x == 99, "resume after invalid");
    unsigned u = 3;
    check(!in.read(u) && u == 3, "unsigned negative");
    check(!in.read(u) && u == 3, "unsigned overflow");
    check(in.read(u) && u == 4294967295U, "unsigned max");
    int y = 45, z = 46;
    check(!in.read(x,y,z) && x == 7 && y == 45 && z == 46, "variadic short circuit");
    check(in.read(x) && x == -123, "resume long overflow");
  } std::fclose(file);
  const std::string long_token(150000, 'a');
  file = input(" \r\nX " + long_token + " +1.25 -0 inf nan 1e99999 +-1 + ++1");
  { blueberry::FastInput in(file); char ch = '?'; std::string s;
    check(in.read(ch, s) && ch == 'X' && s == long_token, "char/string boundary");
    double x = 9;
    check(in.read(x) && x == 1.25, "float plus");
    check(in.read(x) && x == 0 && std::signbit(x), "float negative zero");
    check(in.read(x) && std::isinf(x), "infinity");
    check(in.read(x) && std::isnan(x), "nan");
    for (int i = 0; i < 4; ++i) { x = 9; check(!in.read(x) && x == 9, "float invalid/range"); }
    s = "unchanged"; check(!in.read(s) && s == "unchanged", "string EOF");
  } std::fclose(file);
  file = std::tmpfile(); check(file != nullptr, "float output file");
  { blueberry::FastOutput out(file);
    check(out.writeln(), "empty newline");
    check(out.writeln('A', "hello", std::string("world"), 1.25, -0.0), "mixed output");
    check(out.write(std::string_view(long_token)), "large output");
    check(out.flush(), "mixed flush");
  }
  std::rewind(file);
  { blueberry::FastInput in(file); char ch; std::string s, t; double a, b;
    check(in.read(ch,s,t,a,b) && ch=='A' && s=="hello" && t=="world" && a==1.25 && b==0 && std::signbit(b), "mixed roundtrip");
    check(in.read(s) && s==long_token, "long output roundtrip");
  } std::fclose(file);
#if defined(__SIZEOF_INT128__)
  const std::string minimum128 = "-170141183460469231731687303715884105728";
  const std::string maximum128 = "170141183460469231731687303715884105727";
  const std::string unsigned128 = "340282366920938463463374607431768211455";
  file = input(minimum128 + " " + maximum128 + " " + unsigned128 + " " + unsigned128 + "0");
  __int128_t a{}, b{}; __uint128_t c{};
  { blueberry::FastInput in(file);
    check(in.read(a,b,c) && a==std::numeric_limits<__int128_t>::min() && b==std::numeric_limits<__int128_t>::max() && c==~__uint128_t{0}, "128 extremes");
    check(!in.read(c) && c==~__uint128_t{0}, "128 overflow");
  } std::fclose(file);
  file = std::tmpfile(); check(file != nullptr, "128 output");
  { blueberry::FastOutput out(file); check(out.writeln(a,b,c), "128 write"); }
  std::rewind(file);
  const auto expected = minimum128 + " " + maximum128 + " " + unsigned128 + "\n";
  std::string actual(expected.size()+1, '\0');
  actual.resize(std::fread(actual.data(),1,actual.size(),file));
  check(actual == expected, "128 exact output"); std::fclose(file);
#endif
#if defined(__linux__)
  file = std::fopen(".", "rb"); check(file != nullptr, "directory stream");
  { blueberry::FastInput in(file); int value = 7;
    check(!in.read(value) && value == 7 && std::ferror(file), "input FILE error");
  } std::fclose(file);
  file = std::fopen("/dev/full", "wb"); check(file != nullptr, "dev/full");
  { blueberry::FastOutput out(file); check(out.write("data"), "buffered before error");
    check(!out.flush() && !out.write('x') && !out.flush(), "sticky output error");
  } std::fclose(file);
#endif
}
