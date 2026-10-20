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

// Independent one-digit conversion also supports the GNU 128-bit integer types.
template<class T> std::string decimal(T value) {
  using U = std::make_unsigned_t<T>;
  const bool negative = std::is_signed_v<T> && value < 0;
  U number = negative ? U{0} - static_cast<U>(value) : static_cast<U>(value);
  std::string result;
  do { result.push_back(static_cast<char>('0' + number % 10)); number /= 10; } while (number);
  if (negative) result.push_back('-');
  std::reverse(result.begin(), result.end());
  return result;
}
std::string decimal_successor(std::string value) {
  for (std::size_t i = value.size(); i-- > 0;) {
    if (value[i] != '9') { ++value[i]; return value; }
    value[i] = '0';
  }
  return '1' + value;
}
#if defined(__SIZEOF_INT128__)
template<class T> void wide_integers(std::mt19937_64& rng) {
  std::vector<T> values{0, 1, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
  for (int i = 0; i < 1000; ++i)
    values.push_back(static_cast<T>((static_cast<__uint128_t>(rng()) << 64) | rng()));
  std::string expected;
  for (T value : values) expected += decimal(value) + '\n';
  auto* file = input(expected.substr(0, expected.size() - 1));
  {
    blueberry::FastInput in(file);
    for (T value : values) {
      T actual = 7;
      if (!in.read(actual) || actual != value) {
        std::cerr << "seed=" << seed << " input=" << decimal(value)
                  << " expected=" << decimal(value) << " actual=" << decimal(actual) << '\n';
        std::exit(1);
      }
    }
  }
  std::fclose(file);
  file = std::tmpfile(); check(file != nullptr, "random 128-bit output tmpfile");
  { blueberry::FastOutput out(file); for (T value : values) check(out.writeln(value), "random 128-bit write"); }
  std::rewind(file);
  std::string actual(expected.size() + 1, '\0');
  actual.resize(std::fread(actual.data(), 1, actual.size(), file));
  check(actual == expected, "random 128-bit output oracle");
  std::fclose(file);
}
#endif
template<class T> void integer_boundaries(bool all_lengths) {
  constexpr std::size_t capacity = 1 << 16;
  struct Case { std::string token; T expected; std::size_t split; };
  std::vector<Case> cases;
  auto* file = std::tmpfile(); check(file != nullptr, "boundary input tmpfile");
  const std::string spaces(capacity, ' ');
  std::size_t position = 0;
  auto append = [&](const std::string& token, T expected) {
    // Put every digit, sign, and the terminating delimiter at a refill boundary.
    for (std::size_t split = 0; split <= token.size(); ++split) {
      const auto padding = (capacity - (position + split) % capacity) % capacity;
      check(std::fwrite(spaces.data(), 1, padding, file) == padding, "boundary input padding");
      check(std::fwrite(token.data(), 1, token.size(), file) == token.size(), "boundary input token");
      check(std::fputc('\n', file) != EOF, "boundary input delimiter");
      position += padding + token.size() + 1;
      cases.push_back({token, expected, split});
    }
  };
  if (all_lengths) {
    for (T power = 1;;) {
      append(decimal(power), power);
      if constexpr (std::is_signed_v<T>) append(decimal(T(-power)), T(-power));
      if (power > std::numeric_limits<T>::max() / 10) break;
      power *= 10;
    }
  }
  append(decimal(std::numeric_limits<T>::max()), std::numeric_limits<T>::max());
  append('+' + decimal(std::numeric_limits<T>::max()), std::numeric_limits<T>::max());
  append(decimal(std::numeric_limits<T>::min()), std::numeric_limits<T>::min());
  std::rewind(file);
  {
    blueberry::FastInput in(file);
    for (const auto& test : cases) {
      T actual = 7;
      if (!in.read(actual) || actual != test.expected) {
        std::cerr << "seed=" << seed << " token=" << test.token << " split=" << test.split
                  << " expected=" << decimal(test.expected) << " actual=" << decimal(actual) << '\n';
        std::exit(1);
      }
    }
    T unchanged = 7; check(!in.read(unchanged) && unchanged == 7, "boundary input EOF");
  }
  std::fclose(file);

  // Exercise direct integer writes with every possible insufficient tail length.
  file = std::tmpfile(); check(file != nullptr, "boundary output tmpfile");
  struct Output { std::size_t padding; std::string token; };
  std::vector<Output> output_cases;
  const std::string fill(capacity, 'x');
  position = 0;
  {
    blueberry::FastOutput out(file);
    auto emit = [&](T value) {
      const auto token = decimal(value);
      for (std::size_t remaining = 0; remaining <= token.size() + 1; ++remaining) {
        const auto used = (capacity - remaining) % capacity;
        const auto padding = (used + capacity - position % capacity) % capacity;
        check(out.write(std::string_view(fill.data(), padding)) && out.write(value) && out.write('\n'), "boundary integer output");
        position += padding + token.size() + 1;
        output_cases.push_back({padding, token});
      }
    };
    if (all_lengths) {
      for (T power = 1;;) {
        emit(power); emit(T(power - 1));
        if constexpr (std::is_signed_v<T>) emit(T(-power));
        if (power > std::numeric_limits<T>::max() / 10) break;
        power *= 10;
      }
    }
    emit(std::numeric_limits<T>::min()); emit(std::numeric_limits<T>::max());
  }
  std::rewind(file);
  for (const auto& test : output_cases) {
    char buffer[4096];
    for (auto remaining = test.padding; remaining;) {
      const auto count = std::min(remaining, sizeof buffer);
      check(std::fread(buffer, 1, count, file) == count, "boundary output padding size");
      check(std::all_of(buffer, buffer + count, [](char c) { return c == 'x'; }), "boundary output padding bytes");
      remaining -= count;
    }
    std::string actual(test.token.size(), '\0');
    check(std::fread(actual.data(), 1, actual.size(), file) == actual.size(), "boundary output token size");
    if (actual != test.token) {
      std::cerr << "seed=" << seed << " padding=" << test.padding
                << " expected=" << test.token << " actual=" << actual << '\n';
      std::exit(1);
    }
    check(std::fgetc(file) == '\n', "boundary output newline");
  }
  check(std::fgetc(file) == EOF, "boundary output no extra bytes");
  std::fclose(file);
}
template<class T, bool Interactive = false> void integer_token_recovery() {
  const auto maximum = decimal(std::numeric_limits<T>::max());
  const auto minimum = decimal(std::numeric_limits<T>::min());
  std::vector<std::string> invalid{"+", "-", "+-1", "-+1", "++1", "--1", "123x", "0.0", "0x10",
      std::string("12\0tail", 7), std::string("12\xff", 3), decimal_successor(maximum),
      std::string(150000, '0') + decimal_successor(maximum), std::string(150000, '9')};
  if constexpr (std::is_signed_v<T>) invalid.push_back('-' + decimal_successor(minimum.substr(1)));
  else { invalid.push_back("-0"); invalid.push_back("-1"); }
  // Reject non-digits in every byte of several complete eight-digit groups.
  // The unchanged 24-digit magnitude fits in 128 bits, so overflow alone cannot
  // hide an incorrect grouped digit check in the 128-bit instantiations.
  for (unsigned char byte : std::array<unsigned char, 8>{'/', ':', ';', '?', '@', 0, 0x80, 0xff}) {
    for (std::size_t offset = 0; offset < 24; ++offset) {
      std::string token(24, '7'); token[offset] = static_cast<char>(byte);
      invalid.push_back(token);
    }
  }
  std::string text;
  for (const auto& token : invalid) text += token + " 17 ";
  // Leading zeros do not turn a representable integer into an overflow.
  text += std::string(150000, '0') + maximum + " +0000000 0000019 " + minimum;
  auto* file = input(text);
  {
    blueberry::FastInput<Interactive> in(file);
    for (const auto& token : invalid) {
      T value = 7;
      if (in.read(value) || value != 7) {
        std::cerr << "seed=" << seed << " invalid-token-size=" << token.size()
                  << " expected=unchanged-7 actual=" << decimal(value) << '\n';
        std::exit(1);
      }
      check(in.read(value) && value == 17, "integer recovery after invalid token");
    }
    T value = 7;
    check(in.read(value) && value == std::numeric_limits<T>::max(), "long leading zeros");
    check(in.read(value) && value == 0, "positive signed zero");
    check(in.read(value) && value == 19, "short leading zeros");
    check(in.read(value) && value == std::numeric_limits<T>::min(), "minimum exactly at EOF");
    check(!in.read(value) && value == std::numeric_limits<T>::min(), "recovery EOF unchanged");
  }
  std::fclose(file);
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
  integers<wchar_t>(rng); integers<char8_t>(rng); integers<char16_t>(rng); integers<char32_t>(rng);
  // The standard runner includes seed 1. Run its fixed large-file cases once,
  // while every seed still exercises randomized values and token recovery.
  if (seed == 1) {
    integer_boundaries<long long>(true); integer_boundaries<unsigned long long>(false);
#if defined(__SIZEOF_INT128__)
    integer_boundaries<__int128_t>(false); integer_boundaries<__uint128_t>(true);
#endif
  }
  integer_token_recovery<long long>(); integer_token_recovery<unsigned long long>();
  integer_token_recovery<signed char>(); integer_token_recovery<unsigned char>();
  integer_token_recovery<long long, true>();
#if defined(__SIZEOF_INT128__)
  wide_integers<__int128_t>(rng); wide_integers<__uint128_t>(rng);
  integer_token_recovery<__int128_t>(); integer_token_recovery<__uint128_t>();
#endif
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
