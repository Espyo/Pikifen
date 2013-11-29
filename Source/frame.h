#ifndef FRAME_INCLUDED
#define FRAME_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

class frame {
public:
    //ToDo use subbitmaps.
    ALLEGRO_BITMAP* sprite;
    float duration;
};

#endif //ifndef FRAME_INCLUDED