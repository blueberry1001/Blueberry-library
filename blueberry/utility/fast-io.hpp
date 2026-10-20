#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <deque>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

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
    if constexpr (!Interactive) {
      if (pos_ < end_ && space(buffer_[pos_])) {
        ++pos_;
        if (failed_) return false;
        const T digit = static_cast<T>(c - '0');
        value = negative ? T(-digit) : digit;
        return true;
      }
    }
    U result = 0;
    if constexpr (!Interactive && sizeof(U) >= 4 && std::endian::native == std::endian::little) {
      const U block_quotient = negative ? negative_limit / 100000000 : positive_limit / 100000000;
      const U block_remainder = negative ? negative_limit % 100000000 : positive_limit % 100000000;
      // Once a block contains a delimiter, finish this token in the scalar loop.
      while (end_ - pos_ >= 7 && static_cast<unsigned>(buffer_[pos_] - '0') <= 9) {
        // next() has already consumed c. Load only bytes inside the filled buffer.
        std::uint64_t block;
        std::memcpy(&block, buffer_.data() + pos_ - 1, sizeof block);
        constexpr std::uint64_t high = 0xf0f0f0f0f0f0f0f0ULL;
        if ((block & high) != 0x3030303030303030ULL ||
            ((block + 0x0606060606060606ULL) & high) != 0x3030303030303030ULL) break;
        block &= 0x0f0f0f0f0f0f0f0fULL;
        block = (block * 10 + (block >> 8)) & 0x00ff00ff00ff00ffULL;
        block = (block * 100 + (block >> 16)) & 0x0000ffff0000ffffULL;
        const auto digits = static_cast<unsigned>((block * 10000 + (block >> 32)) & 0xffffffffULL);
        if (result > block_quotient || (result == block_quotient && digits > block_remainder)) {
          discard(c); return false;
        }
        result = U(result * 100000000 + digits);
        pos_ += 7;
        c = next();
      }
    }
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
  template<class T, class U> bool read(std::pair<T, U>& value) {
    return read(value.first) && read(value.second);
  }
  template<class T, class Alloc> bool read(std::vector<T, Alloc>& values) {
    // vector<bool> returns proxies rather than bool references.
    for (std::size_t i = 0; i < values.size(); ++i) {
      if constexpr (std::is_same_v<T, bool>) {
        bool value;
        if (!read(value)) return false;
        values[i] = value;
      } else if (!read(values[i])) return false;
    }
    return true;
  }
  template<class T, class Alloc> bool read(std::deque<T, Alloc>& values) {
    for (auto& value : values) if (!read(value)) return false;
    return true;
  }
  template<class T, std::size_t N> bool read(std::array<T, N>& values) {
    for (auto& value : values) if (!read(value)) return false;
    return true;
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
    constexpr std::size_t maximum = std::numeric_limits<U>::digits10 + 2;
    if (failed_ || (capacity - used_ < maximum && !drain())) return false;
    char* begin = buffer_.data() + used_;
    bool negative = false;
    if constexpr (std::is_signed_v<T>) negative = value < 0;
    U number = negative ? U{0} - static_cast<U>(value) : static_cast<U>(value);
    if (negative) *begin++ = '-';
    if (number < 10) {
      *begin++ = static_cast<char>('0' + number);
    } else if constexpr (sizeof(U) > sizeof(std::uint64_t)) {
      // At most two wide divisions for 128-bit values; format the rest as 64-bit.
      constexpr std::uint64_t base = 10000000000000000000ULL;
      std::uint64_t chunks[2];
      unsigned count = 0;
      while (number >= base) {
        chunks[count++] = static_cast<std::uint64_t>(number % base);
        number /= base;
      }
      begin = std::to_chars(begin, buffer_.data() + capacity, static_cast<std::uint64_t>(number)).ptr;
      while (count) {
        char digits[19];
        const auto end = std::to_chars(digits, digits + sizeof digits, chunks[--count]).ptr;
        const auto size = static_cast<std::size_t>(end - digits);
        std::memset(begin, '0', 19 - size);
        begin += 19 - size;
        std::memcpy(begin, digits, size);
        begin += size;
      }
    } else {
      static constexpr char pairs[] = "0001020304050607080910111213141516171819"
          "2021222324252627282930313233343536373839"
          "4041424344454647484950515253545556575859"
          "6061626364656667686970717273747576777879"
          "8081828384858687888990919293949596979899";
      char digits[maximum];
      char* const end = digits + sizeof digits;
      char* start = end;
      while (number >= 100) {
        const auto remainder = static_cast<unsigned>(number % 100);
        number /= 100;
        *--start = pairs[remainder * 2 + 1];
        *--start = pairs[remainder * 2];
      }
      if (number >= 10) { *--start = pairs[number * 2 + 1]; *--start = pairs[number * 2]; }
      else *--start = static_cast<char>('0' + number);
      const auto size = static_cast<std::size_t>(end - start);
      std::memcpy(begin, start, size);
      begin += size;
    }
    used_ = static_cast<std::size_t>(begin - buffer_.data());
    return true;
  }
  template<class T> requires std::is_floating_point_v<T>
  bool write(T value) {
    char text[std::numeric_limits<T>::max_digits10 + 32];
    const auto result = std::to_chars(text, text + sizeof text, value);
    if (result.ec != std::errc{}) return false;
    return write(std::string_view(text, static_cast<std::size_t>(result.ptr - text)));
  }
  template<class T> requires std::is_floating_point_v<T>
  bool write(T value, int precision) {
    assert(precision >= 0);
    char local[128];
    auto result = std::to_chars(local, local + sizeof local, value, std::chars_format::fixed, precision);
    if (result.ec == std::errc{})
      return write(std::string_view(local, static_cast<std::size_t>(result.ptr - local)));
    if (result.ec != std::errc::value_too_large) return false;
    std::string text(static_cast<std::size_t>(precision) + std::numeric_limits<T>::max_exponent10 + 32, '\0');
    result = std::to_chars(text.data(), text.data() + text.size(), value, std::chars_format::fixed, precision);
    if (result.ec != std::errc{}) return false;
    return write(std::string_view(text.data(), static_cast<std::size_t>(result.ptr - text.data())));
  }
  template<class T, class U> bool write(const std::pair<T, U>& value) {
    return write(value.first) && write(' ') && write(value.second);
  }
  template<class T, class Alloc> bool write(const std::vector<T, Alloc>& values) {
    bool first = true;
    for (const T& value : values) {
      if (!first && !write(' ')) return false;
      first = false;
      if (!write(value)) return false;
    }
    return true;
  }
  template<class T, class Alloc> bool write(const std::deque<T, Alloc>& values) {
    bool first = true;
    for (const auto& value : values) {
      if (!first && !write(' ')) return false;
      first = false;
      if (!write(value)) return false;
    }
    return true;
  }
  template<class T, std::size_t N> bool write(const std::array<T, N>& values) {
    bool first = true;
    for (const auto& value : values) {
      if (!first && !write(' ')) return false;
      first = false;
      if (!write(value)) return false;
    }
    return true;
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
