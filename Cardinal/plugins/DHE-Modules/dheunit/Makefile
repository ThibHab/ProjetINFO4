CXXFLAGS += -I.

# Generate dependency files with the object files
CXXFLAGS += -MMD -MP

# Optimization
CXXFLAGS += -O3 -march=nocona -funsafe-math-optimizations

# Warnings
CXXFLAGS += -Wall -Wextra -Wno-unused-parameter

# C++ standard
CXXFLAGS += -std=c++11

MACHINE = $(shell $(CC) -dumpmachine)
ifneq (, $(findstring apple, $(MACHINE)))
	ARCH_MAC := 1
	ARCH := mac
else ifneq (, $(findstring mingw, $(MACHINE)))
	ARCH_WIN := 1
	ARCH := win
	ifneq ( ,$(findstring x86_64, $(MACHINE)))
		ARCH_WIN_64 := 1
		BITS := 64
	else ifneq (, $(findstring i686, $(MACHINE)))
		ARCH_WIN_32 := 1
		BITS := 32
	endif
else ifneq (, $(findstring linux, $(MACHINE)))
	ARCH_LIN := 1
	ARCH := lin
else
	$(error Could not determine architecture of $(MACHINE).)
endif

# Architecture-dependent flags
ifdef ARCH_LIN
	CXXFLAGS += -Wsuggest-override
endif
ifdef ARCH_MAC
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
	MAC_SDK_FLAGS = -mmacosx-version-min=10.7
	CXXFLAGS += $(MAC_SDK_FLAGS)
	LDFLAGS += $(MAC_SDK_FLAGS)
endif
ifdef ARCH_WIN
	CXXFLAGS += -D_USE_MATH_DEFINES
	CXXFLAGS += -Wsuggest-override
endif

OBJECTS := $(patsubst %.cpp, build/%.cpp.o, $(SOURCES))

SOURCES = $(shell find test -name "*.cpp")

OBJECTS := $(patsubst %, build/%.o, $(SOURCES))

RUNNER = build/runtests

all: test

.PHONY: test vtest
test: testrunner
	$(RUNNER)

vtest: testrunner
	$(RUNNER) --verbose

$(RUNNER): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: testrunner
testrunner: $(RUNNER)




########################################################################
#
# Analyze and format the code
#
########################################################################

HEADERS = $(shell find dheunit -name "*.h")

.PHONY: format
format:
	clang-format -i -style=file $(HEADERS) $(SOURCES)

COMPILATION_DB = compile_commands.json

COMPILATION_DB_ENTRIES := $(patsubst %, build/%.json, $(SOURCES))

$(COMPILATION_DB): $(COMPILATION_DB_ENTRIES)
	sed -e '1s/^/[/' -e '$$s/,$$/]/' $^ | json_pp > $@

.PHONY: setup
setup: $(COMPILATION_DB)

.PHONY: tidy
tidy: setup
	clang-tidy -p=build $(SOURCES)

IWYU := include-what-you-use
IWYU += -Xiwyu --quoted_includes_first
IWYU += -Xiwyu --mapping_file=.iwyu.libcxx.yaml
IWYU += -Xiwyu --transitive_includes_only

iwyu:
	$(MAKE) -Bi CXX='$(IWYU)' $(OBJECTS)

check: tidy iwyu



########################################################################
#
# General rules
#
########################################################################

-include $(OBJECTS:.o=.d)

build/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.json: %
	@mkdir -p $(@D)
	clang $(CXXFLAGS) -MJ $@ -c -o build/$^.o $^

clean:
	rm -rf build
	rm -rf $(COMPILATION_DB)
