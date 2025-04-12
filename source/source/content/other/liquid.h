/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the liquid class and liquid-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include <string>
#include <vector>

#include "../../util/drawing_utils.h"
#include "../animation/animation.h"
#include "../content.h"


using std::string;


/**
 * @brief A liquid type defines how a sector should look to make it look
 * like water.
 *
 * This is considered a "liquid" and not specifically "water" because the
 * engine allows creating other water-like things, like acid, lava, etc.
 * Each have their own color, reflectivity, etc.
 * A hazard can be associated with a liquid. It's the way the
 * engine has to know if a sector is to be shown as a liquid or not.
 */
struct Liquid : public Content {

    //--- Members ---
    
    //Color the body of liquid is.
    ALLEGRO_COLOR bodyColor = COLOR_BLACK;
    
    //Color the shine of liquid is.
    ALLEGRO_COLOR shineColor = COLOR_WHITE;
    
    //Color used for this liquid in the radar.
    ALLEGRO_COLOR radarColor = COLOR_EMPTY;
    
    //Maximum displacement amount.
    Point distortionAmount = Point(14.0f, 4.0f);
    
    //Noise threshold for how much of the liquid will have no shines.
    float shineMinThreshold = 0.5f;
    
    //Noise threshold for how much of the liquid fully covered in shines.
    float shineMaxThreshold = 1.0f;
    
    //How fast the water animates.
    float animSpeed = 1;
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level);
    
};
