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

#include "../game.h"
#include "../mob_fsms/bridge_fsm.h"
#include "../mob_script.h"
#include "../mobs/bridge.h"


/* ----------------------------------------------------------------------------
 * Creates a type of bridge.
 */
bridge_type::bridge_type() :
    mob_type(MOB_CATEGORY_BRIDGES),
    bmp_main_texture(nullptr),
    bmp_left_rail_texture(nullptr),
    bmp_right_rail_texture(nullptr),
    rail_width(16.0f) {
    
    radius = 32;
    max_health = 2000;
    pushable = false;
    pushes = false;
    casts_shadow = false;
    can_block_paths = true;
    target_type = MOB_TARGET_TYPE_PIKMIN_OBSTACLE;
    starting_team = MOB_TEAM_OBSTACLE;
    
    area_editor_tips =
        "Link this object to another object, so that\n"
        "you can specify where the bridge ends.\n"
        "A \"Dummy\" object works perfectly for this.\n"
        "Also, changing its health changes how long Pikmin\n"
        "work on it for, or how many fragments are needed.";
        
    area_editor_prop_struct aep_chunks;
    aep_chunks.name = "Chunks";
    aep_chunks.var = "chunks";
    aep_chunks.type = AEMP_INT;
    aep_chunks.def_value = "10";
    aep_chunks.min_value = 1;
    aep_chunks.max_value = 50;
    aep_chunks.tooltip =
        "How many chunks it's divided by.\n"
        "If the bridge goes up or down, it may need\n"
        "more chunks in order to allow enough steps.";
    area_editor_props.push_back(aep_chunks);
    
    bridge_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector bridge_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(BRIDGE_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(BRIDGE_ANIM_DESTROYED, "destroyed"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void bridge_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("rail_width", rail_width);
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 * file:
 *   File to read from.
 */
void bridge_type::load_resources(data_node* file) {
    reader_setter rs(file);
    
    rs.set("main_texture", main_texture_file_name);
    rs.set("left_rail_texture", left_rail_texture_file_name);
    rs.set("right_rail_texture", right_rail_texture_file_name);
    
    if(!main_texture_file_name.empty()) {
        bmp_main_texture = game.textures.get(main_texture_file_name);
    }
    if(!left_rail_texture_file_name.empty()) {
        bmp_left_rail_texture =
            game.textures.get(left_rail_texture_file_name);
    }
    if(!right_rail_texture_file_name.empty()) {
        bmp_right_rail_texture =
            game.textures.get(right_rail_texture_file_name);
    }
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void bridge_type::unload_resources() {
    game.textures.detach(main_texture_file_name);
    game.textures.detach(left_rail_texture_file_name);
    game.textures.detach(right_rail_texture_file_name);
}
