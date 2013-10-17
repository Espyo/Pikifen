PROG := pfe
SRCS := $(wildcard Source/*.cpp)
OBJS := ${SRCS:.cpp=.o}
CXXFLAGS := -std=c++0x
LDFLAGS += -lm `pkg-config --libs allegro-5 allegro_audio-5 allegro_image-5 allegro_font-5 allegro_acodec-5 allegro_primitives-5`

all: $(PROG)

$(PROG): $(OBJS)
	$(LINK.cc) $(OBJS) -o $(PROG)

clean:
	$(RM) $(OBJS)
	$(RM) $(PROG)

distclean: clean

