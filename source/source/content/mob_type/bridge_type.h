/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge type class and bridge type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../lib/data_file/data_file.h"
#include "mob_type.h"


//Bridge object animations.
enum BRIDGE_ANIM {

    //Idling.
    BRIDGE_ANIM_IDLING,
    
    //Destroyed.
    BRIDGE_ANIM_DESTROYED,
    
};


//Bridge object states.
enum BRIDGE_STATE {

    //Idling.
    BRIDGE_STATE_IDLING,
    
    //Creating a chunk.
    BRIDGE_STATE_CREATING_CHUNK,
    
    //Destroyed.
    BRIDGE_STATE_DESTROYED,
    
    //Total amount of bridge object states.
    N_BRIDGE_STATES,
    
};


/**
 * @brief A type of bridge.
 */
class BridgeType : public MobType {

public:

    //--- Members ---
    
    //Texture used for the main bridge floor.
    ALLEGRO_BITMAP* bmp_main_texture = nullptr;
    
    //Texture used for the left rail.
    ALLEGRO_BITMAP* bmp_left_rail_texture = nullptr;
    
    //Texture used for the right rail.
    ALLEGRO_BITMAP* bmp_right_rail_texture = nullptr;
    
    //Internal name of the main texture bitmap.
    string main_texture_bmp_name;
    
    //Internal name of the left rail texture bitmap.
    string left_rail_texture_bmp_name;
    
    //Internal name of the right rail texture bitmap.
    string right_rail_texture_bmp_name;
    
    //Width of each rail.
    float rail_width = 16.0f;
    
    
    //--- Function declarations ---
    
    BridgeType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    void unloadResources() override;
    
};
