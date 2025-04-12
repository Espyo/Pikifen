/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the spray type class and spray type-related functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "../../util/drawing_utils.h"
#include "../content.h"
#include "status.h"


using std::string;
using std::vector;


/**
 * @brief A spray type. It decides how the spray behaves,
 * what status effect it causes, and some other values.
 */
class SprayType : public Content {

public:

    //--- Members ---
    
    //What the spray does.
    vector<StatusType*> effects;
    
    //True: applied to the entire group. False: applied in a specified range.
    bool group = true;
    
    //Does it only apply to Pikmin in the group, or leaders too?
    bool groupPikminOnly = true;
    
    //Apply the spray to its user as well.
    bool affectsUser = false;
    
    //If applied outside of the group, this is the angle of shooting.
    float angle = 0.0f;
    
    //If applied outside of the group, this is the distance range.
    float distanceRange = 0.0f;
    
    //If applied outside of the group, this is the angle range.
    float angleRange = 0.0f;
    
    //Main color that represents this spray.
    ALLEGRO_COLOR mainColor = COLOR_EMPTY;
    
    //Bitmap for the spray count.
    ALLEGRO_BITMAP* bmpSpray = nullptr;
    
    //How many ingredients are needed in order to concot a new spray.
    //0 means there are no ingredients for this spray type.
    size_t ingredientsNeeded = 10;
    
    //Does it bury Pikmin?
    bool buriesPikmin = false;
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* node, CONTENT_LOAD_LEVEL level);
    
};
