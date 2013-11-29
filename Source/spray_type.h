#ifndef SPRAY_TYPE_INCLUDED
#define SPRAY_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "status.h"

class spray_type {
public:
    status* effect;            //What the spray does.
    bool burpable;             //If true, the spray is applied to the front. If false, to the back.
    float duration;            //How long the status effect last for.
    
    ALLEGRO_COLOR main_color;
    ALLEGRO_BITMAP* bmp_spray; //Bitmap for the spray count.
    ALLEGRO_BITMAP* bmp_berry; //Bitmap for the berry count.
    
    unsigned int berries_needed; //How many berries are needed in order to concot a new spray. 0 means there are no berries for this spray type.
    bool can_drop_blobs;         //Is it possible for the game to randomly give spray blobs of this spray type?
    
    spray_type(status* effect, bool burpable, float duration, ALLEGRO_COLOR main_color, ALLEGRO_BITMAP* bmp_spray, ALLEGRO_BITMAP* bmp_berry, bool can_drop_blobs = true, unsigned int berries_needed = 10);
};

#endif //ifndef SPRAY_TYPE_INCLUDED