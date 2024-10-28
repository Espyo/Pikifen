/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hazard class and hazard-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include <string>
#include <vector>

#include "const.h"
#include "content.h"
#include "libs/data_file.h"
#include "utils/drawing_utils.h"


using std::string;
using std::vector;


struct liquid;
class status_type;


/**
 * @brief An hazard is the likes of fire, water, electricty, crushing, etc.
 *
 * Pikmin can be vulnerable or invulnerable to these.
 * Most of the time, hazards are elements (of nature), but
 * this is not necessarily the case. A hazard is just an abstract danger,
 * not an object that emits said danger.
 */
struct hazard : public content {

    //--- Members ---
    
    //Color that best represents this hazard.
    ALLEGRO_COLOR main_color = COLOR_EMPTY;
    
    //Status effects for mobs that interact with this hazard.
    vector<status_type*> effects;
    
    //If it's got an associated liquid, this points to it.
    liquid* associated_liquid = nullptr;
    
    
    //--- Function declarations ---
    
    void load_from_data_node(data_node* node);
    
};
