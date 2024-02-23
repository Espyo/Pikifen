/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration type class and decoration type-related functions.
 */

#include "decoration_type.h"

#include "../mob_fsms/decoration_fsm.h"
#include "../mob_script.h"
#include "../mobs/decoration.h"


/* ----------------------------------------------------------------------------
 * Creates a type of decoration.
 */
decoration_type::decoration_type() :
    mob_type(MOB_CATEGORY_DECORATIONS),
    tint_random_maximum(COLOR_EMPTY),
    scale_random_variation(0.0f),
    rotation_random_variation(0.0f),
    random_animation_delay(false),
    sfx_data_idx(INVALID) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
    area_editor_prop_struct aep_random_anim_delay;
    aep_random_anim_delay.name = "Random animation delay";
    aep_random_anim_delay.var = "random_animation_delay";
    aep_random_anim_delay.type = AEMP_BOOL;
    aep_random_anim_delay.def_value = "true";
    aep_random_anim_delay.tooltip =
        "If this decoration type can have a random animation delay,\n"
        "this property makes this decoration use it or not.";
    area_editor_props.push_back(aep_random_anim_delay);
    
    area_editor_prop_struct aep_random_tint;
    aep_random_tint.name = "Random tint";
    aep_random_tint.var = "random_tint";
    aep_random_tint.type = AEMP_BOOL;
    aep_random_tint.def_value = "true";
    aep_random_tint.tooltip =
        "If this decoration type can have a random color tint,\n"
        "this property makes this decoration use it or not.";
    area_editor_props.push_back(aep_random_tint);
    
    area_editor_prop_struct aep_random_scale;
    aep_random_scale.name = "Random scale";
    aep_random_scale.var = "random_scale";
    aep_random_scale.type = AEMP_BOOL;
    aep_random_scale.def_value = "true";
    aep_random_scale.tooltip =
        "If this decoration type can have a random scale,\n"
        "this property makes this decoration use it or not.";
    area_editor_props.push_back(aep_random_scale);
    
    area_editor_prop_struct aep_random_rotation;
    aep_random_rotation.name = "Random rotation";
    aep_random_rotation.var = "random_rotation";
    aep_random_rotation.type = AEMP_BOOL;
    aep_random_rotation.def_value = "true";
    aep_random_rotation.tooltip =
        "If this decoration type can have a random scale,\n"
        "this property makes this decoration use it or not.";
    area_editor_props.push_back(aep_random_rotation);
    
    blackout_radius = 0.0f;
    
    decoration_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector decoration_type::get_anim_conversions() const {
    anim_conversion_vector v;
    v.push_back(std::make_pair(DECORATION_ANIM_IDLING, "idling"));
    v.push_back(std::make_pair(DECORATION_ANIM_BUMPED, "bumped"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void decoration_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("random_animation_delay", random_animation_delay);
    rs.set("rotation_random_variation", rotation_random_variation);
    rs.set("scale_random_variation", scale_random_variation);
    rs.set("tint_random_maximum", tint_random_maximum);
    
    rotation_random_variation = deg_to_rad(rotation_random_variation);
    
    for(size_t s = 0; s < sounds.size(); ++s) {
        if(sounds[s].name == "bumped") {
            sfx_data_idx = s;
        }
    }
}
