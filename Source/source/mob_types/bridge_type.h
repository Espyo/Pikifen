/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge type class and bridge type-related functions.
 */

#ifndef BRIDGE_TYPE_INCLUDED
#define BRIDGE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../utils/data_file.h"
#include "mob_type.h"


enum BRIDGE_ANIMATIONS {
    BRIDGE_ANIM_IDLING,
    BRIDGE_ANIM_DESTROYED,
};


enum BRIDGE_STATES {
    BRIDGE_STATE_IDLING,
    BRIDGE_STATE_CREATING_CHUNK,
    BRIDGE_STATE_DESTROYED,
    
    N_BRIDGE_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of bridge.
 */
class bridge_type : public mob_type {
public:
    ALLEGRO_BITMAP* bmp_main_texture;
    ALLEGRO_BITMAP* bmp_left_rail_texture;
    ALLEGRO_BITMAP* bmp_right_rail_texture;
    string main_texture_file_name;
    string left_rail_texture_file_name;
    string right_rail_texture_file_name;
    float rail_width;
    
    bridge_type();
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
    void unload_resources();
};


#endif //ifndef BRIDGE_TYPE_INCLUDED
