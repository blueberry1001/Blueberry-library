#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace blueberry {

// Borrowed FILE streams; portable buffered I/O, including pipes and clean EOF.
template<bool Interactive = false>
class FastInput {
  static constexpr std::size_t capacity = 1 << 16;
  std::FILE* file_;
  std::array<unsigned char, Interactive ? 1 : capacity> buffer_;
  std::size_t pos_ = 0, end_ = 0;
  bool exhausted_ = false, failed_ = false;
  int next() {
    if constexpr (Interactive) {
      if (exhausted_) return EOF;
      const int c = std::fgetc(file_);
      if (c == EOF) { exhausted_ = true; failed_ = std::ferror(file_) != 0; }
      return c;
    } else {
      if (pos_ == end_) {
        if (exhausted_) return EOF;
        end_ = std::fread(buffer_.data(), 1, capacity, file_);
        failed_ = std::ferror(file_) != 0;
        pos_ = 0;
        if (!end_) { exhausted_ = true; return EOF; }
      }
      return buffer_[pos_++];
    }
  }
  static bool space(int c) {
    return c == ' ' || (c >= '\t' && c <= '\r');
  }
  int first() {
    int c;
    do { c = next(); } while (space(c));
    return c;
  }
  void discard(int c) { while (c != EOF && !space(c)) c = next(); }

 public:
  explicit FastInput(std::FILE* file = stdin) : file_(file) { assert(file); }
  FastInput(const FastInput&) = delete;
  FastInput& operator=(const FastInput&) = delete;

  template<class T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
  bool read(T& value) {
    using U = std::make_unsigned_t<T>;
    int c = first();
    if (c == EOF) return false;
    const bool negative = c == '-';
    if (negative || c == '+') c = next();
    constexpr U positive_limit = U(std::numeric_limits<T>::max());
    constexpr U negative_limit = std::is_signed_v<T> ? positive_limit + U{1} : positive_limit;
    const U quotient = negative ? negative_limit / 10 : positive_limit / 10;
    const U remainder = negative ? negative_limit % 10 : positive_limit % 10;
    if ((negative && !std::is_signed_v<T>) || static_cast<unsigned>(c - '0') > 9) {
      discard(c); return false;
    }
    U result = 0;
    while (static_cast<unsigned>(c - '0') <= 9) {
      const unsigned digit = static_cast<unsigned>(c - '0');
      if (result > quotient || (result == quotient && digit > remainder)) {
        discard(c); return false;
      }
      result = U(result * 10 + digit);
      c = next();
    }
    if (c != EOF && !space(c)) { discard(c); return false; }
    if (failed_) return false;
    if constexpr (std::is_signed_v<T>) {
      // Avoid signed overflow, including the most negative representable value.
      if (negative) value = result == U(std::numeric_limits<T>::max()) + U{1}
          ? std::numeric_limits<T>::min() : T(-T(result));
      else value = T(result);
    } else value = result;
    return true;
  }
  bool read(char& value) {
    const int c = first();
    if (c == EOF || failed_) return false;
    value = static_cast<char>(c);
    return true;
  }
  bool read(bool& value) {
    unsigned parsed;
    if (!read(parsed) || parsed > 1) return false;
    value = parsed != 0;
    return true;
  }
  bool read(std::string& value) {
    int c = first();
    if (c == EOF) return false;
    value.clear();
    do { value.push_back(static_cast<char>(c)); c = next(); } while (c != EOF && !space(c));
    return !failed_;
  }
  template<class T> requires std::is_floating_point_v<T>
  bool read(T& value) {
    std::string token;
    if (!read(token)) return false;
    const char* begin = token.data();
    if (*begin == '+') {
      ++begin;
      if (begin == token.data() + token.size() || *begin == '-' || *begin == '+') return false;
    }
    T parsed;
    const auto result = std::from_chars(begin, token.data() + token.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != token.data() + token.size()) return false;
    value = parsed;
    return true;
  }
  template<class T, class U, class... Rest>
  bool read(T& first_value, U& second_value, Rest&... rest) {
    return read(first_value) && read(second_value) && (read(rest) && ...);
  }
};

class FastOutput {
  static constexpr std::size_t capacity = 1 << 16;
  std::FILE* file_;
  std::array<char, capacity> buffer_;
  std::size_t used_ = 0;
  bool failed_ = false;
  bool drain() {
    if (failed_) return false;
    std::size_t sent = 0;
    while (sent < used_) {
      const auto count = std::fwrite(buffer_.data() + sent, 1, used_ - sent, file_);
      if (!count) { failed_ = true; return false; }
      sent += count;
    }
    used_ = 0;
    return true;
  }
 public:
  explicit FastOutput(std::FILE* file = stdout) : file_(file) { assert(file); }
  FastOutput(const FastOutput&) = delete;
  FastOutput& operator=(const FastOutput&) = delete;
  ~FastOutput() { flush(); }
  bool flush() {
    if (!drain()) return false;
    if (std::fflush(file_) != 0) failed_ = true;
    return !failed_;
  }
  bool write(char value) {
    if (failed_ || (used_ == capacity && !drain())) return false;
    buffer_[used_++] = value;
    return true;
  }
  template<class T> requires std::is_same_v<T, bool>
  bool write(T value) { return write(value ? '1' : '0'); }
  bool write(std::string_view value) {
    if (failed_) return false;
    while (!value.empty()) {
      if (used_ == capacity && !drain()) return false;
      const auto count = std::min(capacity - used_, value.size());
      std::memcpy(buffer_.data() + used_, value.data(), count);
      used_ += count;
      value.remove_prefix(count);
    }
    return true;
  }
  template<class T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
  bool write(T value) {
    using U = std::make_unsigned_t<T>;
    static constexpr char pairs[] = "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";
    char digits[std::numeric_limits<U>::digits10 + 3];
    char* end = digits + sizeof digits;
    char* begin = end;
    bool negative = false;
    if constexpr (std::is_signed_v<T>) negative = value < 0;
    U number = negative ? U{0} - static_cast<U>(value) : static_cast<U>(value);
    while (number >= 100) {
      const unsigned remainder = static_cast<unsigned>(number % 100);
      number /= 100;
      *--begin = pairs[remainder * 2 + 1];
      *--begin = pairs[remainder * 2];
    }
    if (number >= 10) { *--begin = pairs[number * 2 + 1]; *--begin = pairs[number * 2]; }
    else *--begin = static_cast<char>('0' + number);
    if (negative) *--begin = '-';
    return write(std::string_view(begin, static_cast<std::size_t>(end - begin)));
  }
  template<class T> requires std::is_floating_point_v<T>
  bool write(T value) {
    char text[std::numeric_limits<T>::max_digits10 + 32];
    const auto result = std::to_chars(text, text + sizeof text, value);
    if (result.ec != std::errc{}) return false;
    return write(std::string_view(text, static_cast<std::size_t>(result.ptr - text)));
  }
  template<class... Args> bool writeln(const Args&... values) {
    bool ok = true;
    bool first = true;
    [[maybe_unused]] auto one = [&](const auto& value) {
      if (!first) ok = ok && write(' ');
      first = false;
      ok = ok && write(value);
    };
    (one(values), ...);
    return ok && write('\n');
  }
};
}  // namespace blueberry
