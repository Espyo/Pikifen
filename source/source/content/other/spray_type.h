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
class spray_type : public content {

public:

    //--- Members ---
    
    //What the spray does.
    vector<status_type*> effects;
    
    //True: applied to the entire group. False: applied in a specified range.
    bool group = true;
    
    //Does it only apply to Pikmin in the group, or leaders too?
    bool group_pikmin_only = true;
    
    //Apply the spray to its user as well.
    bool affects_user = false;
    
    //If applied outside of the group, this is the angle of shooting.
    float angle = 0.0f;
    
    //If applied outside of the group, this is the distance range.
    float distance_range = 0.0f;
    
    //If applied outside of the group, this is the angle range.
    float angle_range = 0.0f;
    
    //Main color that represents this spray.
    ALLEGRO_COLOR main_color = COLOR_EMPTY;
    
    //Bitmap for the spray count.
    ALLEGRO_BITMAP* bmp_spray = nullptr;
    
    //How many ingredients are needed in order to concot a new spray.
    //0 means there are no ingredients for this spray type.
    size_t ingredients_needed = 10;
    
    //Does it bury Pikmin?
    bool buries_pikmin = false;
    
    //--- Function declarations ---
    
    void load_from_data_node(data_node* node, CONTENT_LOAD_LEVEL level);
    
};
