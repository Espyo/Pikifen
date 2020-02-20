/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge type class and bridge type-related functions.
 */

#include "bridge_type.h"

#include "../mob_fsms/bridge_fsm.h"
#include "../mob_script.h"
#include "../mobs/bridge.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of bridge.
 */
bridge_type::bridge_type() :
    mob_type(MOB_CATEGORY_BRIDGES),
    bmp_main_texture(nullptr),
    bmp_rail_texture(nullptr) {
    
    radius = 32;
    max_health = 2000;
    pushable = false;
    pushes = false;
    casts_shadow = false;
    blocks_carrier_pikmin = true;
    target_type = MOB_TARGET_TYPE_PIKMIN_OBSTACLE;
    starting_team = MOB_TEAM_OBSTACLE;
    
    bridge_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void bridge_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    rs.set("main_texture", main_texture_file_name);
    rs.set("rail_texture", rail_texture_file_name);
    
    if(!main_texture_file_name.empty()) {
        bmp_main_texture = textures.get(main_texture_file_name);
    }
    if(!rail_texture_file_name.empty()) {
        bmp_rail_texture = textures.get(rail_texture_file_name);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector bridge_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(BRIDGE_ANIM_IDLING, "idling"));
    v.push_back(make_pair(BRIDGE_ANIM_DESTROYED, "destroyed"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void bridge_type::unload_resources() {
    textures.detach(main_texture_file_name);
    textures.detach(rail_texture_file_name);
}


bridge_type::~bridge_type() { }
