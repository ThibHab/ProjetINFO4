#pragma once

#include "log.h"  // for DefaultLog, Log
#include "test.h" // for Log, Suite, Tester

#include <algorithm> // for for_each
#include <iostream>  // for cerr, ostream
#include <string>    // for string
#include <utility>   // for move
#include <vector>    // for vector

namespace dhe {
namespace unit {
namespace runner {
using log::DefaultLog;
using log::Log;

static auto suites() -> std::vector<Suite *> & {
  static auto suites = std::vector<Suite *>{};
  return suites;
}

/**
 * Run tests, logging to log.
 */
static inline auto run_tests(Log *log) -> bool {
  auto failed = false;
  auto s = suites();
  std::for_each(s.cbegin(), s.cend(), [&failed, log](Suite *suite) {
    log->start(suite->name());
    Tester t{log};
    suite->run(t);
    failed = failed || t.failed();
    log->end();
  });
  return failed;
}

/**
 * Run tests, logging to out with the given verbosity.
 */
static inline auto run_tests(std::ostream &out, bool verbose = false) -> bool {
  auto log = DefaultLog{out, verbose};
  return run_tests(&log);
}

/**
 * Run tests, logging to stderr with the given verbosity.
 */
static inline auto run_tests(bool verbose = false) -> bool {
  return run_tests(std::cerr, verbose);
}

} // namespace runner

Suite::Suite(std::string name) : name_{std::move(name)} {
  runner::suites().push_back(this);
}
} // namespace unit
} // namespace dhe
