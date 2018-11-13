/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge type class and bridge type-related functions.
 */

#include "../mobs/bridge.h"
#include "../mobs/bridge_fsm.h"
#include "bridge_type.h"
#include "../mob_script.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of bridge.
 */
bridge_type::bridge_type() :
    mob_type(MOB_CATEGORY_BRIDGES),
    bmp_main_texture(nullptr),
    bmp_rail_texture(nullptr) {
    
    always_active = true;
    radius = 32;
    max_health = 2000;
    pushable = false;
    pushes = false;
    casts_shadow = false;
    is_obstacle = true;
    blocks_carrier_pikmin = true;
    projectiles_can_damage = false;
    
    bridge_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void bridge_type::load_resources(data_node* file) {
    main_texture_file_name =
        file->get_child_by_name("main_texture")->value;
    if(!main_texture_file_name.empty()) {
        bmp_main_texture = textures.get(main_texture_file_name);
        
    }
    
    rail_texture_file_name =
        file->get_child_by_name("rail_texture")->value;
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
