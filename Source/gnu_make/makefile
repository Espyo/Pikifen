# Makefile for Pikifen. This should be run in the project's root folder. Check the manual in the engine's folder for more info.

PROG         := pikifen
CC           := g++
SRCS         := $(shell find Source/source -name '*.cpp')
OBJS         := $(SRCS:.cpp=.o)
DEPS         := $(OBJS:.o=.d)
ALLEGRO_PKGS := allegro-5 allegro_main-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_primitives-5 allegro_ttf-5
CXXFLAGS     := -std=c++0x -rdynamic -D_GLIBCXX_USE_CXX11_ABI=0 -MMD $(shell pkg-config --cflags $(ALLEGRO_PKGS))
LDFLAGS      += -lm $(shell pkg-config --libs $(ALLEGRO_PKGS))
DEBUGFLAGS   := -g -ggdb -Wall -Wno-unknown-pragmas -O0
RELEASEFLAGS := -Wall -Wextra -Wno-unknown-pragmas -O2
ANALYZEFLAGS := -Wall -Wextra -Wno-unknown-pragmas -fanalyzer

.PHONY: all
all: release

.PHONY: debug
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: $(PROG)

.PHONY: release
release: CXXFLAGS += $(RELEASEFLAGS)
release: $(PROG)

.PHONY: analyze
analyze: CXXFLAGS += $(ANALYZEFLAGS)
analyze: $(PROG)

$(PROG): $(OBJS)
	$(CC) $^ $(CXXFLAGS) $(LDFLAGS) -o $(PROG)
# If the above does not work and gives linker errors, try the following line instead.
#	$(LINK.cc) $(OBJS) -o $(PROG)

-include $(DEPS)

.PHONY: clean
clean:
	$(RM) $(OBJS)
	$(RM) $(DEPS)
	$(RM) $(PROG)
