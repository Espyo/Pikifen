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

#include "../../core/const.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/drawing_utils.h"
#include "../content.h"


using std::string;
using std::vector;


struct LiquidType;
class StatusType;


/**
 * @brief An hazard is the likes of fire, water, electricity, crushing, etc.
 *
 * Pikmin can be vulnerable or invulnerable to these.
 * Most of the time, hazards are elements (of nature), but
 * this is not necessarily the case. A hazard is just an abstract danger,
 * not an object that emits said danger.
 */
struct Hazard : public Content {

    //--- Public members ---
    
    //Color that best represents this hazard.
    ALLEGRO_COLOR mainColor = COLOR_EMPTY;
    
    //Status effects for mobs that interact with this hazard.
    vector<StatusType*> effects;
    
    //Do sectors with this hazard block vulnerable Pikmin paths?
    bool blocksPaths = true;
    
    //If it's got an associated liquid, this points to it.
    LiquidType* associatedLiquid = nullptr;
    
    
    //--- Public function declarations ---
    
    void loadFromDataNode(DataNode* node);
    
};
