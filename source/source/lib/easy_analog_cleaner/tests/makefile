# Makefile for the Easy Analog Cleaner library's test framework. This should be run in the tests folder.


# Basics.
PROG         := easy_analog_cleaner_tests
CXX          := g++
SRCS         := $(shell find .. -name '*.cpp')
OBJS         := $(SRCS:.cpp=.o)
DEPS         := $(OBJS:.o=.d)
CXXFLAGS     := -std=c++17 -D_GLIBCXX_USE_CXX11_ABI=0 -MMD -DEASY_ANALOG_CLEANER_UNIT_TESTS
DEBUGFLAGS   := -g -ggdb -Wall -Wno-unknown-pragmas -O0
RELEASEFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -O2 -Wno-missing-field-initializers



# Default rule.
.PHONY: all
all: release

# Debug build rule.
.PHONY: debug
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(PROG)

# Release build rule.
.PHONY: release
release: CXXFLAGS += $(RELEASEFLAGS)
release: $(PROG)

# Compilation rule.
$(PROG): $(OBJS)
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $(PROG)
# If the above does not work and gives linker errors, try the following line instead.
#	$(LINK.cc) $(OBJS) -o $(PROG)

-include $(DEPS)

# Clean project rule.
.PHONY: clean
clean:
	$(RM) $(OBJS)
	$(RM) $(DEPS)
	$(RM) $(PROG)
