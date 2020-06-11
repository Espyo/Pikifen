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
#include "../game.h"
#include "../mob_fsms/pile_fsm.h"
#include "../mobs/pile.h"
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
    
    target_type = MOB_TARGET_TYPE_PIKMIN_OBSTACLE;
    
    area_editor_prop_struct aep_amount;
    aep_amount.name = "Amount";
    aep_amount.var = "amount";
    aep_amount.type = AEMP_TEXT;
    aep_amount.def_value = "";
    aep_amount.tooltip =
        "How many resources this pile starts with, or leave empty for the max.";
    area_editor_props.push_back(aep_amount);
    
    pile_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pile_type::get_anim_conversions() const {
    anim_conversion_vector v;
    
    v.push_back(std::make_pair(PILE_ANIM_IDLING, "idling"));
    
    return get_anim_conversions_with_groups(v, N_PILE_ANIMS);
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void pile_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    string contents_str;
    string size_animation_suffixes_str;
    data_node* contents_node;
    
    rs.set("can_drop_multiple", can_drop_multiple);
    rs.set("contents", contents_str, &contents_node);
    rs.set("delete_when_finished", delete_when_finished);
    rs.set("health_per_resource", health_per_resource);
    rs.set("hide_when_empty", hide_when_empty);
    rs.set("max_amount", max_amount);
    rs.set("recharge_amount", recharge_amount);
    rs.set("recharge_interval", recharge_interval);
    rs.set("show_amount", show_amount);
    rs.set("size_animation_suffixes", size_animation_suffixes_str);
    
    auto res_type = game.mob_types.resource.find(contents_str);
    if(res_type != game.mob_types.resource.end()) {
        contents = res_type->second;
    } else {
        log_error(
            "Unknown resource type \"" + contents_str + "\"!", contents_node
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
