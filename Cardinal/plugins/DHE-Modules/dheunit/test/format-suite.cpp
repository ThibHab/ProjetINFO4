#include "dheunit/format.h" // for formatted, joined, FormatError
#include "dheunit/test.h"   // for Tester, Tester::FailNowException, Suite

#include <functional> // for function, __base
#include <string>     // for string, operator!=, basic_string
#include <vector>     // for vector

namespace dhe {
namespace unit {
namespace format {
namespace test {

struct FormatTest {
  std::string name_;
  std::function<std::string()> action_;
  std::string want_;
};

static auto join_tests = std::vector<FormatTest>{
    {
        .name_ = "0 args",
        .action_ = []() -> std::string { return joined(); },
        .want_ = "",
    },
    {
        .name_ = "1 arg",
        .action_ = []() -> std::string { return joined(1); },
        .want_ = "1",
    },
    {
        .name_ = "2 args",
        .action_ = []() -> std::string { return joined(1, 2); },
        .want_ = "1 2",
    },
    {
        .name_ = "n args",
        .action_ = []() -> std::string {
          return joined(1, 2, 3, 4, 5); // NOLINT
        },
        .want_ = "1 2 3 4 5",
    },
    {
        .name_ = "various types",
        .action_ = []() -> std::string {
          return joined(true, 1, 2.3F, "three");
        },
        .want_ = "true 1 2.3 three",
    },
};

static auto format_tests = std::vector<FormatTest>{
    {
        .name_ = "empty format",
        .action_ = []() -> std::string { return formatted(""); },
        .want_ = "",
    },
    {
        .name_ = "0 anchors, 0 args",
        .action_ = []() -> std::string { return formatted("0 anchors"); },
        .want_ = "0 anchors",
    },
    {
        .name_ = "1 anchor, 1 arg",
        .action_ = []() -> std::string { return formatted("{} anchor", 1); },
        .want_ = "1 anchor",
    },
    {
        .name_ = "2 anchors, 2 args",
        .action_ = []() -> std::string {
          return formatted("one:{} two:{}", 1, 2);
        },
        .want_ = "one:1 two:2",
    },
    {
        .name_ = "n anchors, n args",
        .action_ = []() -> std::string {
          return formatted("values {}+{}+{}+{}+{}", 1, 2, 3, 4, 5); // NOLINT
        },
        .want_ = "values 1+2+3+4+5",

    },
    {
        .name_ = "various types",
        .action_ = []() -> std::string {
          return formatted("bool:{} int:{} float:{} string:{}", true, 1, 2.3F,
                           "three");
        },
        .want_ = "bool:true int:1 float:2.3 string:three",
    },
    {
        .name_ = "unmatched opening brace",
        .action_ = []() -> std::string { return formatted("{} {} {", 1, 2); },
        .want_ = "1 2 {",
    },
    {
        .name_ = "opening brace before anchor",
        .action_ = []() -> std::string { return formatted("{{} {}", 1, 2); },
        .want_ = "{1 2",
    },
};

struct FormatErrorTest {
  std::string name_;
  std::function<std::string()> action_;
};

static auto format_error_tests = std::vector<FormatErrorTest>{
    {
        .name_ = "more anchors than args",
        .action_ = []() -> std::string { return formatted("{} {}", 1); },
    },
    {
        .name_ = "more args than anchors",
        .action_ = []() -> std::string { return formatted("{}", 1, 2); },
    },
};

struct FormatSuite : public Suite {
  FormatSuite() : Suite("format") {}

  void run(Tester &t) override {
    t.run("joined()", [](Tester &t) {
      for (auto &test : join_tests) {
        t.run(test.name_, [test](Tester &t) {
          auto const got = test.action_();
          if (got != test.want_) {
            t.errorf(R"(Got "{}", want "{}")", got, test.want_);
          }
        });
      }
    });

    t.run("formatted()", [](Tester &t) {
      for (auto &test : format_tests) {
        t.run(test.name_, [test](Tester &t) {
          auto const got = test.action_();
          if (got != test.want_) {
            t.errorf(R"(Got "{}", want "{}")", got, test.want_);
          }
        });
      }
    });

    t.run("invalid format", [](Tester &t) {
      for (auto &test : format_error_tests) {
        t.run(test.name_, [test](Tester &t) {
          try {
            auto const got = test.action_();
            t.errorf(R"(Got "{}", want FormatError)", got);
          } catch (format::FormatError & /*expected*/) {
          }
        });
      }
    });
  }
};

__attribute__((unused)) static auto _ = FormatSuite{};

} // namespace test
} // namespace format
} // namespace unit
} // namespace dhe
