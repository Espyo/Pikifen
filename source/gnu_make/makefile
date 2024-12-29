# Makefile for Pikifen. This should be run in the project's root folder. Check the manual in the engine's folder for more info.


# Basics.
PROG         := pikifen
CXX          := g++
SRCS         := $(shell find source/source -name '*.cpp')
OBJS         := $(SRCS:.cpp=.o)
DEPS         := $(OBJS:.o=.d)
ALLEGRO_PKGS := allegro-5 allegro_main-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_primitives-5 allegro_ttf-5 epoxy
CXXFLAGS     := -std=c++0x -D_GLIBCXX_USE_CXX11_ABI=0 -MMD $(shell pkg-config --cflags $(ALLEGRO_PKGS))
LDFLAGS      += -lm $(shell pkg-config --libs $(ALLEGRO_PKGS))
DEBUGFLAGS   := -g -ggdb -Wall -Wno-unknown-pragmas -O0
RELEASEFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -O2
ANALYZEFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -O0

# g++-specific settings.
GCC_CXXFLAGS     := -rdynamic
GCC_ANALYZEFLAGS := -fanalyzer

# Clang-specific settings.
CLA_CXXFLAGS     := 
CLA_ANALYZEFLAGS := --analyze --analyzer-output html


# Setup.
ifeq ($(CXX),g++)
  CXXFLAGS += $(GCC_CXXFLAGS)
  ANALYZEFLAGS += $(GCC_ANALYZEFLAGS)
else ifeq ($(CXX),clang++)
  CXXFLAGS += $(CLA_CXXFLAGS)
  ANALYZEFLAGS += $(CLA_ANALYZEFLAGS)
endif


# Default rule.
.PHONY: all
all: release

# Debug build rule.
.PHONY: debug
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(PROG)

# Release build rule. Has more checks.
.PHONY: release
release: CXXFLAGS += $(RELEASEFLAGS)
release: $(PROG)

# Static analyzer build rule.
.PHONY: analyze
analyze: CXXFLAGS += $(ANALYZEFLAGS)
analyze: $(PROG)

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
