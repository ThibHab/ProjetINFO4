#pragma once

#include "format.h"  // for formatted, joined
#include "log.h"     // for Level, Level::Error, Log, Level::Info
#include <exception> // for exception
#include <string>    // for operator+, string

namespace dhe {
namespace unit {

using log::Level;
using log::Log;

/**
 * Each test function receives a tester to report test failures and other
 * information. A test function ends when it returns, throws an exception, or
 * calls fatal(), fatalf(), or fail_now().
 */
class Tester {
  class FailNowException : public std::exception {};

public:
  /**
   * Joins the string representation of each arg by spaces and reports the
   * result to the test's log with the given log level. Note that log does not
   * mark the test as failed, even if level indicates an error.
   */
  template <typename... Args> void log(Level level, Args... args) {
    log_->write(level, format::joined(args...));
  }

  /**
   * Replaces each {} in the format with the string representation of the
   * corresponding arg, and reports the result to the test's log with the given
   * log level. Note that logf does not mark the test as failed, even if level
   * indicates an error.
   */
  template <typename... Args>
  void logf(Level level, std::string const &format, Args... args) {
    log_->write(level, format::formatted(format, args...));
  }

  /**
   * Equivalent to log(Level::Info, args).
   */
  template <typename... Args> void log(Args... args) {
    log(Level::Info, args...);
  }

  /**
   * Equivalent to logf(Level::Info, format, args).
   */
  template <typename... Args>
  void logf(std::string const &format, Args... args) {
    logf(Level::Info, format, args...);
  }

  /**
   * Equivalent to log(Level::Error, args) followed by fail().
   */
  template <typename... Args> void error(Args... args) {
    log(Level::Error, fail_text, args..., normal_text);
    fail();
  }

  /**
   * Equivalent to logf(Level::Error, format, args) followed by fail().
   */
  template <typename... Args>
  void errorf(std::string const &format, Args... args) {
    logf(Level::Error, fail_text + format + normal_text, args...);
    fail();
  };

  /**
   * Equivalent to log(Level::Error, args) followed by fail_now().
   */
  template <typename... Args> void fatal(Args... args) {
    log(Level::Error, fail_text, args..., normal_text);
    fail_now();
  }

  /**
   * Equivalent to logf(Level::Error, format, args) followed by fail_now().
   */
  template <typename... Args>
  void fatalf(std::string const &format, Args... args) {
    logf(Level::Error, fail_text + format + normal_text, args...);
    fail_now();
  };

  /**
   * Marks the test as failed and continues executing it.
   */
  void fail() { failed_ = true; }

  /**
   * Marks the test as failed and stops executing it.
   */
  void fail_now() {
    fail();
    throw FailNowException{};
  }

  /**
   * Indicates whether the test has been marked as failed.
   */
  auto failed() const -> bool { return failed_; }

  /**
   * Runs the given test function as a subtest of this test. The test function
   * must have a signature equivalent to: void fun(Tester &t).
   */
  template <typename TestFunc>
  void run(std::string const &name, TestFunc test_func) {
    Tester t{log_};
    log_->start(name);
    try {
      test_func(t);
    } catch (FailNowException const &ignored) {
    } catch (char const *s) {
      t.error("Unexpected string exception:", s);
    } catch (std::exception const &e) {
      t.error("Unexpected exception:", e.what());
    } catch (...) {
      t.error("Unrecognized exception");
    }
    if (t.failed()) {
      fail();
    }
    log_->end();
  }

  Tester(Log *log) : log_{log} {}

private:
  Log *log_;
  bool failed_{false};

  static auto constexpr *fail_text = "\u001b[31m";
  static auto constexpr *normal_text = "\u001b[0m";
};

/**
 * A suite of tests.
 */
class Suite {
public:
  /**
   * Constructs a test suite and registers it with the test runner.
   */
  Suite(std::string name);

  /**
   * Called by the test runner to run the suite's tests. Your suite then calls
   * t.run(name, test) to run each test. Tests can themselves run
   * t.run(name, subtest) to run subtests.
   */
  virtual void run(Tester &t) = 0;
  auto name() const -> std::string const & { return name_; }

private:
  std::string const name_;
};
} // namespace unit
} // namespace dhe
