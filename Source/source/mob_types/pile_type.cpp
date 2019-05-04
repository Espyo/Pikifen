/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile type class and pile type-related functions.
 */

#include "pile_type.h"

#include "../functions.h"
#include "../mobs/pile.h"
#include "../mob_fsms/pile_fsm.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a type of pile.
 */
pile_type::pile_type() :
    mob_type(MOB_CATEGORY_PILES),
    contents(nullptr),
    recharge_interval(0.0f),
    recharge_amount(0),
    max_amount(1),
    health_per_resource(1.0f),
    can_drop_multiple(false),
    show_amount(true),
    hide_when_empty(true),
    delete_when_finished(true) {
    
    pile_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void pile_type::load_parameters(data_node* file) {
    reader_setter rs(file);
    string contents_str;
    string size_animation_suffixes_str;
    rs.set("contents", contents_str);
    rs.set("recharge_interval", recharge_interval);
    rs.set("recharge_amount", recharge_amount);
    rs.set("max_amount", max_amount);
    rs.set("health_per_resource", health_per_resource);
    rs.set("can_drop_multiple", can_drop_multiple);
    rs.set("size_animation_suffixes", size_animation_suffixes_str);
    rs.set("show_amount", show_amount);
    rs.set("hide_when_empty", hide_when_empty);
    rs.set("delete_when_finished", delete_when_finished);
    
    auto res_type = resource_types.find(contents_str);
    if(res_type != resource_types.end()) {
        contents = res_type->second;
    } else {
        log_error(
            "Unknown resource type \"" + contents_str + "\"!", file
        );
    }
    
    animation_group_suffixes =
        semicolon_list_to_vector(size_animation_suffixes_str);
        
    if(animation_group_suffixes.empty()) {
        //Let's make life easier. If no suffixes were given, then create an
        //implied one.
        animation_group_suffixes.push_back("");
    }
    
    max_health = health_per_resource * max_amount;
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pile_type::get_anim_conversions() {
    anim_conversion_vector v;
    
    v.push_back(make_pair(PILE_ANIM_IDLING, "idling"));
    
    return get_anim_conversions_with_groups(v, N_PILE_ANIMS);
}


pile_type::~pile_type() { }
