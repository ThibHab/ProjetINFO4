#pragma once

#include <sstream>   // for ostream, operator<<, basic_ostringstream, boola...
#include <stdexcept> // for runtime_error
#include <string>    // for string

namespace dhe {
namespace unit {
namespace format {
struct FormatError : public std::runtime_error {
  FormatError(char const *what) : std::runtime_error{what} {}
};

static inline void write(std::ostream &out) {}

template <typename Arg> void write(std::ostream &out, Arg arg) { out << arg; }

template <typename First, typename Second, typename... More>
static inline void write(std::ostream &out, First first, Second second,
                         More... more) {
  out << first << ' ';
  write(out, second, more...);
}

static inline void writef(std::ostream &out, char const *format) {
  if (format == nullptr) {
    throw FormatError{"Log format error: null format"};
  }
  while (format[0] != 0) {
    if (format[0] == '{' && format[1] == '}') {
      throw FormatError{"Log format error: not enough arguments"};
    }
    out << format[0];
    format++;
  }
}

template <typename First, typename... More>
static inline void writef(std::ostream &out, char const *format, First first,
                          More... more) {
  if (format == nullptr) {
    throw FormatError{"Log format error: null format"};
  }
  while (format[0] != 0) {
    if (format[0] == '{' && format[1] == '}') {
      out << first;
      return writef(out, format + 2, more...);
    }
    out << format[0];
    format++;
  }
  throw FormatError{"Log format error: too many arguments"};
}

template <typename... Args>
static inline auto joined(Args... args) -> std::string {
  auto s = std::ostringstream{};
  s << std::boolalpha;
  write(s, args...);
  return s.str();
}

template <typename... Args>
static inline auto formatted(std::string const &format, Args... args)
    -> std::string {
  auto s = std::ostringstream{};
  s << std::boolalpha;
  writef(s, format.c_str(), args...);
  return s.str();
}
} // namespace format
} // namespace unit
} // namespace dhe
