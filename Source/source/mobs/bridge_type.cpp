/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge type class and bridge type-related functions.
 */

#include "bridge.h"
#include "bridge_fsm.h"
#include "bridge_type.h"
#include "../mob_script.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of bridge.
 */
bridge_type::bridge_type() :
    mob_type(),
    bmp_main_texture(nullptr),
    bmp_rail_texture(nullptr) {
    
    always_active = true;
    radius = 32;
    max_health = 2000;
    pushable = false;
    pushes = false;
    casts_shadow = false;
    is_obstacle = true;
    
    bridge_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads data about the bridge type from a data file.
 */
void bridge_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    string main_texture_name = file->get_child_by_name("main_texture")->value;
    if(!main_texture_name.empty()) {
        bmp_main_texture =
            bitmaps.get(TEXTURES_FOLDER_NAME + "/" + main_texture_name);
    }
    
    string rail_texture_name = file->get_child_by_name("rail_texture")->value;
    if(!rail_texture_name.empty()) {
        bmp_rail_texture =
            bitmaps.get(TEXTURES_FOLDER_NAME + "/" + rail_texture_name);
    }
    
    anim_conversions->push_back(make_pair(BRIDGE_ANIM_IDLING, "idling"));
    anim_conversions->push_back(make_pair(BRIDGE_ANIM_DESTROYED, "destroyed"));
}
