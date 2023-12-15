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

#include "../libs/data_file.h"
#include "mob_type.h"


//Bridge object animations.
enum BRIDGE_ANIMATIONS {
    //Idling.
    BRIDGE_ANIM_IDLING,
    //Destroyed.
    BRIDGE_ANIM_DESTROYED,
};


//Bridge object states.
enum BRIDGE_STATES {
    //Idling.
    BRIDGE_STATE_IDLING,
    //Creating a chunk.
    BRIDGE_STATE_CREATING_CHUNK,
    //Destroyed.
    BRIDGE_STATE_DESTROYED,
    
    //Total amount of bridge object states.
    N_BRIDGE_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of bridge.
 */
class bridge_type : public mob_type {
public:
    //Texture used for the main bridge floor.
    ALLEGRO_BITMAP* bmp_main_texture;
    //Texture used for the left rail.
    ALLEGRO_BITMAP* bmp_left_rail_texture;
    //Texture used for the right rail.
    ALLEGRO_BITMAP* bmp_right_rail_texture;
    //File name of the main texture.
    string main_texture_file_name;
    //File name of the left rail texture.
    string left_rail_texture_file_name;
    //File name of the right rail texture.
    string right_rail_texture_file_name;
    //Width of each rail.
    float rail_width;
    
    bridge_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    void unload_resources() override;
};


#endif //ifndef BRIDGE_TYPE_INCLUDED
