PROG := pikifen
SRCS := $(shell find ../ -name '*.cpp')
OBJS := ${SRCS:.cpp=.o}
CXXFLAGS := -std=c++0x -g -rdynamic -D_GLIBCXX_USE_CXX11_ABI=0
LDFLAGS += -lm `pkg-config --libs allegro-5 allegro_main-5 allegro_audio-5 allegro_image-5 allegro_font-5 allegro_acodec-5 allegro_primitives-5 allegro_dialog-5`

all: $(PROG)

$(PROG): $(OBJS)
	g++ $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(PROG)
# If the above does not work and gives linker errors, use the following line instead.
#	$(LINK.cc) $(OBJS) -o $(PROG)
# If you prefer Clang, use the following line instead.
#   clang++ $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(PROG)

clean:
	$(RM) $(OBJS)
	$(RM) $(PROG)

distclean: clean

