/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the spray type and spray type-related functions.
 */

#ifndef SPRAY_TYPE_INCLUDED
#define SPRAY_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "status.h"

/* ----------------------------------------------------------------------------
 * A spray type. It decides how the spray behaves,
 * what status effect it causes, and some other values.
 */
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
    
    spray_type(status* effect, const bool burpable, const float duration, const ALLEGRO_COLOR main_color, ALLEGRO_BITMAP* bmp_spray, ALLEGRO_BITMAP* bmp_berry, const bool can_drop_blobs = true, const unsigned int berries_needed = 10);
};

#endif //ifndef SPRAY_TYPE_INCLUDED
