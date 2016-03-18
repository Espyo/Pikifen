PROG := pfe
SRCS := $(shell find Source/ -name '*.cpp')
OBJS := ${SRCS:.cpp=.o}
CXXFLAGS := -std=c++0x -g -D_GLIBCXX_USE_CXX11_ABI=0
LDFLAGS += -lm `pkg-config --libs allegro-5.0 allegro_audio-5.0 allegro_image-5.0 allegro_font-5.0 allegro_acodec-5.0 allegro_primitives-5.0 allegro_dialog-5.0`

all: $(PROG)

$(PROG): $(OBJS)
	g++ $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(PROG)
#If the above does not work and gives linker errors, use the following line instead.
#	$(LINK.cc) $(OBJS) -o $(PROG)

clean:
	$(RM) $(OBJS)
	$(RM) $(PROG)

distclean: clean

