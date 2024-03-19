#include "dheunit/log.h"  // for Log, Level, Level::Info, DefaultLog, Level...
#include "dheunit/test.h" // for Log, Level, Tester, Tester::FailNowException

#include <functional> // for function, __base
#include <sstream>    // for string, operator<<, basic_ostringstream
#include <string>     // for basic_string, char_traits, operator!=
#include <vector>     // for vector

namespace dhe {
namespace unit {
namespace log {
namespace test {

struct LogTest {
  std::string name_;
  std::function<void(Log &)> action_;
  std::vector<std::string> want_;
};

static auto verbose_log_tests = std::vector<LogTest>{
    {
        .name_ = "start() writes name",
        .action_ = [](Log &l) { l.start("test name"); },
        .want_ = {"test name"},
    },
    {
        .name_ = "start() indents subsequent writes",
        .action_ =
            [](Log &l) {
              l.start("start 1");
              l.write(Level::Info, "write 1");
              l.write(Level::Info, "write 2");
              l.write(Level::Info, "write 3");
            },
        .want_ =
            {
                "start 1",
                "    write 1",
                "    write 2",
                "    write 3",
            },
    },
    {
        .name_ = "start() indents subsequent starts",
        .action_ =
            [](Log &l) {
              l.start("start 1");
              l.start("start 2");
            },
        .want_ =
            {
                "start 1",
                "    start 2",
            },
    },
    {
        .name_ = "each start() increases indent",
        .action_ =
            [](Log &l) {
              l.start("start 1");
              l.write(Level::Info, "write 1");
              l.start("start 2");
              l.write(Level::Info, "write 2");
              l.start("start 3");
              l.write(Level::Info, "write 3");
            },
        .want_ =
            {
                "start 1",
                "    write 1",
                "    start 2",
                "        write 2",
                "        start 3",
                "            write 3",
            },
    },
    {
        .name_ = "each end() decreases indent",
        .action_ =
            [](Log &l) {
              l.start("start test 1");
              l.write(Level::Info, "test 1 info");
              l.start("start test 2");
              l.write(Level::Info, "test 2 info");
              l.start("start test 3");
              l.write(Level::Info, "test 3 info");
              l.end();
              l.write(Level::Info, "more test 2 info");
              l.end();
              l.write(Level::Info, "more test 1 info");
              l.end();
            },
        .want_ =
            {
                "start test 1",
                "    test 1 info",
                "    start test 2",
                "        test 2 info",
                "        start test 3",
                "            test 3 info",
                "        more test 2 info",
                "    more test 1 info",
            },
    },
    {
        .name_ = "write() writes Info line",
        .action_ =
            [](Log &l) {
              l.start("start");
              l.write(Level::Info, "Info line");
            },
        .want_ =
            {
                "start",
                "    Info line",
            },
    },
    {
        .name_ = "write() writes Error line",
        .action_ =
            [](Log &l) {
              l.start("start");
              l.write(Level::Error, "Error line");
            },
        .want_ =
            {
                "start",
                "    Error line",
            },
    },
};

static auto quiet_log_tests = std::vector<LogTest>{};

struct DefaultLogSuite : public Suite {
  DefaultLogSuite() : Suite("DefaultLog") {}

  void run(Tester &t) override {
    t.run("verbose", [](Tester &t) {
      for (auto &test : verbose_log_tests) {
        t.run(test.name_, [&test](Tester &t) {
          auto out = std::ostringstream{};
          auto log = DefaultLog{out, true};

          test.action_(log);

          auto want_str = std::ostringstream{};
          for (auto &str : test.want_) {
            want_str << str << '\n';
          }

          auto const got = out.str();
          auto const want = want_str.str();
          if (got != want) {
            t.errorf("\n----- Got\n{}\n----- Want\n{}\n-----\n", got, want);
          }
        });
      }
    });

    t.run("quiet", [](Tester &t) {
      for (auto &test : quiet_log_tests) {
        auto out = std::ostringstream{};
        auto log = DefaultLog{out, false};

        test.action_(log);

        auto want_str = std::ostringstream{};
        for (auto &str : test.want_) {
          want_str << str << '\n';
        }

        auto const got = out.str();
        auto const want = want_str.str();
        if (got != want) {
          t.errorf("\n----- Got\n{}\n----- Want\n{}\n-----\n", got, want);
        }
      }
    });
  }
};

__attribute__((unused)) static auto _ = DefaultLogSuite{};
} // namespace test
} // namespace log
} // namespace unit
} // namespace dhe
